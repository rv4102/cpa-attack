#include "../measure/measure.h"

#include <iostream>
#include <fstream>
#include <ippcp.h>
#include <string.h>
#include <vector>
#include <pthread.h>
#include <atomic>

#define S 5000
#define N 50000
#define NUM_THREADS 4


std::atomic<int> threads_completed(0);
std::atomic<int> threads_ready(0);
std::atomic<bool> start_execution(false);


Ipp8u ptext[17];
Ipp8u ctext[17];
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

    Ipp8u key[] = "\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00";

    if(strcmp(argv[1], "0")) {
        memset(ptext, 0x00, sizeof(ptext)-1);
    } else {
        memset(ptext, 0xFF, sizeof(ptext)-1);
    }
    ptext[16] = '\0';

    // std::cout << ptext << std::endl;
    
    int ctxSize;
    ippsAESGetSize(&ctxSize);
    pAES = (IppsAESSpec*)( new Ipp8u [ctxSize] );
    ippsAESInit(key, sizeof(key)-1, pAES, ctxSize);

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
        fprintf(stderr, "%f\n", sample.energy);
    }

    return 0;
}
