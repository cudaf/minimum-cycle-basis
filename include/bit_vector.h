#pragma once
#include <cstdint>
#include <cstring>
#include <cmath>
#include <stack>
#include <omp.h>


class bit_vector {

public:
	int num_elements;
	int size;
	uint64_t *elements;
	bool pinned;

	void (*free_pinned_memory)(unsigned *);

	bit_vector(int &n) {
		num_elements = n;
		size = (int) (ceil((double) n / 64));
		elements = new uint64_t[size];
		memset(elements, 0, sizeof(uint64_t) * size);
		pinned = false;
	}

	bit_vector(int &n, unsigned *(*mem_alloc)(int, int),
			void (*mem_free)(unsigned *)) {
		num_elements = n;
		size = (int) (ceil((double) n / 64));
		elements = (uint64_t*) mem_alloc(size, 2);
		pinned = true;
		free_pinned_memory = mem_free;
	}

	~bit_vector() {
	}

	void init_zero() {
		memset(elements, 0, sizeof(uint64_t) * size);
	}

	void clear_memory() {
		if (!pinned)
			delete[] elements;
		else
			free_pinned_memory((unsigned *) elements);
	}

	int get_size() {
		return size;
	}

	int get_num_elements() {
		return num_elements;
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

	void copy_vector(const bit_vector *src_vector) {
		memcpy(elements, src_vector->elements,
				sizeof(uint64_t) * size);
	}

	//Return the actual index of the element containing the offset.
	inline uint64_t &get_element_for_pos(int &pos) {
		int index = pos / 64;
		return elements[index];
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

	inline void set_bit(int pos, bool val) {
		uint64_t &item = get_element_for_pos(pos);
		int offset = pos & 63;
		uint64_t or_number = get_or_number(offset, val);
		item = item | or_number;
	}

	void do_xor(bit_vector *vector) {
		assert(vector->size == size);
		for (int i = 0; i < size; i++)
			elements[i] = elements[i] ^ vector->elements[i];
	}

	unsigned dot_product(bit_vector *vector1) {
		unsigned val = 0;
		for (int i = 0; i < size; i++)
			val ^= get_and_numbers(elements[i], vector1->elements[i]);
		return val;
	}

	void print() {
		for (int i = 0; i < size; i++) {
			print_bits(elements[i]);
			printf(" ");
		}
		printf("\n");
	}

	//get bit value at the position pos such that pos belongs to [0- num_elements - 1]
	inline unsigned get_bit(int pos) {
		uint64_t &item = get_element_for_pos(pos);
		int offset = pos & 63;
		unsigned val = (item >> offset) & 1;
		return val;
	}
};
