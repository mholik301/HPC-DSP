#include <algorithm>
#include <atomic>
#include <chrono>
#include <iostream>
#include <numeric>
#include <print>
#include <random>
#include <thread>
#include <vector>


// code can also be, cache-friendly code, not just data: L2 shares code and data, so similar things we do
// for data we can do for code too:
// linear data structures -> avoiding jumping to distant functions
// avoiding pointer chasing -> avoiding indirect (virtual) jumps
// minimizing size of working set -> minimizing amount of code in hot loops


// despite our benchmarking for-loop being able to change the size of our benchmark data container
// (see "pointer aliasing" in printerHelper.h), we can reassure the optimizer that it won't by
// manually hoisting out the '; i<data.size();' from the loop (or use a range-based for)


using namespace std::chrono;

const size_t L2_LIMIT = 512 * 1024;    		// 512 KB
const size_t L3_LIMIT = 16 * 1024 * 1024;   // 16 MB

// --- EXPERIMENT 1: AoS vs SoA ---

// BAD: Array of Structures (AoS)
// Metadata "bloats" the cache line, so we load 128 bytes to use 12.
struct ParticleAoS {
    float x, y, z;        // Hot data (12 bytes)
    float vx, vy, vz;     // Hot data (12 bytes)
    char padding[104];    // Metadata/Bloat (104 bytes)
};

// GOOD: Structure of Arrays (SoA)
// We only load what we need. 100% Cache Line Utilization.
struct ParticleSoA {
    std::vector<float> x, y, z, vx, vy, vz;
    std::vector<char> metadata; // Kept in a separate "cold" bucket
};

void run_aos_vs_soa() {
    std::print("Exp 1: AoS vs SoA (1M Particles)\n");

    const size_t COUNT = 1'000'000;
    std::vector<ParticleAoS> aos(COUNT);

    // Test AoS (Bad)
    auto s1 = high_resolution_clock::now();
    for (auto& p : aos) {
        p.x += p.vx; p.y += p.vy; p.z += p.vz;
    }
    auto e1 = high_resolution_clock::now();

    // Test SoA (Good)
    ParticleSoA soa;
    soa.x.resize(COUNT); soa.y.resize(COUNT); soa.z.resize(COUNT);
    soa.vx.resize(COUNT); soa.vy.resize(COUNT); soa.vz.resize(COUNT);

    auto s2 = high_resolution_clock::now();
    for (size_t i = 0; i < COUNT; ++i) {
        soa.x[i] += soa.vx[i];
        soa.y[i] += soa.vy[i];
        soa.z[i] += soa.vz[i];
    }
    auto e2 = high_resolution_clock::now();


    std::print(" > Bad (AoS):  {:>10} ns\n", duration_cast<nanoseconds>(e1 - s1).count());
    std::print(" > Good (SoA): {:>10} ns\n", duration_cast<nanoseconds>(e2 - s2).count());
    std::print(" Conclusion: SoA is faster because it prevents 'Metadata Bloat' from choking the Cache Lines.\n\n");
}

// --- EXPERIMENT 2: Sequential vs. Random ---

// Despite our data only being read once, leading us to expect the cache-friendliness will not have any effect,
// modern CPUs have prefetchers: they notice memory access patterns and will start preloading data they
// think we will need.
// So, cache locality is important even for one-pass algorithms

void run_sequential_vs_random() {
    std::print("Exp 2: Sequential vs. Random (64MB Scan)\n");

    const size_t SIZE = 64 * 1024 * 1024; // 64MB (Well into RAM)
    std::vector<int> data(SIZE / sizeof(int), 1);
    std::vector<size_t> indices(data.size());
    std::iota(indices.begin(), indices.end(), 0);

    // BAD: Random Access
    std::shuffle(indices.begin(), indices.end(), std::mt19937{std::random_device{}()});
    volatile long long sum = 0;

    auto s1 = high_resolution_clock::now();
    for (size_t i = 0; i < indices.size(); ++i) sum += data[indices[i]];
    auto e1 = high_resolution_clock::now();

    // GOOD: Sequential Access
    sum = 0;
    auto s2 = high_resolution_clock::now();
    for (size_t i = 0; i < data.size(); ++i) sum += data[i];
    auto e2 = high_resolution_clock::now();


    std::print(" > Bad (Random): {:>10} ns\n", duration_cast<nanoseconds>(e1 - s1).count());
    std::print(" > Good (Seq):   {:>10} ns\n", duration_cast<nanoseconds>(e2 - s2).count());
    std::print(" Conclusion: Sequential is faster because the CPU predicts your next move and pre-loads the data.\n\n");
}

// --- EXPERIMENT 3: Tiling ---

// We will perform two passes over a large dataset.
// If the dataset is too big, the second pass must fetch from RAM again.
// If we "Tile", we finish both passes while the data is still in L2.

void run_tiling() {
    std::print("Exp 3: Naive vs. Tiled (32MB Double Pass)\n");

    const size_t TOTAL_SIZE = 32 * 1024 * 1024; // 32MB
    std::vector<float> data(TOTAL_SIZE / sizeof(float), 1.0f);

    // BAD: Two full passes over 32MB (Evicts itself from L3 twice)
    auto s1 = high_resolution_clock::now();
    for (auto& x : data) x *= 2.0f;
    for (auto& x : data) x += 1.0f;
    auto e1 = high_resolution_clock::now();

    // GOOD: Tiled access (Processes in 512KB chunks)
    auto s2 = high_resolution_clock::now();
    const size_t CHUNK = L2_LIMIT / sizeof(float);
    for (size_t i = 0; i < data.size(); i += CHUNK) {
        size_t end = std::min(i + CHUNK, data.size());
        for (size_t j = i; j < end; ++j) data[j] *= 2.0f;
        for (size_t j = i; j < end; ++j) data[j] += 1.0f;
    }
    auto e2 = high_resolution_clock::now();


    std::print(" > Bad (Naive): {:>10} ns\n", duration_cast<nanoseconds>(e1 - s1).count());
    std::print(" > Good (Tiled):{:>10} ns\n", duration_cast<nanoseconds>(e2 - s2).count());
    std::print(" Conclusion: Tiling keeps the 'Working Set' in L2, avoiding the 100ns round-trip to RAM.\n\n");
}

// --- EXPERIMENT 4: False Sharing ---

struct BadCounter {
    std::atomic<uint64_t> countA{0}; // uint64 matches the size of std::hardware_destructive_interference_size
    std::atomic<uint64_t> countB{0}; // see isolatedThreads.cpp
    // These two atomics sit on the SAME 64-byte cache line.
};

struct GoodCounter {
    alignas(64) std::atomic<uint64_t> countA{0};
    alignas(64) std::atomic<uint64_t> countB{0};
    // These are forced onto SEPARATE cache lines.
};

void run_false_sharing() {
    std::print("Exp 4: False Sharing (Contended Atomics)\n");

    const int ITY = 10'000'000;
    
    // BAD: Threads fight over the same cache line
    BadCounter bc;
    auto s1 = high_resolution_clock::now();
    std::thread t1([&] { for(int i=0; i<ITY; ++i) bc.countA++; });
    std::thread t2([&] { for(int i=0; i<ITY; ++i) bc.countB++; });
    t1.join(); t2.join();
    auto e1 = high_resolution_clock::now();

    // GOOD: Threads have their own cache lines
    GoodCounter gc;
    auto s2 = high_resolution_clock::now();
    std::thread t3([&] { for(int i=0; i<ITY; ++i) gc.countA++; });
    std::thread t4([&] { for(int i=0; i<ITY; ++i) gc.countB++; });
    t3.join(); t4.join();
    auto e2 = high_resolution_clock::now();


    std::print(" > Bad (Shared Line): {:>10} ns\n", duration_cast<nanoseconds>(e1 - s1).count());
    std::print(" > Good (Padded):     {:>10} ns\n", duration_cast<nanoseconds>(e2 - s2).count());
    std::print(" Conclusion: Parallel code slows down if cores fight for the same 64-byte silicon chunk.\n\n");
}

// --- EXPERIMENT 5: Branch Prediction ---

void run_branch_prediction() {
    std::print("Exp 5: Branch Prediction (Sorted vs Unsorted)\n");

    const size_t SIZE = 10'000'000;
    std::vector<int> data(SIZE);
    for(auto& x : data) x = rand() % 256;

    auto work = [](std::vector<int>& v) {
        long long sum = 0;
        for (auto x : v) if (x >= 128) sum += x;
        return sum;
    };

    // BAD: Unsorted data (Random branches)
    auto s1 = high_resolution_clock::now();
    volatile long long res1 = work(data);
    auto e1 = high_resolution_clock::now();
    std::cout << "res1=" << res1 << '\n'; // using the data so the loop is not optimized out

    // GOOD: Sorted data (Predictable branches) or partitioned data (no branches)
    std::sort(data.begin(), data.end());
    auto s2 = high_resolution_clock::now();
    volatile long long res2 = work(data);
    auto e2 = high_resolution_clock::now();
    std::cout << "res2=" << res2 << '\n'; // using the data so the loop is not optimized out


    std::print(" > Bad (Random):  {:>10} ns\n", duration_cast<nanoseconds>(e1 - s1).count());
    std::print(" > Good (Sorted): {:>10} ns\n", duration_cast<nanoseconds>(e2 - s2).count());
    std::print(" Conclusion: The CPU 'guesses' the result of 'if' statements. Random data breaks the guesser.\n\n");
}

// --- EXPERIMENT 6: Dependency Chains vs ILP (Instruction Level Parallelism) ---

void run_dependency_chains() {
    std::print("Exp 6: Dependency Chains (ILP)\n");

    const int ITY = 100'000'000;
    double a = 1.1, b = 1.1, c = 1.1, d = 1.1;

    // BAD: Serial Dependency (Each line needs the result of the previous)
    auto s1 = high_resolution_clock::now();
    for(int i=0; i<ITY; ++i) {
        a = a * b; // Result used in next line...
        a = a * c; 
        a = a * d;
        if(a > 1e10) a = 1.1;
    }
    auto e1 = high_resolution_clock::now();
    std::cout << "a=" << a << '\n'; // using the data so the loop is not optimized out

    // GOOD: Parallel Chains (Independent calculations)
    a = 1.1; b = 1.1; c = 1.1; d = 1.1;
    auto s2 = high_resolution_clock::now();
    for(int i=0; i<ITY; ++i) {
        a = a * b; // Independent
        c = c * d; // Independent - CPU can do both at the same time
        if(a > 1e10) a = 1.1;
        if(c > 1e10) c = 1.1;
    }
    auto e2 = high_resolution_clock::now();
    std::cout << "a=" << a << '\n';


    std::print(" > Bad (Dependent):  {:>10} ns\n", duration_cast<nanoseconds>(e1 - s1).count());
    std::print(" > Good (Parallel):  {:>10} ns\n", duration_cast<nanoseconds>(e2 - s2).count());
    std::print(" Conclusion: Modern CPUs have multiple 'math units'. Independent lines run in parallel.\n\n");
}

// TODO: experiment with ALU arithmetic type speeds. For instance, accumulating 1e7 ints might
// actually be faster then 1e7 uint8, despite unsigned chars fitting better into cache, if the
// ALU has better int accumulators



int main() {
    run_aos_vs_soa();
    run_sequential_vs_random();
    run_tiling();

    run_false_sharing();
    run_branch_prediction();
    run_dependency_chains();
    return 0;
}
