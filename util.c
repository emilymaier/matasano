#include <stddef.h>

void memxor(void* destination, void* source1, void* source2, size_t size) {
	for(size_t idx = 0; idx < size; idx++) {
		*(((char*) destination) + idx) = *(((char*) source1) + idx) ^ *(((char*) source2) + idx);
	}
}
