#include "str4.h"
#include <string.h>
#include "common.h"

int str4_errorp(Str4 *str) {
	return str->error;
}

Str4 str4_create(Allocator *allocator) {
	return (Str4) {.error=ERR_OK, .allocator=allocator, .str=NULL, .str_length=0};
}

Str4 str4_createConstant(const uint8_t *str, size_t str_length) {
	static Allocator dummy = {.context=NULL, .alloc=allocator_dummy_alloc, .free=allocator_dummy_free, .quit=allocator_dummy_quit};
	return (Str4) {.error=ERR_OK, .allocator=&dummy, .str=((uint8_t *) str), .str_length=str_length};
}

// Copy `str` into `str4`.
void str4_copyC(Str4 *str4, const uint8_t *str, size_t str_length) {
	if (str && !str_length) {
		str4->error = ERR_NULLPOINTER;
		return;
	}
	Allocator *destination_allocator = str4->allocator;
	uint8_t *destination_bytes = NULL;
	int e = destination_allocator->alloc(destination_allocator->context,
	                                     (void **) &destination_bytes,
	                                     (str_length + 1) * sizeof(uint8_t));
	if (e) {
		str4->error = e;
		return;
	}
	(void) memcpy(destination_bytes, str, str_length);
	destination_bytes[str_length] = '\0';
	str4->str = destination_bytes;
	str4->str_length = str_length;
}

void str4_copy(Str4 *destination, Str4 *source) {
	if (source->error) {
		destination->error = source->error;
		return;
	}
	Allocator *destination_allocator = destination->allocator;
	size_t destination_bytes_length = source->str_length;
	uint8_t *destination_bytes = NULL;
	int e = destination_allocator->alloc(destination_allocator->context,
	                                     (void **) &destination_bytes,
	                                     (destination_bytes_length + 1) * sizeof(uint8_t));
	if (e) {
		destination->error = e;
		return;
	}
	(void) memcpy(destination_bytes, source->str, destination_bytes_length);
	destination_bytes[destination_bytes_length] = '\0';
	destination->str = destination_bytes;
	destination->str_length = destination_bytes_length;
}

void str4_concatenate(Str4 *destination, Str4 *left, Str4 *right) {
	if (left->error || right->error) {
		destination->error = (left->error > right->error) ? left->error : right->error;
		return;
	}
	Allocator *destination_allocator = destination->allocator;
	size_t destination_str_length = left->str_length + right->str_length;
	uint8_t *destination_str = NULL;
	int e = destination_allocator->alloc(destination_allocator->context,
	                                     (void **) &destination_str,
	                                     (destination_str_length + 1) * sizeof(uint8_t));
	if (e) {
		destination->error = e;
		return;
	}
	(void) memcpy(destination_str, left->str, left->str_length);
	(void) memcpy(destination_str + left->str_length, right->str, right->str_length);
	destination_str[destination_str_length] = '\0';

	// Commit.
	destination->str = destination_str;
	destination->str_length = destination_str_length;
}

void str4_append(Str4 *destination, Str4 *right) {
	if (destination->error || right->error) {
		if (destination->error < right->error) destination->error = right->error;
		return;
	}
	if (right->str_length == 0) return;
	Allocator *destination_allocator = destination->allocator;
	size_t destination_str_length = destination->str_length + right->str_length;
	uint8_t *destination_str = NULL;
	int e = destination_allocator->alloc(destination_allocator->context,
	                                     (void **) &destination_str,
	                                     (destination_str_length + 1) * sizeof(uint8_t));
	if (e) {
		destination->error = e;
		return;
	}
	(void) memcpy(destination_str, destination->str, destination->str_length);
	(void) memcpy(destination_str + destination->str_length, right->str, right->str_length);
	destination_str[destination_str_length] = '\0';

	// Commit.
	destination->str = destination_str;
	destination->str_length = destination_str_length;
}
