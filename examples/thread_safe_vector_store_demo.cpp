#include <thread>
#include <string>
#include <vector>
#include <atomic>
#include <syncstream>
#include "vectorDB/ThreadSafeVectorStore.hpp"
#include "vectorDB/utils/Importer.hpp"

std::string dir = "/home/rohitfeb641/vector-db/sift1M/";

auto sift_res = Importer::import_sift1m(dir + "sift_base.fvecs", dir + "sift_query.fvecs", dir + "sift_groundtruth.ivecs").take_ok_value();

const std::vector<std::vector<float>>& queries{sift_res.queries};
const std::vector<std::vector<std::uint32_t>>& truths{sift_res.truths};
const std::vector<std::uint32_t>& truth_k{sift_res.truth_k};
std::vector<Vector> vectors{std::move(sift_res.vectors)};

// std::size_t num_queries{100};
std::size_t insert_sz = vectors.size();
std::size_t query_sz = queries.size();

std::atomic<int> pid{0};
std::atomic<int> cid{0};


void producer(ThreadSafeVectorStore& vdb) {
    thread_local int id = pid++;
    int current_iter{0};
    while (current_iter < 100000) {
        vdb.insert(current_iter + (100000 * id), vectors[current_iter % insert_sz]);
        current_iter++;

        if (current_iter % 10000 == 0) {
            std::osyncstream(std::cout) << "Reached for producer" + std::to_string(id) + ": " + std::to_string(current_iter) << '\n';
        }

    }
}

void consumer(const ThreadSafeVectorStore& vdb) {
    thread_local int id = cid++;
    int current_iter{0};

    while (current_iter < 100) {
        int my_k = truth_k[current_iter % query_sz];
        vdb.query(queries[current_iter % query_sz], my_k, Metric::L2);
        current_iter++;

        if (current_iter % 10 == 0) {
            std::osyncstream(std::cout) << "Reached for consumer" + std::to_string(id) + ": " + std::to_string(current_iter) << '\n';
        }

    }
}

int main() {
    ThreadSafeVectorStore vdb;
    // std::cout << "Insert: " << insert_sz << ' ' << query_sz << '\n';

    std::vector<std::thread> producers;
    for (int i{0}; i < 10; i++) {
        producers.emplace_back(producer, std::ref(vdb));
    }

    std::vector<std::thread> consumers;
    for (int i{0}; i < 4; i++) {
        consumers.emplace_back(consumer, std::ref(vdb));
    }

    for (auto& p : producers) {
        p.join();
    }

    for (auto& c : consumers) {
        c.join();
    }

    std::cout << "VDB SIZE: " << vdb.size().ok_value();
}