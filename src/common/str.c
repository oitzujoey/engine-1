
#include "str.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>

static int getMSB(unsigned int n) {
	int i;
	
	for (i = 0; (1<<i) <= n; i++);
	
	return i-1;
}

/* @TODO: Make more efficient. Can probably do it without loops. */
int string_realloc_grow(string_t *s) {
	
	while (s->length+1 > (1<<s->memsize)) {
		
		s->memsize++;
		s->value = (char *) realloc(s->value, (1<<s->memsize) * sizeof(char));
		
		if (s->value == NULL)
			return 1;
	}
	
	return 0;
}

/* @TODO: Make more efficient. Can probably do it without loops. */
int string_realloc_shrink(string_t *s) {
	
	while (s->length+1 <= (1<<(s->memsize-1))) {
		if (s->memsize <= 0)
			break;

		--s->memsize;
		s->value = (char *) realloc(s->value, (1<<s->memsize) * sizeof(char));
		
		if (s->value == NULL)
			return 2;
	}
	
	return 0;
}

/* @TODO: Make more efficient. Can probably do it without loops. */
int string_realloc(string_t *s) {

	while (s->length+1 > (1<<s->memsize)) {

		s->memsize++;
		s->value = (char *) realloc(s->value, (1<<s->memsize) * sizeof(char));
		
		if (s->value == NULL)
			return 1;
	}
	
	while (s->length+1 <= (1<<(s->memsize-1))) {
		if (s->memsize <= 0)
			break;

		--s->memsize;
		s->value = (char *) realloc(s->value, (1<<s->memsize) * sizeof(char));
		
		if (s->value == NULL)
			return 2;
	}
	
	return 0;
}

int string_append_char(string_t *s, char c) {

	s->value[s->length] = c;
	s->length++;
	if (string_realloc_grow(s)) {
		return 1;
	}
	s->value[s->length] = '\0';

	return 0;
}

int string_init(string_t *s) {

	s->length = 0;
	s->memsize = 0;
	s->value = malloc((1<<s->memsize) * sizeof(char));
	if (s->value == NULL)
		return 2;
	s->value[0] = '\0';
	return 0;
}

void string_free(string_t *s) {
	free(s->value);
}

int string_copy_c(string_t *destination, const char *source) {
	
	int error = 0;

	destination->length = strlen(source);
	error = string_realloc(destination);

	if (error)
		return error;

	strcpy(destination->value, source);
	return 0;
}

int string_copy(string_t *destination, const string_t *source) {

	destination->length = source->length;
	destination->memsize = source->memsize;

	destination->value = realloc(destination->value, (1<<destination->memsize) * sizeof(char));
	if (destination->value == NULL)
		return 1;

	strcpy(destination->value, source->value);

	return 0;
}

int string_copy_length_c(string_t *destination, const char *source, int length) {
	
	int error = 0;
	
	if (length < 0)
		return 1;
	
	for (destination->length = 0; (destination->length < length) && (source[destination->length] != '\0'); destination->length++);
	
	error = string_realloc(destination);
	if (error)
		return 2;
	
	strncpy(destination->value, source, destination->length);
	destination->value[destination->length] = '\0';
}

int string_index_of(const string_t *s, const int index, const char c) {

	int charcount = 0;
	
	for (int i = 0; i < s->length; i++) {
		if (s->value[i] == c) {
			charcount++;
			if (charcount > index) {
				return i;
			}
		}
	}
	return -1;
}

int string_substring(string_t *destination, const string_t *source, const int index, const int length) {

	int error = 0;
	int actuallength = length;

	/* If the length is less than zero, then copy to end of line. */
	if (length < 0) {
		actuallength = source->length - index;
	}

	/* Disallow out-of-bounds indices. */
	if (index < 0)
		return 1;
	if (index >= source->length)
		return 2;
	if (index + actuallength > source->length)
		return 3;

	/* If the destination string is too small, then grow it. */
	if (destination->length < actuallength) {
		destination->length = actuallength;
		error = string_realloc_grow(destination);
		if (error)
			return 4;
	}
	
	/* Do the transfer. */
	for (int i = 0; i < actuallength; i++) {
		destination->value[i] = source->value[i+index];
	}
	
	/* If the destination string is too large, then shrink it. */
	if (destination->length > actuallength) {
		destination->length = actuallength;
		error = string_realloc_shrink(destination);
		if (error)
			return 5;
	}
	
	destination->value[destination->length] = '\0';
	
	return 0;
}

int string_normalize(string_t *s) {
	s->length = strlen(s->value);
	return string_realloc(s);
}

int string_count(const string_t *s, const char c) {
	int count = 0;
	
	for (int i = 0; i < s->length; i++) {
		if (s->value[i] == c) {
			count++;
		}
	}
	return count;
}

/* Remove comments */
int string_removeLineComments(string_t *line, const char linecomment) {

	int error = 0;

	/* Remove comments. */
	error = string_substring(line, line, 0, string_index_of(line, 0, linecomment));
	if (error > 2) {
		return error;
	}

	return 0;
}

/* Remove whitespace */
int string_removeWhitespace(string_t *line, const char *config) {

	int error = 0;
	int gap;
	const bool leading = strchr(config, 'l') != NULL;
	const bool middle = strchr(config, 'm') != NULL;
	const bool trailing = strchr(config, 't') != NULL;
	const bool extra = strchr(config, 'e') != NULL;
	int startindex = 0;
	int endlength = line->length;
	bool deletespaces;
	bool sawspace = false;

	/* Remove leading space. */
	gap = 0;
	deletespaces = true;
	for (int i = 0; i < line->length; i++) {
		if (isspace(line->value[i]) && deletespaces) {
			gap++;
		}
		else {
			deletespaces = false;
			if (leading) {
				line->value[i - gap] = line->value[i];
			}
		}
	}
	if (leading) {
		endlength -= gap;
	}
	else {
		startindex = gap;
	}
	
	/* Remove end whitespace. */
	gap = 0;
	deletespaces = true;
	for (int i = endlength-1; i >= 0; --i) {
		if (isspace(line->value[i]) && deletespaces) {
			gap++;
		}
		else if (trailing) {
			deletespaces = false;
			line->value[i + gap] = line->value[i];
		}
	}
	if (!trailing) {
		endlength = line->length - gap;
	}

	/* Remove middle whitespace. */
	gap = 0;
	if (middle) {
		for (int i = startindex; i < endlength; i++) {
			if (isspace(line->value[i])) {
				if (sawspace || !extra) {
					gap++;
				}
				else {
					sawspace = true;
				}
			}
			else {
				line->value[i - gap] = line->value[i];
				sawspace = false;
			}
		}
	}
	
	/* Normalize the resulting string since we did a major surgery on it. */
	line->value[endlength] = '\0';
	error = string_normalize(line);
	if (error)
		return error;		
}

int string_print(string_t *s) {
	printf("string: length %i, memsize %i, value \"%s\"\n", s->length, s->memsize, s->value);
}
