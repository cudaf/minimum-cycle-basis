#pragma once
#include <cstdint>
#include <cstring>
#include <cmath>
#include <stack>
#include <omp.h>


class bit_vector {

public:
	uint64_t *data;
	bool pinned;
	int capacity;
	int size;

	void (*free_pinned_memory)(unsigned *);

	bit_vector(int &n) {
		size = n;
		capacity = (int) (ceil((double) n / 64));
		data = new uint64_t[capacity];
		memset(data, 0, sizeof(uint64_t) * capacity);
		pinned = false;
	}

	bit_vector(int &n, unsigned *(*mem_alloc)(int, int),
			void (*mem_free)(unsigned *)) {
		size = n;
		capacity = (int) (ceil((double) n / 64));
		data = (uint64_t*) mem_alloc(capacity, 2);
		pinned = true;
		free_pinned_memory = mem_free;
	}

	~bit_vector() {
	}

	void init_zero() {
		memset(data, 0, sizeof(uint64_t) * capacity);
	}

	void clear_memory() {
		if (!pinned)
			delete[] data;
		else
			free_pinned_memory((unsigned *) data);
	}

	int get_size() {
		return capacity;
	}

	int get_num_elements() {
		return size;
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
		memcpy(data, src_vector->data,
				sizeof(uint64_t) * capacity);
	}

	//Return the actual index of the element containing the offset.
	inline uint64_t &get_element_for_pos(int &pos) {
		int index = pos / 64;
		return data[index];
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
		assert(vector->capacity == capacity);
		for (int i = 0; i < capacity; i++)
			data[i] = data[i] ^ vector->data[i];
	}

	unsigned dot_product(bit_vector *vector1) {
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

	//get bit value at the position pos such that pos belongs to [0- num_elements - 1]
	inline unsigned get_bit(int pos) {
		uint64_t &item = get_element_for_pos(pos);
		int offset = pos & 63;
		unsigned val = (item >> offset) & 1;
		return val;
	}
};
