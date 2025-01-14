#include "../measure/measure.h"
#include "utils.hpp"

#include <iostream>
#include <fstream>

#define S 100
#define N 1500
#ifndef NUM_PT
#define NUM_PT 400
#endif
#define PRINT_EVERY 10


int main(int argc, char *argv[]) {
    // init the measurement library
    std::ofstream plaintexts("results/plaintexts");
    std::ofstream traces("results/traces.csv");
    std::ofstream ciphertexts("results/ciphertexts");
    std::ofstream last_round_key("results/last_round_key.txt");
    std::ifstream initial_key("results/initial_key.txt");

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

    __m128i key, roundKeys[11];
    initial_key >> key;
    generateRoundKeys(key, roundKeys);
    last_round_key << roundKeys[10] << std::endl;

    for (int pt = 0; pt < NUM_PT; pt++) {
        std::string ciphertext;
        char b[16];
        for (int i = 0; i < 16; i++) {
            b[i] = rand() % 256;
        }
        __m128i plaintext = _mm_loadu_si128((const __m128i*)b);
        plaintexts << plaintext << std::endl;

        // warm-up
        __asm__ __volatile__("mfence");
        for (int i = 0; i < 1000; ++i) {
            aesEncrypt(plaintext, roundKeys, &ciphertext);
        }
        __asm__ __volatile__("mfence");

        // measurement
        for (int s = 0; s < S; ++s) {
            Measurement start = measure();

            __asm__ __volatile__("mfence");
            for (int i = 0; i < N; ++i) {
                aesEncrypt(plaintext, roundKeys, &ciphertext);
            }
            __asm__ __volatile__("mfence");

            Measurement stop = measure();
            Sample sample = convert(start, stop);

            traces << sample.energy << ((s < S - 1) ? "," : "\n");
        }
        ciphertexts << ciphertext << std::endl;

        if ((pt + 1) % PRINT_EVERY == 0)
            std::cout << "Processed " << pt + 1 << " plaintexts" << std::endl;
    }

    return 0;
}
