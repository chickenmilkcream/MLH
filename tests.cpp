//
// Created by jason on 2/9/2023.
//


#include <iostream>
#include <chrono>
#include <fstream>
#include <vector>

#include "memtable.h"
#include "kv_store.h"
#include <random>

int main() {
    std::vector<int> input_sizes = {100000, 200000, 300000, 400000, 500000};
    std::vector<double> runtimes;

    for (int input_size : input_sizes) {

        auto kv_store = Memtable((uint32_t ) 69420000);

        auto start = std::chrono::high_resolution_clock::now();

        for (int i; i<input_size; i++) {
//            std::mt19937 generator(std::random_device{}());
//            std::uniform_int_distribution<int> distribution(1, 10000000);
//            int random_integer = distribution(generator);
//            random_integer = i;
            kv_store.put((uint64_t )i, (uint64_t ) 69420);
        }

        auto stop = std::chrono::high_resolution_clock::now();

        // Calculate duration
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
        double runtime = duration.count() / 1000000.0; // convert to seconds

        // Store the runtime
        runtimes.push_back(runtime);

        // Output the runtime
        std::cout << "Input size: " << input_size << "; Time taken: " << runtime << " seconds" << std::endl;
        std::cout << kv_store.size << std::endl;


    }

    // Write the results to a CSV file
    std::ofstream file("runtimes.csv");
    file << "Input size,Runtime\n";
    for (int i = 0; i < input_sizes.size(); ++i) {
        file << input_sizes[i] << "," << runtimes[i] << "\n";
    }
    file.close();

    return 0;
}

