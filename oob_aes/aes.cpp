#include "msr_handler.h"
#include <iostream>
#include <fstream>
#include <ippcp.h>
#include <string.h>
#include <unistd.h>

#define PRINT_EVERY 100

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

    std::ofstream plaintexts("results/plaintexts.txt");
    
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
        plaintexts << ptext << std::endl;

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
            usleep(1000); // 1ms delay
            
            // Execute AES encryption
            __asm__ __volatile__("mfence");
            for (int i = 0; i < N; ++i) {
                ippsAESEncryptECB(ptext, ctext, sizeof(ptext)-1, pAES);
            }
            __asm__ __volatile__("mfence");
            
            // Signal to out-of-band measurement to stop (A2)
            msr_handler.clear_PMC0_lsb();
            
            // Add a small delay between iterations
            usleep(5000); // 5ms delay
        }

        if ((pt + 1) % PRINT_EVERY == 0)
            std::cout << "Processed " << pt + 1 << " plaintexts" << std::endl;
    }

    delete[] pAES;
    return 0;
}