#include "../measure/measure.h"

#include <emmintrin.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <time.h>
#include <wmmintrin.h>

#define S 100
#define N 1000
#ifndef NUM_PT
#define NUM_PT 100
#endif
#define PRINT_EVERY 50

std::string m128iToHexString(const __m128i &value) {
  std::ostringstream oss;
  const uint8_t *bytes = reinterpret_cast<const uint8_t *>(&value);
  for (int i = 0; i < 16; ++i) {
    oss << std::hex << std::setfill('0') << std::setw(2)
        << static_cast<int>(bytes[i]);
  }
  return oss.str();
}

void generateRoundKeys(const __m128i &key, __m128i *roundKeys) {
  roundKeys[0] = key;
  for (int i = 1; i <= 10; ++i) {
    __m128i temp;
    switch (i) {
    case 1:
      temp = _mm_aeskeygenassist_si128(roundKeys[i - 1], 0x01);
      break;
    case 2:
      temp = _mm_aeskeygenassist_si128(roundKeys[i - 1], 0x02);
      break;
    case 3:
      temp = _mm_aeskeygenassist_si128(roundKeys[i - 1], 0x04);
      break;
    case 4:
      temp = _mm_aeskeygenassist_si128(roundKeys[i - 1], 0x08);
      break;
    case 5:
      temp = _mm_aeskeygenassist_si128(roundKeys[i - 1], 0x10);
      break;
    case 6:
      temp = _mm_aeskeygenassist_si128(roundKeys[i - 1], 0x20);
      break;
    case 7:
      temp = _mm_aeskeygenassist_si128(roundKeys[i - 1], 0x40);
      break;
    case 8:
      temp = _mm_aeskeygenassist_si128(roundKeys[i - 1], 0x80);
      break;
    case 9:
      temp = _mm_aeskeygenassist_si128(roundKeys[i - 1], 0x1B);
      break;
    case 10:
      temp = _mm_aeskeygenassist_si128(roundKeys[i - 1], 0x36);
      break;
    }
    roundKeys[i] =
        _mm_xor_si128(roundKeys[i - 1], _mm_slli_si128(roundKeys[i - 1], 4));
    roundKeys[i] = _mm_xor_si128(roundKeys[i], _mm_slli_si128(roundKeys[i], 8));
    roundKeys[i] = _mm_xor_si128(
        roundKeys[i], _mm_shuffle_epi32(temp, _MM_SHUFFLE(3, 3, 3, 3)));
  }
}

void aesEncrypt(__m128i plaintext, __m128i *roundKeys, std::string *ciphertext) {
  __m128i state = _mm_xor_si128(plaintext, roundKeys[0]);
  for (int i = 1; i <= 9; ++i) {
    state = _mm_aesenc_si128(state, roundKeys[i]);
  }
  state = _mm_aesenclast_si128(state, roundKeys[10]);

  *ciphertext = m128iToHexString(state);
}

int main(int argc, char *argv[]) {
  // init the measurement library
  std::ofstream plaintexts("results/plaintexts");
  std::ofstream traces("results/traces.csv");
  std::ofstream ciphertexts("results/ciphertexts");

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
    b[i] = static_cast<char>(std::stoi(argv[i + 1], nullptr, 16));
  }
  __m128i key =
      _mm_set_epi8(b[15], b[14], b[13], b[12], b[11], b[10], b[9], b[8], b[7],
                   b[6], b[5], b[4], b[3], b[2], b[1], b[0]);
  __m128i roundKeys[11];
  generateRoundKeys(key, roundKeys);
  std::cout << "Last round key: " << m128iToHexString(roundKeys[10]) << std::endl;
  for (int pt = 0; pt < NUM_PT; pt++) {
    // generate random plaintext
    std::string ciphertext;
    for (int i = 0; i < 16; i++) {
      b[i] = rand() % 256;
    }
    __m128i plaintext =
        _mm_set_epi8(b[15], b[14], b[13], b[12], b[11], b[10], b[9], b[8], b[7],
                     b[6], b[5], b[4], b[3], b[2], b[1], b[0]);
    auto temp = m128iToHexString(plaintext);
    plaintexts << temp << std::endl;

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
      if (s < S-1) {
        traces << ",";
      }
    }
    traces << "\n";
    ciphertexts << ciphertext << std::endl;

    if ((pt+1) % PRINT_EVERY == 0) {
      std::cout << "Processed " << pt+1 << " plaintexts" << std::endl;
    }
  }

  return 0;
}
