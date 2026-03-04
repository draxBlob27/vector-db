#include <string>
#include <algorithm>
#include <vector>
#include <random>
#include <iostream>
#include <cstdint>
#include <chrono>
#include "archive.hpp"

class Timer {
private:
    using millisec = std::chrono::duration<double, std::ratio<1, 1000>>;
    using Clock = std::chrono::high_resolution_clock;

    std::chrono::time_point<Clock> m_beg{Clock::now()};

public:
    void reset() {
        m_beg = Clock::now();
    }

    double elapsed() const {
        return std::chrono::duration_cast<millisec>(Clock::now() - m_beg).count();
    }
};

std::vector<std::vector<double>> generateData(const int dim, const int count, bool isZero=0)
{
    std::vector<std::vector<double>> res(count, std::vector<double>(dim, 0));
    if (isZero)
        return res;

    std::mt19937 mt{ std::random_device{}()};
    std::uniform_real_distribution<float> unf;

    
    for (int i{0}; i < count; i++) {
        for (int j{0}; j < dim; j++) {
            res[i][j] = unf(mt);
        }
    }

    return res;
}

void description() {
        std::cout << R"(
Commands:
        ├── create <file> <dim> <count> [--random]
        ├── load <file> [--verify]
        ├── info <file>
        ├── verify <file>
        └── append <file> <new_vectors>)" << '\n';
}

int main(int argc, char **argv)
{
    Timer t{};
    try
    {
        if (argc < 3 || argc > 6)
        {
            // call help
            std::cout << "Length inappropriate\n";
            description();
        }
        else
        {
            VectorArchive vecd{};
            const std::string command{argv[1]};
            if (command == "create")
            {
                if (argc == 5) { //zero initialised
                    const std::string filepath{argv[2]};
                    if (argv[3][0] == '-') {
                        std::cout << "Dimension can't be negative.";
                        return 1;
                    }
                    const std::uint32_t dim{static_cast<std::uint32_t>(std::stoul(argv[3]))};
                    if (argv[4][0] == '-') {
                        std::cout << "Count can't be negative.";
                        return 1;
                    }
                    const std::uint64_t count{std::stoull(argv[4])};

                    vecd.save(filepath, generateData(dim, count, 1));
                } else if (argc == 6) { //random data
                    const std::string filepath{argv[2]};
                    if (argv[3][0] == '-') {
                        std::cout << "Dimension can't be negative.";
                        return 1;
                    }
                    const std::uint32_t dim{static_cast<std::uint32_t>(std::stoul(argv[3]))};
                    if (argv[4][0] == '-') {
                        std::cout << "Count can't be negative.";
                        return 1;
                    }
                    const std::uint64_t count{std::stoull(argv[4])};

                    std::string isRandom{argv[5]};

                    if (isRandom == "--random") {
                        std::vector<std::vector<double>> data{generateData(dim, count, 0)};
                        t.reset();
                        vecd.save(filepath, data);
                        std::cout << t.elapsed() << '\n';
                    } else {
                        std::cout << "Invalid flag with save.";
                    }

                } else {
                    //call help
                    std::cout << "Error in create\n";
                    description();
                }
            }
            else if (command == "append") {
                if (argc == 4) {
                    const std::string filepath{argv[2]};
                    //argv[3] will be holding vectors
                    VectorArchive::FileInfo f_info{vecd.info(filepath)};
                    vecd.append(filepath, generateData(f_info.dim, f_info.count, 1)); //known, will update later
                } else {
                    //call help
                    std::cout << "Error in append\n";
                    description();
                }
            }
            else if (command == "load") {
                if (argc == 3) {
                    std::string filepath{argv[2]};
                    vecd.load(filepath, false);
                } else if (argc == 4) {
                    std::string filepath{argv[2]};
                    std::string isVerify{argv[3]};

                    if (isVerify == "--verify") {
                        if (vecd.verify(filepath)) {
                            vecd.load(filepath, true);
                        } else {
                            std::cout << "Verification failed\n";
                        }
                    } else {
                        std::cout << "Invalid flag with verify.";
                    }
                } else {
                    description();
                }
            }
            else if (command == "info") {
                if (argc == 3) {
                    std::string filepath{argv[2]};
                    std::cout << vecd.info(filepath);
                } else {
                    description();
                }
            }
            else if (command == "verify") {
                if (argc == 3) {
                    std::string filepath{argv[2]};
                    if (vecd.verify(filepath)) {
                        std::cout << "Verification OK\n";
                    } else {
                        std::cout << "Verification failed\n";
                    }
                } else {
                    description();
                }
            } else {
                description();
            }
        }
    }
    catch (const ArchiveError& exception)
    {
        std::cerr << exception.what() << '\n';
    }
    catch (const std::exception& exception) {
        std::cerr << exception.what() << '\n';
    }
}