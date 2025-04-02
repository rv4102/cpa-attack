#include <iostream>
#include <fstream>
#include <ippcp.h>

#define PRINT_EVERY 100
int num_plaintexts;

uint64_t hammingWeight(Ipp8u byte) {
    uint64_t count = 0;
    while (byte) {
        count += byte & 1;
        byte >>= 1;
    }
    return count;
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        perror("Correct format: ./hamming <num_plaintexts>");
        exit(1);
    }

    num_plaintexts = atoi(argv[1]);

    // std::ifstream ciphertexts("results/ciphertexts");
    std::ifstream plaintexts("results/plaintexts.txt");
    std::ofstream hamm[16];
    for (int i = 0; i < 16; i++) {
        hamm[i] = std::ofstream("results/hamm" + std::to_string(i) + ".csv");
    }

    for (int i = 0; i < num_plaintexts; i++) {
        Ipp8u plaintext[17];
        plaintexts >> plaintext;

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