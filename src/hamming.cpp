#include "utils.hpp"

#include <iostream>
#include <fstream>


#define NUM_SUB_KEY 256
#ifndef NUM_PT
#define NUM_PT 400
#endif
#define PRINT_EVERY 50


int main(int argc, char *argv[]) {
    std::ifstream ciphertexts("results/ciphertexts");
    std::ofstream hamm[16];
    for (int i = 0; i < 16; i++) {
        hamm[i] = std::ofstream("results/hamm" + std::to_string(i) + ".csv");
    }

    for (int i = 0; i < NUM_PT; i++) {
        __m128i ciphertext;
        ciphertexts >> ciphertext;

        for (int j = 0; j < NUM_SUB_KEY; j++) {
            char b[16];
            for (int k = 0; k < 16; k++) {
                b[k] = j;
            }
            __m128i sub_key = _mm_loadu_si128((const __m128i*)b);
            __m128i sbox_inv = performInvSubBytes(_mm_xor_si128(ciphertext, sub_key));
            std::string inv_sbox_output = m128iToHexString(sbox_inv);

            for (int k = 0; k < 16; k++) {
                int byte = std::stoi(inv_sbox_output.substr(k*2, 2), nullptr, 16);
                hamm[k] << hammingWeight(byte) << ((j < NUM_SUB_KEY - 1) ? "," : "\n");
            }
        }

        if((i+1) % PRINT_EVERY == 0)
            std::cout << "Completed " << (i+1) << std::endl;
    }

    return 0;
}