//
// Created by jason on 2/9/2023.
//


#include <iostream>
#include <chrono>
#include <fstream>
#include <vector>

#include "../memtable.h"
#include "../kv_store.h"
#include <random>

int main() {
    int arr[] = {
        100000,
        200000,
        300000,
        400000,
        500000,
        600000,
        700000,
        800000,
        900000,
        1000000
    };
    std::vector<int> input_sizes(arr, arr + sizeof(arr) / sizeof(int));
    std::vector<double> put_runtimes;
    std::vector<double> get_runtimes;
    std::vector<double> scan_all_runtimes;
    std::vector<double> scan_some_runtimes;

    std::cout << "Runtimes for PUT, GET, SCAN operation" << std::endl;
    for (int input_size : input_sizes) {

        auto memtable = Memtable((uint32_t ) 69420000);

        // Set up PUT operations
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < input_size; i++) {
            memtable.put((uint64_t) i, (uint64_t) 69420);
        }
        auto stop = std::chrono::high_resolution_clock::now();
        
        // Calculate duration
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
        double runtime = duration.count() / 1000000.0; // convert to seconds

        // Store the runtime
        put_runtimes.push_back(runtime);

        // Set up GET operations
        start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < input_size; i++)
        {
            memtable.get((uint64_t)i);
        }
        stop = std::chrono::high_resolution_clock::now();

        // Calculate duration
        duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
        runtime = duration.count() / 1000000.0; // convert to seconds

        // Store the runtime
        get_runtimes.push_back(runtime);

        // Set up SCAN ALL operations
        start = std::chrono::high_resolution_clock::now();
        memtable.scan(0, 99999999);
        stop = std::chrono::high_resolution_clock::now();

        // Calculate duration
        duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
        runtime = duration.count() / 1000000.0; // convert to seconds

        // Store the runtime
        scan_all_runtimes.push_back(runtime);

        // Set up SCAN SOME operations
        start = std::chrono::high_resolution_clock::now();
        memtable.scan(10000, input_size - 10000);
        stop = std::chrono::high_resolution_clock::now();

        // Calculate duration
        duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
        runtime = duration.count() / 1000000.0; // convert to seconds

        // Store the runtime
        scan_some_runtimes.push_back(runtime);
    }

    // Output the runtime
    for (int i = 0; i < input_sizes.size(); ++i) {
        std::cout << "PUT Operation Input size: " << input_sizes[i] << "; Time taken: " << put_runtimes[i] << " seconds" << std::endl;
    }
    for (int i = 0; i < input_sizes.size(); ++i)
    {
        std::cout << "GET Operation Input size: " << input_sizes[i] << "; Time taken: " << get_runtimes[i] << " seconds" << std::endl;
    }
    for (int i = 0; i < input_sizes.size(); ++i)
    {
        std::cout << "SCAN ALL Operation Input size: " << input_sizes[i] << "; Time taken: " << scan_all_runtimes[i] << " seconds" << std::endl;
    }
    for (int i = 0; i < input_sizes.size(); ++i)
    {
        std::cout << "SCAN SOME Operation Input size: " << input_sizes[i] << "; Time taken: " << scan_some_runtimes[i] << " seconds" << std::endl;
    }

    // Write the results to a CSV file
    std::ofstream put_file("experiments/phase1_put_runtimes.csv");
    put_file << "Input size,Runtime\n";
    for (int i = 0; i < input_sizes.size(); ++i) {
        put_file << input_sizes[i] << "," << put_runtimes[i] << "\n";
    }
    put_file.close();

    std::ofstream get_file("experiments/phase1_get_runtimes.csv");
    get_file << "Input size,Runtime\n";
    for (int i = 0; i < input_sizes.size(); ++i)
    {
        get_file << input_sizes[i] << "," << get_runtimes[i] << "\n";
    }
    get_file.close();

    std::ofstream scan_all_file("experiments/phase1_scan_all_runtimes.csv");
    scan_all_file << "Input size,Runtime\n";
    for (int i = 0; i < input_sizes.size(); ++i)
    {
        scan_all_file << input_sizes[i] << "," << scan_all_runtimes[i] << "\n";
    }
    scan_all_file.close();

    std::ofstream scan_some_file("experiments/phase1_scan_some_runtimes.csv");
    scan_some_file << "Input size,Runtime\n";
    for (int i = 0; i < input_sizes.size(); ++i)
    {
        scan_some_file << input_sizes[i] << "," << scan_some_runtimes[i] << "\n";
    }
    scan_some_file.close();

    return 0;
}
