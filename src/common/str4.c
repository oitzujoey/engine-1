#include "str4.h"
#include <string.h>
#include "common.h"
#include "array.h"
#include "arena.h"

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

void str4_appendC(Str4 *destination, const uint8_t *right, size_t right_length) {
	if (destination->error) {
		return;
	}
	if ((right == NULL) && (right_length != 0)) {
		destination->error = ERR_GENERIC;
		return;
	}
	if (right_length == 0) return;
	Allocator *destination_allocator = destination->allocator;
	size_t destination_str_length = destination->str_length + right_length;
	uint8_t *destination_str = NULL;
	int e = destination_allocator->alloc(destination_allocator->context,
	                                     (void **) &destination_str,
	                                     (destination_str_length + 1) * sizeof(uint8_t));
	if (e) {
		destination->error = e;
		return;
	}
	(void) memcpy(destination_str, destination->str, destination->str_length);
	(void) memcpy(destination_str + destination->str_length, right, right_length);
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

size_t str4_length(Str4 *string) {
	return string->str_length;
}

void str4_substring(Str4 *destination, Str4 *source, ptrdiff_t start_index, ptrdiff_t end_index) {
	if (source->error) {
		destination->error = source->error;
		return;
	}
	// end_index > length is probably OK.
	if (end_index > source->str_length) {
		end_index = source->str_length;
	}
	// ...but these are bad.
	if (start_index > end_index || start_index < 0 || start_index > source->str_length) {
		destination->error = ERR_GENERIC;
		return;
	}

	// Shortcut.
	if (start_index == end_index) {
		destination->str = NULL;
		destination->str_length = 0;
		return;
	}

	Allocator *destination_allocator = destination->allocator;
	size_t destination_bytes_length = end_index - start_index;
	uint8_t *destination_bytes = NULL;
	int e = destination_allocator->alloc(destination_allocator->context,
	                                     (void **) &destination_bytes,
	                                     (destination_bytes_length + 1) * sizeof(uint8_t));
	if (e) {
		destination->error = e;
		return;
	}
	(void) memcpy(destination_bytes, &source->str[start_index], destination_bytes_length);
	destination_bytes[destination_bytes_length] = '\0';
	destination->str = destination_bytes;
	destination->str_length = destination_bytes_length;
}

int str4_splitLines(array_t *array, Str4 *string) {
	int e = ERR_OK;
	if (string->error) {
		e = string->error;
		goto cleanup;
	}

	Allocator arena;
	// Use the array's allocator.
	e = allocator_create_arena(&arena, array->allocator);
	if (e) goto cleanup;
	
	Str4 currentLine;

	size_t string_length = str4_length(string);
	size_t substring_start = 0;
	enum {
		NORMAL,
		CARRIAGE_RETURN,
		NEWLINE,
	} state = NORMAL;
	for (size_t string_index = 0; string_index < string_length; string_index++) {
		currentLine = str4_create(&arena);  // Does no allocation.
		uint8_t character = string->str[string_index];
		if (character == '\n') {
			if (state != CARRIAGE_RETURN) {
				(void) str4_substring(&currentLine, string, substring_start, string_index);
				e = array_push(array, &currentLine);
				if (e) goto cleanup;
			}
			else if (state == CARRIAGE_RETURN) {
				// Windows.
				(void) str4_substring(&currentLine, string, substring_start, string_index-1);
				e = array_push(array, &currentLine);
				if (e) goto cleanup;
			}
			state = NEWLINE;
		}
		else if (character == '\r') {
			state = CARRIAGE_RETURN;
		}
		else {
			if (state != NORMAL) {
				substring_start = string_index;
				state = NORMAL;
			}
		}
	}

 cleanup: return e;
}

bool str4_equalC(Str4 *lstring, const uint8_t *rstring, size_t str_length) {
	if (lstring->error) return false;
	if (lstring->str_length != str_length) return false;

	uint8_t *lstring_str = lstring->str;
	bool equal = true;
	for (size_t i = 0; i < str_length; i++) {
		if (lstring_str[i] != rstring[i]) {
			equal = false;
			break;
		}
	}
	return equal;
}
