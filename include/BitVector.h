#pragma once
#include <cstdint>
#include <cstring>
#include <cmath>
#include <stack>
#include "utils.h"

using std::stack;


struct BitVector {
  fn_free_t freeFn;
  uint64_t *data;
  int capacity;
  int size;

  BitVector(int &n) {
    size = n;
    capacity = CEILDIV(n, 64);
    data = new uint64_t[capacity];
    memset(data, 0, capacity*sizeof(uint64_t));
    freeFn = NULL;
  }

  BitVector(int &n, fn_alloc_t falloc, fn_free_t ffree) {
    capacity = CEILDIV(n, 64);
    data = (uint64_t*) falloc(capacity, 2);
    size = n;
    freeFn = ffree;
  }

  void init() {
    memset(data, 0, capacity*sizeof(uint64_t));
  }

  void free() {
    if (!freeFn) delete[] data;
    else freeFn((int*) data);
  }

  inline int get_and_numbers(uint64_t &val1, uint64_t &val2) {
    uint64_t temp = (val1 & val2);
    int count = 0;
    while (temp != 0) {
      temp -= (temp & -temp);
      count ^= 1;
    }
    return count;
  }

  void copy_from(const BitVector *y) {
    memcpy(data, y->data, capacity*sizeof(uint64_t));
  }

  inline void print_bits(uint64_t val) {
    stack<bool> bits;
    int count = 64;
    while (val || (count > 0)) {
      if (val & 1)
        bits.push(1);
      else
        bits.push(0);
      val >>= 1;
      count--;
    }
    while (!bits.empty()) {
      printf("%d", bits.top());
      bits.pop();
    }
  }

  inline int get(int i) {
    int d = i/64, b = i&63;
    return (int) (data[d] >> b) & 1;
  }

  inline void set(int i, bool v) {
    int d = i/64, b = i&63;
    data[d] &= ~(1 << b);
    data[d] |= v << b;
  }

  void do_xor(BitVector *vector) {
    assert(vector->capacity == capacity);
    for (int i = 0; i < capacity; i++)
      data[i] = data[i] ^ vector->data[i];
  }

  int dot_product(BitVector *vector1) {
    int val = 0;
    for (int i = 0; i < capacity; i++)
      val ^= get_and_numbers(data[i], vector1->data[i]);
    return val;
  }

  void print() {
    for (int i = 0; i < capacity; i++) {
      print_bits(data[i]);
      printf(" ");
    }
    printf("\n");
  }
};
