#include "../measure/measure.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <atomic>
#include <unistd.h>
#include <vector>

#define S 2000  // Number of samples to collect
#define N 25000000  // Number of iterations per thread

// Synchronization variables
std::atomic<int> threads_completed(0);
std::atomic<int> threads_ready(0);
std::atomic<bool> start_execution(false);

// Thread argument structure
struct ThreadArg {
    uint64_t factor;
    int thread_id;
};

// Thread function
void* run_workload(void* arg) {
    ThreadArg* targ = (ThreadArg*)arg;
    volatile uint64_t prod = 3;
    
    // Signal that this thread is ready
    threads_ready++;
    
    // Wait for the start signal
    while (!start_execution.load()) {
        // Busy wait
    }
    
    // Run exactly N iterations
    for (int i = 0; i < N; ++i) {
        asm volatile("imul %[factor], %[result]":[result]"=r"(prod):[factor]"r"(targ->factor));
    }
    
    // Signal that this thread has completed
    threads_completed++;
    
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("usage: %s factor num_threads\n", argv[0]);
        return -1;
    }

    uint64_t factor = strtol(argv[1], NULL, 10);
    int num_threads = strtol(argv[2], NULL, 10);

    printf("Running with factor=%lu on %d threads, %d iterations per thread\n", 
           factor, num_threads, N);
    
    // init the measurement library
    init();

    // measure imul with multiple threads for S samples
    for (int s = 0; s < S; ++s) {
        // Reset synchronization variables
        threads_completed = 0;
        threads_ready = 0;
        start_execution = false;
        
        // Create thread arguments and threads
        std::vector<ThreadArg> thread_args(num_threads);
        std::vector<pthread_t> threads(num_threads);
        
        // Create threads
        for (int t = 0; t < num_threads; ++t) {
            thread_args[t].factor = factor;
            thread_args[t].thread_id = t;
            pthread_create(&threads[t], NULL, run_workload, &thread_args[t]);
        }
        
        // Wait until all threads are ready
        while (threads_ready.load() < num_threads) {
            // Wait for threads to be ready
        }
        
        // Start measurement right before threads begin their workload
        Measurement start = measure();
        
        // Signal all threads to start execution simultaneously
        start_execution = true;
        
        // Wait until all threads have completed
        while (threads_completed.load() < num_threads) {
            // Wait for threads to complete
        }
        
        // Take final measurement after all threads have completed
        Measurement stop = measure();
        
        // Wait for all threads to finish
        for (int t = 0; t < num_threads; ++t) {
            pthread_join(threads[t], NULL);
        }
        
        // Process and output measurement
        Sample sample = convert(start, stop);
        fprintf(stderr, "%f\n", sample.energy);
    }

    return 0;
}