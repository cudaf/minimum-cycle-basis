#pragma once
#include <cstdint>
#include <cstring>
#include <cmath>
#include <stack>

typedef unsigned* (*fn_alloc_t)(int, int);
typedef void  (*fn_free_t)(unsigned*);


struct BitVector {
	fn_free_t fn_free;
	uint64_t *data;
	bool pinned;
	int capacity;
	int size;

	BitVector(int &n) {
		size = n;
		capacity = (int) (ceil((double) n / 64));
		data = new uint64_t[capacity];
		memset(data, 0, sizeof(uint64_t) * capacity);
		pinned = false;
	}

	BitVector(int &n, fn_alloc_t mem_alloc, fn_free_t mem_free) {
		data = (uint64_t*) mem_alloc(capacity, 2);
		capacity = (int) (ceil((double) n / 64));
		size = n;
		pinned = true;
		fn_free = mem_free;
	}

	~BitVector() {
	}

	void init() {
		memset(data, 0, sizeof(uint64_t) * capacity);
	}

	void free() {
		if (!pinned) delete[] data;
		else fn_free((unsigned *) data);
	}

	inline uint64_t get_or_number(int &offset, bool &val) {
		uint64_t initial_value = val;
		if (val == false)
			return initial_value;
		initial_value <<= offset;
		return initial_value;
	}

	inline unsigned get_and_numbers(uint64_t &val1,
			uint64_t &val2) {
		uint64_t temp = (val1 & val2);
		unsigned count = 0;
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
		std::stack<bool> bits;
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

	inline unsigned get(int i) {
		int d = i/64, b = i&63;
		return (unsigned) (data[d] >> b) & 1;
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

	unsigned dot_product(BitVector *vector1) {
		unsigned val = 0;
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
