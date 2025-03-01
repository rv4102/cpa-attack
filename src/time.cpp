#include "../measure/measure.h"
#include "utils.hpp"
#include <iostream>
#include <chrono>
#include <fstream>

int main() {
    init();

    std::ifstream initial_key("results/initial_key.txt");
    char b[16];
    for (int i = 0; i < 16; i++) {
        b[i] = rand() % 256;
    }
    __m128i plaintext = _mm_loadu_si128((const __m128i*)b);
    std::string ciphertext;
    __m128i key, roundKeys[11];
    initial_key >> key;
    generateRoundKeys(key, roundKeys);

    for(int loop_size = 1000; loop_size <= 1e5; loop_size += 100) {
        Measurement start = measure();
        auto begin = std::chrono::high_resolution_clock::now();
        // code block
        __asm__ __volatile__("mfence");
        for (int i = 0; i < loop_size; ++i) {
            aesEncrypt(plaintext, roundKeys, &ciphertext);
        }
        __asm__ __volatile__("mfence");
        auto end = std::chrono::high_resolution_clock::now();
        Measurement stop = measure();

        Sample sample = convert(start, stop);
        std::chrono::duration<double> duration = end - begin;

        if (duration.count() > 0.001) {
            std::cout << "Loop size " << loop_size << " is correct. Energy: " << sample.energy << std::endl;
        } else {
            std::cout << "Incorrect, energy measured: " << sample.energy << std::endl;
        }
    }

    return 0;
}