#include "../measure/measure.h"
#include "utils.hpp"

#include <iostream>
#include <fstream>

#define S 100
#define N 1'000'000
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

    init();
    srand(time(NULL));

    // warmup
    for (int s = 0; s < 10; ++s) {
        Measurement start = measure();
        __asm__ __volatile__("mfence");
        // Profile a tight loop of nops
        for (int i = 0; i < N; ++i) {
        __asm__ __volatile__("nop");
        __asm__ __volatile__("mfence");
        }
        __asm__ __volatile__("mfence");
        Measurement stop = measure();
        Sample sample = convert(start, stop);
    }

    // parse the key
    char b[16];
    for (int i = 0; i < 16; i++) {
        b[i] = static_cast<char>(strtol(argv[i + 1], nullptr, 16));
    }
    __m128i key = _mm_set_epi8(b[15], b[14], b[13], b[12], b[11], b[10], b[9], b[8], b[7], b[6], b[5], b[4], b[3], b[2], b[1], b[0]);
    __m128i roundKeys[11];
    generateRoundKeys(key, roundKeys);
    last_round_key << roundKeys[10] << std::endl;

    for (int pt = 0; pt < NUM_PT; pt++) {
        // generate random plaintext
        std::string ciphertext;
        for (int i = 0; i < 16; i++) {
            b[i] = rand() % 256;
        }
        __m128i plaintext = _mm_set_epi8(b[15], b[14], b[13], b[12], b[11], b[10], b[9], b[8], b[7],b[6], b[5], b[4], b[3], b[2], b[1], b[0]);
        plaintexts << plaintext << std::endl;

        // measurement
        for (int s = 0; s < S; ++s) {
            Measurement start = measure();
            __asm__ __volatile__("mfence");

            // Profile a tight loop of nops
            for (int i = 0; i < N; ++i) {
                aesEncrypt(plaintext, roundKeys, &ciphertext);
            }

            __asm__ __volatile__("mfence");
            Measurement stop = measure();
            Sample sample = convert(start, stop);

            traces << sample.energy;
            if (s < S - 1) {
                traces << ",";
            }
        }
        traces << "\n";
        ciphertexts << ciphertext << std::endl;

        if ((pt + 1) % PRINT_EVERY == 0) {
            std::cout << "Processed " << pt + 1 << " plaintexts" << std::endl;
        }
    }

    return 0;
}
