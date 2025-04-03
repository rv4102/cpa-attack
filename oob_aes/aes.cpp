#include "msr_handler.h"
#include <iostream>
#include <fstream>
#include <ippcp.h>
#include <string.h>
#include <unistd.h>
#include <vector>

#define PRINT_EVERY 1

uint64_t hammingWeight(Ipp8u byte) {
    uint64_t count = 0;
    while (byte) {
        count += byte & 1;
        byte >>= 1;
    }
    return count;
}

Ipp8u ptext[17], ctext[17];
IppsAESSpec* pAES;
int num_plaintexts, S, N;

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

    // warmup
    for (int s = 0; s < 10; ++s) {
        __asm__ __volatile__("mfence");
        for (int i = 0; i < N; ++i) {
            __asm__ __volatile__("nop");
            __asm__ __volatile__("mfence");
        }
        __asm__ __volatile__("mfence");
    }

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
        
        // Store plaintext in buffer instead of writing to file
        plaintexts_buffer.push_back(std::string((char*)ptext));

        // warm-up
        __asm__ __volatile__("mfence");
        for (int i = 0; i < 100; ++i) {
            ippsAESEncryptECB(ptext, ctext, sizeof(ptext)-1, pAES);
        }
        __asm__ __volatile__("mfence");

        // measurement
        for (int s = 0; s < S; ++s) {
            // Signal to out-of-band measurement to start (A2)
            msr_handler.set_PMC0_lsb();
            
            // Small delay to ensure A2 has started measurement
            usleep(500000); // 500ms delay
            
            // Execute AES encryption
            __asm__ __volatile__("mfence");
            for (int i = 0; i < N; ++i) {
                ippsAESEncryptECB(ptext, ctext, sizeof(ptext)-1, pAES);
            }
            __asm__ __volatile__("mfence");
            
            // Signal to out-of-band measurement to stop (A2)
            msr_handler.clear_PMC0_lsb();
            
            // Add a small delay between iterations
            usleep(2500000); // 2500ms delay
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