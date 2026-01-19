#include <iostream>
#include <chrono>
#include <vector>

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

struct Point3D
{
    double x, y, z;
};

// SLOWEST: Manually swaps every single item one by one.
// Like moving 10 million bricks by hand.
void copy_swap_vectors(std::vector<Point3D>& a, std::vector<Point3D>& b) {
    for (int i{0}; i < a.size(); i++) {
        std::swap(a[i], b[i]);
    }
}

// FAST: Transfers ownership.
// Like giving someone the keys to a truck full of bricks.
void move_vectors(std::vector<Point3D>& a, std::vector<Point3D>& b) {
    a = std::move(b);
}

// INSTANT: Just swaps the memory addresses.
// Like two people trading trucks without touching the cargo.
void pure_swap_vectors(std::vector<Point3D>& a, std::vector<Point3D>& b) {
    std::swap(a, b);
}

int main() {
    Timer t{};

    std::vector<Point3D> a1;
    const int len{10000000};
    
    // INFO: Reserve tells the computer exactly how much memory to set aside.
    // RESULT: This makes adding elements much faster.
    a1.reserve(len);
    for (int i{0}; i < len; i++) {
        a1.push_back({1.0 * i, 2.0 * i , 3.0 * i});
    }
    std::cout << "Reserve with pushback Time: " << t.elapsed() << " mili seconds\n";
    t.reset();

    std::vector<Point3D> a4;
    a4.reserve(len);
    // INFO: Emplace back builds the data directly inside the list's memory.
    for (int i{0}; i < len; i++) {
        a4.emplace_back(1.0 * i, 2.0 * i , 3.0 * i);
    }
    std::cout << "Reserve with emplace back Time: " << t.elapsed() << " mili seconds\n";
    t.reset();

    std::vector<Point3D> a2;
    // RESULT: Much slower because the computer must keep stopping to 
    // find bigger memory spots as the list grows.
    for (int i{0}; i < len; i++) {
        a2.emplace_back(1.0 * i, 2.0 * i , 3.0 * i);
    }
    std::cout << "Emplace back Time: " << t.elapsed() << " mili seconds\n";
    t.reset();

    std::vector<Point3D> a3;
    for (int i{0}; i < len; i++) {
        a3.push_back({1.0 * i, 2.0 * i , 3.0 * i});
    }
    std::cout << "Pushback Time: " << t.elapsed() << " mili seconds\n";
    t.reset();

    // RESULT: Extremely slow because it processes 10,000,000 individual items.
    copy_swap_vectors(a1, a2);
    std::cout << "Using loop assignment Time: " << t.elapsed() << " mili seconds\n";
    t.reset();

    // RESULT: Near 0ms. It only changes where the list "points" in memory.
    pure_swap_vectors(a1, a2);
    std::cout << "Using std::swap assignment Time: " << t.elapsed() << " mili seconds\n";
    t.reset();
    
    // RESULT: Fast, but takes a few ms because it has to delete the 
    // old data that was in 'a1' before taking 'a2's data.
    move_vectors(a1, a2);
    std::cout << "Using move assignment Time: " << t.elapsed() << " mili seconds\n";
    t.reset();
}