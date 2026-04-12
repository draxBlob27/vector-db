#include "vectorDB/NSW_Index.hpp"
#include "vectorDB/utils/Importer.hpp"
#include "vectorDB/utils/Vector.hpp"
#include "vectorDB/utils/Timer.hpp"
#include <thread>
#include <random>
#include <algorithm>
#include <fstream>

std::string dir = "/home/rohitfeb641/vector-db/sift10K/";
            
auto sift_res{Importer::import_sift1m(dir + "sift_base.fvecs", dir + "sift_query.fvecs", dir + "sift_groundtruth.ivecs").take_ok_value()};

const std::vector<std::uint64_t>& ids{sift_res.ids};
std::vector<Vector> vectors{std::move(sift_res.vectors)};
std::vector<int> order(vectors.size());

const std::vector<std::vector<float>>& queries{sift_res.queries};
const std::vector<std::vector<std::uint32_t>>& truths{sift_res.truths};
const std::vector<std::uint32_t>& truth_k{sift_res.truth_k};

int m = 16, constr = 200, search = 100;
NSW_Index db(m, constr, search);
int my_k{10};
std::atomic<int> intersection = 0;

void neighbor_search_thread(int st, int end, std::vector<std::vector<std::pair<std::uint64_t, float>>>& best) {
    for (int i{st}; i < end; i++) {
        best[i - st] = std::move(db.find_neighbors(vectors[order[i]]));
    }
}

void insert_node(int st, int end, const std::vector<std::vector<std::pair<std::uint64_t, float>>>& best) {
    for (int i{st}; i < end; i++) {
        db.link_node(ids[order[i]], vectors[order[i]], best[i - st]);
    }
}

void query_thread(int st, int end) {
    for (int i{st}; i < end; i++) {
        auto res = db.query(queries[i], my_k);
        for (std::uint32_t j = 0; j < my_k; j++) {
            for (std::uint32_t k = 0; k < my_k; k++) {
                if (res[j].first == truths[i][k]) {
                    intersection.fetch_add(1, std::memory_order_relaxed);
                    break;
                }
            }
        }
    }
}


void bootup_func(int bootup_size) {
    for (int i = 0; i < bootup_size; i++) {
        db.link_node(ids[order[i]], vectors[order[i]], db.find_neighbors(vectors[order[i]]));
    }
}

int main() {
    std::atomic<double> build_time = 0, query_time = 0;
    Timer t{};
    std::random_device rd;
    std::mt19937 g(rd());
    std::iota(order.begin(), order.end(), 0);
    std::shuffle(order.begin(), order.end(), g);

    int w = 8;

    int bootup_size = 100;
    t.reset();
    std::thread bootup(bootup_func, bootup_size);
    bootup.join();
    build_time.fetch_add(t.elapsed(), std::memory_order_relaxed);

    int rem = vectors.size() - bootup_size;
    int batch_size = 1000;
    int st = bootup_size;
    
    std::vector<std::vector<std::vector<std::pair<std::uint64_t, float>>>> best_neighbors(w, std::vector<std::vector<std::pair<std::uint64_t, float>>>((batch_size + w - 1) / w));

    t.reset();
    while (st < vectors.size()) {
        int batch_end = std::min(st + batch_size, (int)vectors.size());
        int total = batch_end - st;
        int chunk = total / w;
        int extra = total % w;

        std::vector<std::thread> workers;
        int cur = st;
        for (int i = 0; i < w; i++) {
            int sz = chunk + (i < extra ? 1 : 0);  // distribute remainder
            int end = cur + sz;

            if (cur >= batch_end) break;
            workers.emplace_back(neighbor_search_thread, cur, end, std::ref(best_neighbors[i]));
            cur = end;
        }

        for (auto& th : workers) {
            th.join();
        }

        workers.clear();
        cur = st;
        for (int i = 0; i < w; i++) {
            int sz = chunk + (i < extra ? 1 : 0);  // distribute remainder
            int end = cur + sz;

            if (cur >= batch_end) break;
            workers.emplace_back(insert_node, cur, end, std::ref(best_neighbors[i]));
            cur = end;
        }

        for (auto& th : workers) {
            th.join();
        }

        st = batch_end;
    }
    build_time.fetch_add(t.elapsed(), std::memory_order_relaxed);

    std::vector<std::thread> consumers;
    int c = 4;
    int num_queries = 100;
    int ch_size = num_queries / c;

    t.reset();
    for (int i = 0; i < c; i++) {
        consumers.emplace_back(query_thread, i * ch_size, (i + 1) * ch_size);
    }


    for (auto& c : consumers) {
        c.join();
    }
    query_time.fetch_add(t.elapsed(), std::memory_order_relaxed);

    std::ofstream outf{"/home/rohitfeb641/vector-db/benchmarks/benchmark_nsw.txt", std::ios::app};
    outf << "Parameters(M = " << m << ", efConst = " << constr << ", efSearch: " << search << ")\n";

    double qt = query_time.load() / num_queries;
    outf << "MsPQ for " << db.getSize() << " vectors : " << qt << " Milliseconds\n";
    outf << "QPS for "  << db.getSize() << " vectors : " << static_cast<int>(1000.0 / qt) << " queries\n";
    outf.flush();
}