#include <stdint.h>

static const uint32_t sha256_k[] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1,
    0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786,
    0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
    0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
    0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,
    0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

#define rotr(v, b) (((uint32_t)v >> b) | (v << (32 - b)))

static inline void sha256round(uint32_t *hs, uint32_t *w) {
  for (int i = 16; i < 64; ++i) {
    uint32_t s0 = rotr(w[i - 15], 7) ^ rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
    uint32_t s1 = rotr(w[i - 2], 17) ^ rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
    w[i] = (w[i - 16] + s0 + w[i - 7] + s1) | 0;
  }

  uint32_t a = hs[0];
  uint32_t b = hs[1];
  uint32_t c = hs[2];
  uint32_t d = hs[3];
  uint32_t e = hs[4];
  uint32_t f = hs[5];
  uint32_t g = hs[6];
  uint32_t h = hs[7];

  for (int i = 0; i < 64; ++i) {
    uint32_t s1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
    uint32_t ch = (e & f) ^ (~e & g);
    uint32_t temp1 = (h + s1 + ch + sha256_k[i] + w[i]);
    uint32_t s0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
    uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
    uint32_t temp2 = (s0 + maj);

    h = g;
    g = f;
    f = e;
    e = (d + temp1);
    d = c;
    c = b;
    b = a;
    a = (temp1 + temp2);
  }

  hs[0] += a;
  hs[1] += b;
  hs[2] += c;
  hs[3] += d;
  hs[4] += e;
  hs[5] += f;
  hs[6] += g;
  hs[7] += h;
}

__attribute__((always_inline)) static inline void
sha256block(uint8_t *buf, uint32_t len, uint32_t *dst) {
  uint32_t hs[] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
                   0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};

  uint32_t w[64];

  for (uint32_t i = 0; i < len; i += 64) {
    for (uint32_t j = 0; j < 16; j++) {
      uint32_t off = (j << 2) + i;
      w[j] = (buf[off] << 24) | (buf[off + 1] << 16) | (buf[off + 2] << 8) |
             buf[off + 3];
    }
    sha256round(hs, w);
  }

  dst[0] = hs[0];
  dst[1] = hs[1];
}

int Reset_Handler(uint32_t *dst, uint8_t *ptr, uint32_t pageSize,
                  uint32_t numPages) {
  for (uint32_t i = 0; i < numPages; ++i) {
    sha256block(ptr, pageSize, dst);
    dst += 2;
    ptr += pageSize;
  }
  __asm__("bkpt 42");
  return 0;
}

#if 0
#define PS 1024
#define NP 10

#include <stdio.h>
#include <string.h>

int main() {
  uint8_t buf[NP * PS];
  uint32_t sums[NP * 2];
  memset(buf, 0, sizeof(buf));
  for (int i = 0; i < PS; ++i)
  buf[i]=i;
  for (int i = 0; i < PS; ++i)
  buf[i+PS]=108;
  Reset_Handler(sums, buf, PS, NP);
  for (int i  = 0; i < NP; ++i) {
    printf("%08x %08x\n", sums[i*2],sums[i*2+1]);
  }
  return 0;
}
#endif