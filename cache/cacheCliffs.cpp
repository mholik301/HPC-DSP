#include <iostream>
#include <vector>
#include <chrono>
#include <numeric>
#include <print>
#include <algorithm>
#include <random>
#include <format>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/*
For measuring latency, the idea is to allocate a block of memory, progressively bigger and bigger,
then roll the dice and access a random address.
If the block completely fits in L1, 100% accesses (after initial loading), are in L1.
If the block is 1.5xL1, 1/3 of accesses will miss and go to L2 etc.

For measuring bandwidth, instead of random access we do a sequential read + a trivial operation like
accumulate, and we measure how long it takes to complete for some number of bytes
 */

using namespace std::chrono;

// --- 1. Hardware Discovery (Win32 API) ---
struct Cache {
    int level;
    size_t size;
    size_t line;
};

std::vector<Cache> discover_caches() {
    std::vector<Cache> caches;
    DWORD sz = 0;
    GetLogicalProcessorInformation(nullptr, &sz);
    std::vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION> buf(sz / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION));
    GetLogicalProcessorInformation(buf.data(), &sz);
    for (auto& i : buf) {
        if (i.Relationship == RelationCache) 
            caches.push_back({ (int)i.Cache.Level, i.Cache.Size, i.Cache.LineSize });
    }
    std::sort(caches.begin(), caches.end(), [](auto& a, auto& b) { return a.level < b.level; });
    caches.erase(std::unique(caches.begin(), caches.end(), [](auto& a, auto& b) { 
        return a.level == b.level && a.size == b.size; 
    }), caches.end());
    return caches;
}


// --- 2. Measurements ---

// We use 'volatile' to force the compiler to actually perform the memory operations.
// To reduce the instruction bottleneck, we unroll the sum.
// Setting "cold" to true will just do one loop iteration, ensuring caches are filled
double measure_bandwidth(size_t bytes, bool cold) {
    const int iterations = cold ? 1 : 100;
    size_t count = bytes / sizeof(uint64_t);
    if (count < 8) count = 8;
    std::vector<uint64_t> data(count, 1);
    
    volatile uint64_t sum = 0;
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        for (size_t j = 0; j < count; j += 8) {
            sum += data[j]; sum += data[j+1];
            sum += data[j+2]; sum += data[j+3];
            sum += data[j+4]; sum += data[j+5];
            sum += data[j+6]; sum += data[j+7];
        }
    }
    
    auto end = high_resolution_clock::now();
    auto total_ns = duration_cast<nanoseconds>(end - start).count();
    // Return GB/s
    return ((double)bytes * iterations / 1e9) / (total_ns / 1e9);
}

// Pointer chasing to measure absolute silicon latency
double measure_latency(size_t bytes) {
    size_t count = bytes / sizeof(void*);
    if (count < 2) return 0;

    std::vector<void*> ptrs(count);
    std::vector<size_t> indices(count);
    std::iota(indices.begin(), indices.end(), 0);
    std::ranges::shuffle(indices, std::mt19937{std::random_device{}()});

    for (size_t i = 0; i < count - 1; ++i) 
        ptrs[indices[i]] = &ptrs[indices[i + 1]];
    ptrs[indices[count - 1]] = &ptrs[indices[0]];

    void** curr = (void**)ptrs.data();
    auto start = high_resolution_clock::now();
    
    constexpr int lat_iters = 1e6;
    for (int i = 0; i < lat_iters; ++i) {
        curr = (void**)*curr;
    }
    
    auto end = high_resolution_clock::now();
    if (curr == nullptr) [[unlikely]] std::print(" ");
    
    return (double)duration_cast<nanoseconds>(end - start).count() / lat_iters;
}

// --- 3. The Sweep Logic ---

int main() {
    std::print("--- Memory Hierarchy Rigorous Analysis (C++23) ---\n\n");

    auto caches = discover_caches();
    std::print("Step 1: System Discovery Results\n");
    for (auto& c : caches) {
        std::print(" > L{} Cache found: {} KB (Line: {} bytes)\n", c.level, c.size / 1024, c.line);
    }
    
    std::print("\nStep 2: Commencing Data Sweep (Calculating Hit/Miss Penalties)\n");
    std::print("Rationale: 'Cold' measures the cost of loading data from the layer below.\n");
    std::print("           'Warm' measures the peak speed of the current layer.\n\n");

    // Define the range of sizes to test: From 4KB to 256MB
    std::vector<size_t> test_sizes;
    for (size_t s = 4 * 1024; s <= 256 * 1024 * 1024; s *= 2) {
        test_sizes.push_back(s);
    }

    std::print("{:<12} | {:<12} | {:>15} | {:>15} | {:>12}\n", 
               "Data Size", "Fit Target", "Cold (GB/s)", "Warm (GB/s)", "Latency (ns)");
    std::print("{:-<75}\n", "");

    double last_warm_bw = 0;

    for (size_t bytes : test_sizes) {
        // Determine which cache this size fits into
        std::string target = "RAM";
        for (auto& c : caches) {
            if (bytes <= c.size) {
                target = std::format("L{}", c.level);
                break;
            }
        }

        double cold_bw = measure_bandwidth(bytes, true);
        double warm_bw = measure_bandwidth(bytes, false);
        double latency = measure_latency(bytes);

        // Highlight "Cliffs" (where performance drops significantly)
        std::string marker = "";
        if (last_warm_bw > 0 && warm_bw < (last_warm_bw * 0.7)) {
            marker = " <-- CLIFF";
        }
        last_warm_bw = warm_bw;

        std::print("{:>10} KB | {:<12} | {:>15.2f} | {:>15.2f} | {:>12.2f} {}\n", 
                   bytes / 1024, target, cold_bw, warm_bw, latency, marker);
    }

    std::print("\n--- Final Conclusions ---\n");
    std::print("1. Instruction Bottleneck: If Warm BW is constant (~2-5 GB/s), your CPU is bound by loop logic.\n");
    std::print("2. Prefetcher Effect: If Cold BW is nearly equal to Warm BW, the hardware is hiding the load cost.\n");
    std::print("3. Latency Truth: Look at the Latency column. It is the only metric that cannot be faked by the CPU.\n");

    return 0;
}

/*
--- Memory Hierarchy Rigorous Analysis (C++23) ---

Step 1: System Discovery Results
 > L1 Cache found: 32 KB (Line: 64 bytes)
 > L2 Cache found: 512 KB (Line: 64 bytes)
 > L3 Cache found: 16384 KB (Line: 64 bytes)

Step 2: Commencing Data Sweep (Calculating Hit/Miss Penalties)
Rationale: 'Cold' measures the cost of loading data from the layer below.
           'Warm' measures the peak speed of the current layer.

 Data Size    | Fit Target   |     Cold (GB/s) |     Warm (GB/s) | Latency (ns)
---------------------------------------------------------------------------
         4 KB | L1           |           20.48 |           29.26 |         0.99
         8 KB | L1           |           20.48 |           28.95 |         0.99
        16 KB | L1           |           27.31 |           29.20 |         1.00
        32 KB | L1           |           27.31 |           28.52 |         1.02 // L1
        64 KB | L2           |           27.31 |           28.58 |         2.25 // L2
       128 KB | L2           |           27.89 |           25.70 |         2.56
       256 KB | L2           |           28.19 |           27.47 |         2.79
       512 KB | L2           |           28.34 |           28.52 |         3.91  // L2
      1024 KB | L3           |           28.34 |           17.56 |         7.95  // L3 <-- Bandwidth CLIFF
      2048 KB | L3           |           28.42 |           16.30 |         9.27
      4096 KB | L3           |           28.28 |           15.83 |        10.11
      8192 KB | L3           |           21.89 |           16.17 |        17.42
     16384 KB | L3           |           13.80 |           14.07 |        29.13
     32768 KB | RAM          |           16.31 |           13.92 |        82.04
     65536 KB | RAM          |           13.97 |           13.99 |        79.19
    131072 KB | RAM          |           16.62 |           14.26 |        89.72
    262144 KB | RAM          |           16.96 |           13.91 |        87.74

--- Final Conclusions ---
1. Instruction Bottleneck: If Warm BW is constant (~2-5 GB/s), your CPU is bound by loop logic (did you forget to compile -O2 or -O3?)
2. Prefetcher Effect: If Cold BW is nearly equal to Warm BW, the hardware is hiding the load cost.
3. Look at the Latency column. It is the only metric that cannot be faked by the CPU.

Also note that L2 is shared between code and data, so you do not get all 512KB just for an array.
 */