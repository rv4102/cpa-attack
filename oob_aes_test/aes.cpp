#include <iostream>
#include <ippcp.h>
#include <string.h>
#include <vector>
#include <pthread.h>
#include <atomic>

#define NUM_THREADS 4


std::atomic<int> threads_completed(0);
std::atomic<int> threads_ready(0);
std::atomic<bool> start_execution(false);


Ipp8u ptext[17], ctext[17];
IppsAESSpec* pAES;
int N = 50000;


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
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <key_type>" << std::endl;
        return 1;
    }

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

    Ipp8u key[17];
    if (strcmp(argv[1], "0")) {
        memset(key, 0x00, sizeof(key));
    } else {
        memset(key, 0xFF, sizeof(key)-1);
        key[16] = '\0';
    }
    
    int ctxSize;
    ippsAESGetSize(&ctxSize);
    pAES = (IppsAESSpec*)( new Ipp8u [ctxSize] );
    ippsAESInit(key, sizeof(key)-1, pAES, ctxSize);
    ptext[16] = '\0';

    unsigned int i = 0;
    while (1) {
        for (int i = 0; i < 16; i++)
            ptext[i] = 97 + (rand() % 26); // all lowercase letters

        // warm-up
        __asm__ __volatile__("mfence");
        for (int i = 0; i < 100; ++i) {
            ippsAESEncryptECB(ptext, ctext, sizeof(ptext), pAES);
        }
        __asm__ __volatile__("mfence");

        // measurement
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

        start_execution = true;
        while (threads_completed.load() < NUM_THREADS) {
            // wait for threads to complete
        }

        for (int t = 0; t < NUM_THREADS; ++t) {
            pthread_join(threads[t], NULL);
        }

        i++;
        std::cout << "AES " << i << " complete" << std::endl;
    }

    return 0;
}
