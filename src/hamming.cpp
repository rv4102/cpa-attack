#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <emmintrin.h>
#include <wmmintrin.h>
#include <immintrin.h>


#define NUM_SUB_KEY 256
#ifndef NUM_PT
#define NUM_PT 2'000'000
#endif
#define PRINT_EVERY 100'000

// globals
// https://www.intel.com/content/dam/doc/white-paper/advanced-encryption-standard-new-instructions-set-paper.pdf
const __m128i ZERO = _mm_setzero_si128();
const __m128i ISOLATE_SROWS_MASK = 
  _mm_set_epi32(0x0B06010C,0x07020D08, 0x030E0904, 0x0F0A0500);
const  __m128i ISOLATE_SBOX_MASK = 
  _mm_set_epi32(0x0306090C, 0x0F020508, 0x0B0E0104, 0x070A0D00);

__m128i performSubBytes(const __m128i &input) {
  auto temp = _mm_shuffle_epi8(input, ISOLATE_SBOX_MASK);
  temp = _mm_aesenclast_si128(temp, ZERO);
  return temp;
}

__m128i performInvSubBytes(const __m128i &input) {
  auto temp = _mm_shuffle_epi8(input, ISOLATE_SROWS_MASK);
  temp = _mm_aesdeclast_si128(temp, ZERO);

  return temp;
}

int hammingWeight(int byte) {
  int count = 0;
  while (byte) {
    count += byte & 1;
    byte >>= 1;
  }
  return count;
}

std::string m128iToHexString(const __m128i &value) {
  std::ostringstream oss;
  const uint8_t *bytes = reinterpret_cast<const uint8_t *>(&value);
  for (int i = 0; i < 16; ++i) {
    oss << std::hex << std::setfill('0') << std::setw(2)
        << static_cast<int>(bytes[i]);
  }
  return oss.str();
}

__m128i hexStringToM128i(const std::string &hexString) {
    uint8_t bytes[16];
    for (int i = 0; i < 16; ++i) {
        std::string byteString = hexString.substr(i * 2, 2);
        bytes[i] = static_cast<uint8_t>(std::stoi(byteString, nullptr, 16));
    }

    // Cast the bytes array to __m128i
    return _mm_loadu_si128(reinterpret_cast<const __m128i*>(bytes));
}

int main(int argc, char *argv[]) {
  // if (argc != 2) {
  //   std::cerr << "Usage: hamming <key>" << std::endl;
  //   return 1;
  // }

  // uint8_t key_idx = std::stoi(argv[1], nullptr, 10);

  std::ifstream ciphertexts("results/ciphertexts");
  std::string name[16];
  std::ofstream hamm[16];
  for (int i = 0; i < 16; i++) {
    name[i] = "results/hamm" + std::to_string(i) + ".csv";
    hamm[i] = std::ofstream(name[i]);
  }

  std::string temp;
  for (int i = 0; i < NUM_PT; i++) {
    ciphertexts >> temp;
    __m128i ciphertext = hexStringToM128i(temp);

    for (int j = 0; j < NUM_SUB_KEY; j++) {
      __m128i sub_key = _mm_set_epi8(j,j,j,j,j,j,j,j,j,j,j,j,j,j,j,j);
      __m128i sbox_inv = performInvSubBytes(_mm_xor_si128(ciphertext, sub_key));
      std::string inv_sbox_output = m128iToHexString(sbox_inv);

      for (int k = 0; k < 16; k++) {
        int byte = std::stoi(inv_sbox_output.substr(k*2, 2), nullptr, 16);
        hamm[k] << hammingWeight(byte);
        if (j < NUM_SUB_KEY-1) {
          hamm[k] << ",";
        }
      }
    }

    for (int k = 0; k < 16; k++)
      hamm[k] << "\n";

    if((i+1) % PRINT_EVERY == 0)
      std::cout << "Completed " << (i+1) << std::endl;
  }

  return 0;
}