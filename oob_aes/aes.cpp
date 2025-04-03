#include "msr_handler.h"
#include <iostream>
#include <fstream>
#include <ippcp.h>
#include <string.h>
#include <unistd.h>
#include <vector>

#define PRINT_EVERY 1
#define NUM_THREADS 4


std::atomic<int> threads_completed(0);
std::atomic<int> threads_ready(0);
std::atomic<bool> start_execution(false);

Ipp8u ptext[17], ctext[17];
IppsAESSpec* pAES;
int num_plaintexts, S, N;


uint64_t hammingWeight(Ipp8u byte) {
    uint64_t count = 0;
    while (byte) {
        count += byte & 1;
        byte >>= 1;
    }
    return count;
}


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
    if (argc != 4) {
        perror("Correct format: ./aes <num_plaintext> <S> <N>");
        exit(1);
    }

    num_plaintexts = atoi(argv[1]);
    S = atoi(argv[2]);
    N = atoi(argv[3]);

    // Create a buffer to store plaintexts instead of writing them immediately
    std::vector<std::string> plaintexts_buffer;
    MSRHandler msr_handler(0); // Using CPU 0

    srand(time(NULL));

    // Disable PMC0 at the beginning of the attack
    msr_handler.disable_PMC0();

    Ipp8u key[17];
    memset(key, 0x00, sizeof(key));
    
    int ctxSize;
    ippsAESGetSize(&ctxSize);
    pAES = (IppsAESSpec*)(new Ipp8u[ctxSize]);
    ippsAESInit(key, sizeof(key)-1, pAES, ctxSize);
    ptext[16] = '\0';

    for (int pt = 0; pt < num_plaintexts; pt++) {
        for (int i = 0; i < 16; i++)
            ptext[i] = 97 + (rand() % 26); // all lowercase letters
        
        plaintexts_buffer.push_back(std::string((char*)ptext));

        // warm-up
        __asm__ __volatile__("mfence");
        for (int i = 0; i < 100; ++i) {
            ippsAESEncryptECB(ptext, ctext, sizeof(ptext)-1, pAES);
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

            // Signal to out-of-band measurement to start (A2)
            msr_handler.set_PMC0_lsb();
            
            // Small delay to ensure A2 has started measurement
            usleep(10000); // 10ms delay
            start_execution = true;
            while (threads_completed.load() < NUM_THREADS) {
                // wait for threads to complete
            }
            usleep(25000); // 25ms delay
            // Signal to out-of-band measurement to stop (A2)
            msr_handler.clear_PMC0_lsb();
        
            for (int t = 0; t < NUM_THREADS; ++t) {
                pthread_join(threads[t], NULL);
            }
        }

        if ((pt + 1) % PRINT_EVERY == 0)
            std::cout << "Processed " << pt + 1 << " plaintexts" << std::endl;
    }

    delete[] pAES;

    std::cout << "[+] Finished AES encryption" << std::endl;
    std::cout << "[+] Generating hamming weight model..." << std::endl;
    
    // Write all plaintexts to file at the end
    std::ofstream plaintext_file("results/plaintexts.txt");
    for (const auto& pt : plaintexts_buffer) {
        plaintext_file << pt << std::endl;
    }
    plaintext_file.close();

    // Open files for hamming weight model
    std::ofstream hamm[16];
    for (int i = 0; i < 16; i++) {
        hamm[i] = std::ofstream("results/hamm" + std::to_string(i) + ".csv");
    }

    // Process hamming weights directly from buffer rather than reading from file
    for (int i = 0; i < num_plaintexts; i++) {
        const std::string& plaintext = plaintexts_buffer[i];

        for (int j = 0; j < 16; j++) {
            Ipp8u plaintext_byte = plaintext[j];
            for (int k = 0; k < 256; k++) {
                Ipp8u key_byte_guess = (Ipp8u) k;
                hamm[j] << hammingWeight(plaintext_byte ^ key_byte_guess) << ((k < 255) ? "," : "\n");
            }
        }

        if((i+1) % PRINT_EVERY == 0)
            std::cout << "Completed " << (i+1) << std::endl;
    }

    return 0;
}