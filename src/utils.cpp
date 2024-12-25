#include "utils.hpp"
#include <ostream>


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

std::istream &operator>>(std::istream &is, __m128i &value) {
    std::string temp;
    is >> temp;
    value = hexStringToM128i(temp);
    return is;
}


std::ostream &operator<<(std::ostream &os, const __m128i &value) {
    os << m128iToHexString(value);
    return os;
}