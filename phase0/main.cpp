#include <iostream>
#include <vector>
#include <chrono> 
#include <algorithm>

struct Point3D{
    float x, y, z;
};

void copy_vector(std::vector<Point3D>& a, std::vector<Point3D>& b) {
    // using namespace std::chorno_literals;
    auto st_copy = std::chrono::steady_clock::now();
    a = b;
    auto end_copy = std::chrono::steady_clock::now();
    std::chrono::duration t_copy = end_copy - st_copy;

    std::cout << "Copy time: " << t_copy << '\n';

    return;
}

void move_vector(std::vector<Point3D>& a, std::vector<Point3D>& b) {
    // using namespace std::chorno_literals;
    auto st_move = std::chrono::steady_clock::now();
    a = std::move(b);
    auto end_move = std::chrono::steady_clock::now();
    std::chrono::duration t_move = end_move - st_move;

    std::cout << "Move time: " << t_move << '\n';

    return;
}

void process_data([[maybe_unused]] const std::vector<Point3D>& a) {
    // a[0].x = 5.0f;
    return;
}

int main() {
    const int N = 10'000'000; // 10 million elements as per Phase 0 [cite: 102]

    std::cout << "--- Phase 0: Memory-Safe Vector Operations Benchmark ---\n";
    std::cout << "Generating 10M points for benchmark...\n\n";

    // --- Part 1: Capacity Optimization (Reserve vs Push_back) [cite: 104] ---
    
    // Benchmark A: With reserve()
    std::vector<Point3D> a;
    auto start_reserve = std::chrono::steady_clock::now();
    
    a.reserve(N); // Pre-allocate memory
    for(int i = 0; i < N; ++i) {
        a.push_back({1.0f, 2.0f, 3.0f});
    }
    
    auto end_reserve = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff_reserve = end_reserve - start_reserve;
    std::cout << "1. Initialization WITH reserve(): " << diff_reserve.count() << " s\n";


    // Benchmark B: Without reserve() (Dynamic Reallocation)
    std::vector<Point3D> b;
    auto start_no_reserve = std::chrono::steady_clock::now();
    
    // No reserve called here - will trigger multiple reallocations
    for(int i = 0; i < N; ++i) {
        b.push_back({4.0f, 5.0f, 6.0f});
    }
    
    auto end_no_reserve = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff_no_reserve = end_no_reserve - start_no_reserve;
    std::cout << "2. Initialization WITHOUT reserve(): " << diff_no_reserve.count() << " s\n";
    
    std::cout << "   (Expect reserve() to be significantly faster)\n\n";


    // --- Part 2: Move Semantics vs Copy [cite: 102] ---
    std::cout << "--- Testing Swap Performance ---\n";
    // Passing filled vectors 'a' and 'b' to your function

    std::vector<Point3D> c, d;
    copy_vector(c, b);
    move_vector(d, b); 


    // --- Part 3: Const-Correctness [cite: 103] ---
    std::cout << "\n--- Testing Const-Correctness ---\n";
    process_data(a);
    std::cout << "process_data executed successfully (Read-Only access).\n";

    return 0;
}