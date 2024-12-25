#include <iomanip>
#include <sstream>
#include <cstdint>
#include <wmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

// https://www.intel.com/content/dam/doc/white-paper/advanced-encryption-standard-new-instructions-set-paper.pdf
const __m128i ZERO = _mm_setzero_si128();
const __m128i ISOLATE_SROWS_MASK = _mm_set_epi32(0x0B06010C,0x07020D08, 0x030E0904, 0x0F0A0500);
const  __m128i ISOLATE_SBOX_MASK = _mm_set_epi32(0x0306090C, 0x0F020508, 0x0B0E0104, 0x070A0D00);


std::string m128iToHexString(const __m128i &value);
__m128i hexStringToM128i(const std::string &hexString);

void generateRoundKeys(const __m128i &key, __m128i *roundKeys);
void aesEncrypt(__m128i plaintext, __m128i *roundKeys, std::string *ciphertext);
__m128i performSubBytes(const __m128i &input);
__m128i performInvSubBytes(const __m128i &input);
int hammingWeight(int byte);

std::istream &operator>>(std::istream &is, __m128i &value);
std::ostream &operator<<(std::ostream &os, const __m128i &value);
