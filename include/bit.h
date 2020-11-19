#pragma once
#include <cstdint>


/**
 * The bit is a basic unit of information in information theory, computing.
 * This package includes bit twiddling hacks by Sean Eron Anderson and many
 * others.
 */
namespace bit {
const int DEBRUIJN_POS32[] = {
  0,  9,  1, 10, 13, 21,  2, 29, 11, 14, 16, 18, 22, 25,  3, 30, 
  8, 12, 20, 28, 15, 17, 24,  7, 19, 27, 23,  6, 26,  5,  4, 31
};

const int MOD37_POS32[] = {
  32, 0, 1, 26, 2, 23, 27, 0, 3, 16, 24, 30, 28, 11, 0, 13, 4, 7, 17, 0,
  25, 22, 31, 15, 29, 10, 12, 6, 0, 21, 14, 9, 5, 20, 8, 19, 18
};


/**
 * Counts bits set.
 * @param x a uint32
 * @return count
 */
inline int count(uint32_t x) {
  x = x - ((x>>1) & 0x55555555);
  x = (x & 0x33333333) + ((x>>2) & 0x33333333);
  return ((x + (x>>4) & 0x0F0F0F0F) * 0x01010101)>>24;
}

/**
 * Counts bits set.
 * @param x a uint64
 * @return count
 */
inline int count(uint64_t x) {
  x = x - ((x>>1) & 0x5555555555555555L);
  x = (x & 0x3333333333333333L) + ((x>>2) & 0x3333333333333333L);
  x = ((x + (x>>4) & 0x0F0F0F0F0F0F0F0FL) * 0x0101010101010101L)>>56;
  return (int) x;
}


/**
 * Gets a bit.
 * @param x an int
 * @param i bit index
 * @return bit
 */
inline int get(uint32_t x, int i) {
  return (x>>i) & 1;
}

/**
 * Gets a bit.
 * @param x a long
 * @param i bit index
 * @return bit
 */
inline int get(uint64_t x, int i) {
  return (int) (x>>i) & 1;
}


/**
 * Gets bits as per mask.
 * @param x an int
 * @param m bit mask
 * @return bits
 */
inline int getAs(uint32_t x, uint32_t m) {
  return x & m;
}

/**
 * Gets bits as per mask.
 * @param x a long
 * @param m bit mask
 * @return bits
 */
inline uint64_t getAs(uint64_t x, uint64_t m) {
  return x & m;
}


/**
 * Interleaves bits of two shorts.
 * @param x first short
 * @param y second short
 * @return int
 */
inline uint32_t interleave(uint32_t x, uint32_t y) {
  x = (x | (x<<8)) & 0x00FF00FF;
  x = (x | (x<<4)) & 0x0F0F0F0F;
  x = (x | (x<<2)) & 0x33333333;
  x = (x | (x<<1)) & 0x55555555;
  y = (y | (y<<8)) & 0x00FF00FF;
  y = (y | (y<<4)) & 0x0F0F0F0F;
  y = (y | (y<<2)) & 0x33333333;
  y = (y | (y<<1)) & 0x55555555;
  return y | (x<<1);
}

/**
 * Interleaves bits of two ints.
 * @param x first int
 * @param y second int
 * @return interleaved long
 */
inline uint64_t interleave(uint64_t x, uint64_t y) {
  x = (x | (x<<16)) & 0x0000FFFF0000FFFFL;
  x = (x | (x<<8)) & 0x00FF00FF00FF00FFL;
  x = (x | (x<<4)) & 0x0F0F0F0F0F0F0F0FL;
  x = (x | (x<<2)) & 0x3333333333333333L;
  x = (x | (x<<1)) & 0x5555555555555555L;
  y = (y | (y<<16)) & 0x0000FFFF0000FFFFL;
  y = (y | (y<<8)) & 0x00FF00FF00FF00FFL;
  y = (y | (y<<4)) & 0x0F0F0F0F0F0F0F0FL;
  y = (y | (y<<2)) & 0x3333333333333333L;
  y = (y | (y<<1)) & 0x5555555555555555L;
  return y | (x<<1);
}


/**
 * Merges bits as per mask.
 * @param x first int
 * @param y second int
 * @param m bit mask (0 => from x)
 * @return merged int
 */
inline uint32_t merge(uint32_t x, uint32_t y, uint32_t m) {
  return x ^ ((x^y) & m);
}

/**
 * Merges bits as per mask.
 * @param x first long
 * @param y second long
 * @param m bit mask (0 => from x)
 * @return merged long
 */
inline uint64_t merge(uint64_t x, uint64_t y, uint64_t m) {
  return x ^ ((x^y) & m);
}


/**
 * Gets 1-bit parity.
 * @param x an int
 * @return parity
 */
inline int parity(uint32_t x) {
  x ^= x>>16;
  x ^= x>>8;
  x ^= x>>4;
  x &= 0xF;
  return (0x6996>>x) & 1;
}

/**
 * Gets 1-bit parity.
 * @param x a long
 * @return parity
 */
inline int parity(uint64_t x) {
  x ^= x>>32;
  x ^= x>>16;
  x ^= x>>8;
  x ^= x>>4;
  x &= 0xF;
  return (0x6996>>x) & 1;
}

/**
 * Gets n-bit parity.
 * @param x an int
 * @param n number of bits (1)
 * @return parity
 */
inline int parity(uint32_t x, int n) {
  if (n == 1) return parity(x);
  int m = (1<<n) - 1, a = 0;
  while (x!=0) {
    a ^= x & m;
    x >>= n;
  }
  return a;
}

/**
 * Gets n-bit parity.
 * @param x an int
 * @param n number of bits (1)
 * @return parity
 */
inline int parity(uint64_t x, int n) {
  if (n == 1) return parity(x);
  long m = (1<<n) - 1, a = 0;
  while (x!=0) {
    a ^= x & m;
    x >>= n;
  }
  return (int) a;
}


/**
 * Reverses all bits.
 * @param x an int
 * @return reversed int
 */
inline uint32_t reverse(uint32_t x) {
  x = ((x>>1) & 0x55555555) | ((x & 0x55555555)<<1);
  x = ((x>>2) & 0x33333333) | ((x & 0x33333333)<<2);
  x = ((x>>4) & 0x0F0F0F0F) | ((x & 0x0F0F0F0F)<<4);
  x = ((x>>8) & 0x00FF00FF) | ((x & 0x00FF00FF)<<8);
  return (x>>16) | (x<<16);
}

/**
 * Reverses all bits.
 * @param x a long
 * @return reversed long
 */
inline uint64_t reverse(uint64_t x) {
  x = ((x>>1) & 0x5555555555555555L) | ((x & 0x5555555555555555L)<<1);
  x = ((x>>2) & 0x3333333333333333L) | ((x & 0x3333333333333333L)<<2);
  x = ((x>>4) & 0x0F0F0F0F0F0F0F0FL) | ((x & 0x0F0F0F0F0F0F0F0FL)<<4);
  x = ((x>>8) & 0x00FF00FF00FF00FFL) | ((x & 0x00FF00FF00FF00FFL)<<8);
  x = ((x>>16) & 0x0000FFFF0000FFFFL) | ((x & 0x0000FFFF0000FFFFL)<<16);
  return (x>>32) | (x<<32);
}


/**
 * Rotates bits.
 * @param x an int
 * @param n rotate amount (+ve: left, -ve: right)
 * @return rotated int
 */
inline uint32_t rotate(uint32_t x, int n) {
  return n<0? x<<32+n | x>>-n : x<<n | x>>32-n;
}

/**
 * Rotates bits.
 * @param x a long
 * @param n rotate amount (+ve: left, -ve: right)
 * @return rotated long
 */
inline uint64_t rotate(uint64_t x, int n) {
  return n<0? x<<64+n | x>>-n : x<<n | x>>64-n;
}


/**
 * Gets index of first set bit from LSB.
 * @param x an int
 * @return bit index
 */
inline int scan(uint32_t x) {
  return MOD37_POS32[(-x & x) % 37];
}

/**
 * Gets index of first set bit from LSB.
 * @param x a long
 * @return bit index
 */
inline int scan(uint64_t x) {
  int a = scan((uint32_t)(x & 0xFFFFFFFFL));
  int b = scan((uint32_t)(x>>32));
  return a==32? b+32 : a;
}


/**
 * Gets index of first set bit from MSB.
 * @param x an int32
 * @return bit index
 */
inline int scanReverse(uint32_t x) {
  x |= x>>1;
  x |= x>>2;
  x |= x>>4;
  x |= x>>8;
  x |= x>>16;
  return DEBRUIJN_POS32[(x*0x07C4ACDD)>>27];
}

/**
 * Gets index of first set bit from LSB.
 * @param x a long
 * @return bit index
 */
inline int scanReverse(uint64_t x) {
  int a = scanReverse((uint32_t)(x & 0xFFFFFFFFL));
  int b = scanReverse((uint32_t)(x>>32));
  return a==32? b+32 : a;
}


/**
 * Sets a bit.
 * @param x an int
 * @param i bit index
 * @param f bit value (1)
 * @return set int
 */
inline uint32_t set(uint32_t x, int i, int f) {
  return (x & ~(1<<i)) | (f<<i);
}

/**
 * Sets a bit.
 * @param x a long
 * @param i bit index
 * @param f bit value (1)
 * @return set long
 */
inline uint64_t set(uint64_t x, int i, int f) {
  return (x & ~(1<<i)) | ((long) f<<i);
}


/**
 * Sets bits as per mask.
 * @param x an int
 * @param m bit mask
 * @param f bit value (1)
 * @return set int
 */
inline uint32_t setAs(uint32_t x, uint32_t m, int f) {
  return (x & ~m) | (-f & m);
}

/**
 * Sets bits as per mask.
 * @param x a long
 * @param m bit mask
 * @param f bit value (1)
 * @return set long
 */
inline uint64_t setAs(uint64_t x, uint64_t m, int f) {
  return (x & ~m) | ((uint64_t) -f & m);
}


/**
 * Sign extends variable bit-width integer.
 * @param x variable bit-width int
 * @param w bit width (32)
 * @return sign-extended int
 */
inline uint32_t signExtend(uint32_t x, int w) {
  w = 32-w;
  return (x<<w)>>w;
}

/**
 * Sign extends variable bit-width integer.
 * @param x variable bit-width long
 * @param w bit width (64)
 * @return sign-extended long
 */
inline uint64_t signExtend(uint64_t x, int w) {
  w = 64-w;
  return (x<<w)>>w;
}


/**
 * Swaps bit sequences.
 * @param x an int
 * @param i first bit index
 * @param j second bit index
 * @param n bit width (1)
 * @return swapped int
 */
inline uint32_t swap(uint32_t x, int i, int j, int n) {
  uint32_t t = ((x>>i)^(x>>j)) & ((1<<n)-1);
  return x ^ ((t<<i)|(t<<j));
}

/**
 * Swaps bit sequences.
 * @param x a long
 * @param i first bit index
 * @param j second bit index
 * @param n bit width (1)
 * @return swapped long
 */
inline uint64_t swap(uint64_t x, int i, int j, int n) {
  uint64_t t = ((x>>i)^(x>>j)) & ((1L<<n)-1L);
  return x ^ ((t<<i)|(t<<j));
}


/**
 * Toggles a bit.
 * @param x an int
 * @param i bit index
 * @return toggled int
 */
inline uint32_t toggle(uint32_t x, int i) {
  return x ^ (1<<i);
}

/**
 * Toggles a bit.
 * @param x a long
 * @param i bit index
 * @return toggled long
 */
inline uint64_t toggle(uint64_t x, int i) {
  return x ^ (1<<i);
}


/**
 * Toggles bits as per mask.
 * @param x an int
 * @param m bit mask
 * @return toggled int
 */
inline uint32_t toggleAs(uint32_t x, uint32_t m) {
  return x ^ m;
}

/**
 * Toggles bits as per mask.
 * @param x a long
 * @param m bit mask
 * @return toggled long
 */
inline uint64_t toggleAs(uint64_t x, uint64_t m) {
  return x ^ m;
}
}
