#include "../measure/measure.h"

#include <iostream>
#include <fstream>
#include <ippcp.h>
#include <string.h>
#include <vector>
#include <pthread.h>
#include <atomic>

#define S 50
#define N 1000
#ifndef NUM_PT
#define NUM_PT 400
#endif
#define PRINT_EVERY 50
#define NUM_THREADS 4


std::atomic<int> threads_completed(0);
std::atomic<int> threads_ready(0);
std::atomic<bool> start_execution(false);


Ipp8u ptext[17], ctext[17];
IppsAESSpec* pAES;


void* run_workload(void* arg) {
    threads_ready++;

    while (!start_execution.load()) {
        // Busy wait
    }
    
    __asm__ __volatile__("mfence");
    for (int i = 0; i < N; ++i) {
        ippsAESEncryptECB(ptext, ctext, sizeof(ptext), pAES);
    }
    __asm__ __volatile__("mfence");

    threads_completed++;
    return NULL;
}


int main(int argc, char *argv[]) {
    // init the measurement library
    std::ofstream plaintexts("results/plaintexts");
    // std::ofstream ciphertexts("results/ciphertexts");
    std::ofstream traces("results/traces.csv");

    init();
    srand(time(NULL));

    // warmup
    for (int s = 0; s < 10; ++s) {
        __asm__ __volatile__("mfence");
        // Profile a tight loop of nops
        for (int i = 0; i < N; ++i) {
            __asm__ __volatile__("nop");
            __asm__ __volatile__("mfence");
        }
        __asm__ __volatile__("mfence");
    }

    const Ipp8u* key;
    if(strcmp(argv[1], "0")) {
        key = "\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00";
    } else {
        key = "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF";
    }
    
    int ctxSize;
    ippsAESGetSize(&ctxSize);
    pAES = (IppsAESSpec*)( new Ipp8u [ctxSize] );
    ippsAESInit(key, sizeof(key)-1, pAES, ctxSize);
    ptext[16] = '\x00';

    for (int pt = 0; pt < NUM_PT; pt++) {
        for (int i = 0; i < 16; i++)
            ptext[i] = 97 + (rand() % 26); // all lowercase letters
        plaintexts << ptext << std::endl;

        // warm-up
        __asm__ __volatile__("mfence");
        for (int i = 0; i < 100; ++i) {
            ippsAESEncryptECB(ptext, ctext, sizeof(ptext), pAES);
        }
        __asm__ __volatile__("mfence");

        // measurement
        for (int s = 0; s < S; ++s) {
            threads_completed = 0;
            threads_ready = 0;
            start_execution = false;

            std::vector<pthread_t> threads(NUM_THREADS);
            for(int t = 0; t < NUM_THREADS; ++t) {
                pthread_create(&threads[t], NULL, run_workload, NULL);
            }

            while (threads_ready.load() < NUM_THREADS) {
                // wait for threads to be ready
            }

            Measurement start = measure();
            start_execution = true;
            while (threads_completed.load() < NUM_THREADS) {
                // wait for threads to complete
            }
            Measurement stop = measure();
            for (int t = 0; t < NUM_THREADS; ++t) {
                pthread_join(threads[t], NULL);
            }
            Sample sample = convert(start, stop);
            traces << sample.energy << ((s < S - 1) ? "," : "\n");
        }
        // ciphertexts << ctext << std::endl;

        if ((pt + 1) % PRINT_EVERY == 0)
            std::cout << "Processed " << pt + 1 << " plaintexts" << std::endl;
    }

    return 0;
}
