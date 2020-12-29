
#include "str.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include "common.h"
#include "insane.h"

int string_realloc(string_t *s) {
	// unsigned int msb = MSB(s->length) + 1;
	// if (s->memsize < msb) {
	// 	s->memsize = msb;
	// }
	s->memsize = MSB(s->length) + 1;
	s->value = (char *) realloc(s->value, (1<<s->memsize) * sizeof(char));
	if (s->value == NULL) {
		return ERR_OUTOFMEMORY;
	}
	return ERR_OK;
}

int string_append_char(string_t *s, char c) {

	s->value[s->length] = c;
	s->length++;
	if (string_realloc(s)) {
		return 1;
	}
	s->value[s->length] = '\0';

	return 0;
}

int string_init(string_t *s) {

	s->length = 0;
	s->memsize = 0;
	s->value = malloc((1<<s->memsize) * sizeof(char));
	if (s->value == NULL) {
		return ERR_OUTOFMEMORY;
	}
	s->value[0] = '\0';
	return ERR_OK;
}

void string_free(string_t *s) {
	insane_free(s->value);
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
		return ERR_OUTOFMEMORY;

	strcpy(destination->value, source->value);

	return ERR_OK;
}

int string_concatenate_c(string_t *destination, const char *source) {
	
	int error = 0;

	destination->length += strlen(source);
	error = string_realloc(destination);

	if (error)
		return error;

	strcat(destination->value, source);
	return 0;
}

int string_concatenate(string_t *destination, const string_t *source) {

	destination->length += source->length;

	// destination->value = realloc(destination->value, (1<<destination->memsize) * sizeof(char));
	// if (destination->value == NULL)
	// 	return ERR_OUTOFMEMORY;
	error = string_realloc(destination);
	if (error) {
		error = ERR_OUTOFMEMORY;
		return ERR_OUTOFMEMORY;
	}

	strcat(destination->value, source->value);

	return ERR_OK;
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
	
	if (index < 0) {
		return 0;
	}
	
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
		error = string_realloc(destination);
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
		error = string_realloc(destination);
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

int string_count_char(const string_t *s, const char c) {
	int count = 0;
	
	for (int i = 0; i < s->length; i++) {
		if (s->value[i] == c) {
			count++;
		}
	}
	return count;
}

int string_count_c(const string_t *s, const char *c) {
	int count = 0;
	
	for (int i = 0; i < s->length; i++) {
		if (!strcmp(s->value + i, c)) {
			count++;
		}
	}
	return count;
}

/* Remove comments */
int string_removeLineComments(string_t *line, const char *linecomment) {
	int error = ERR_OK;
	
	char *commentStart = strstr(line->value, linecomment);
	if (commentStart == NULL) {
		return ERR_OK;
	}

	/* Remove comments. */
	error = string_substring(line, line, 0, commentStart - line->value);
	if (error > 2) {
		return error;
	}

	return ERR_OK;
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
		}
	}
	// if (!trailing) {
		endlength = line->length - gap;
	// }

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
	printf(COLOR_CYAN"string: "
	       COLOR_BLUE"[length] "COLOR_CYAN"%i"COLOR_NORMAL" ; "
	       COLOR_BLUE"[memsize] "COLOR_CYAN"%i"COLOR_NORMAL" ; "
	       COLOR_BLUE"[value] "COLOR_CYAN"\"%s\""COLOR_NORMAL"\n",
	       s->length, s->memsize, s->value);
}

// /*
// Copied from insane.h and modified by Joey.
// */
// int string_vasprintf(string_t **strp, const char *fmt, va_list ap)
// {
//   int r = -1, size;

//   va_list ap2;
//   va_copy(ap2, ap);

//   size = vsnprintf(0, 0, fmt, ap2);

//   if ((size >= 0) && (size < INT_MAX))
//   {
//     *(strp)->value = (char *)malloc(size+1); //+1 for null
//     if (*strp)
//     {
//       r = vsnprintf(*strp, size+1, fmt, ap);  //+1 for null
//       if ((r < 0) || (r > size))
//       {
//         insane_free(*strp);
//         r = -1;
//       }
//     }
//   }
//   else { *strp = 0; }

//   va_end(ap2);

//   return(r);
// }

// /*
// The va_list is the single worst hack in all of C. I literally cannot implement
// this function with format as string_t because of this terrible hack. I find it
// very difficult to call it a feature. I almost wish it never existed in the first
// place.
// */
// void string_format(string_t *out, const char *format, ...) {
	
// 	va_list args;
// 	va_start(args, format);
// 	error = string_vasprintf(&out, format, args);
// 	va_end(args);
// }

/* Not the safest... */
const string_t * const string_const(const char *char_array) {
	static string_t string[5];
	static int i = 0;
	
	i++;
	if (i == 5) {
		i = 0;
	}
	
	string[i].length = strlen(char_array);
	/* Dirty, but I don't know how else to do it. */
	string[i].value = (char *) char_array;
	string[i].memsize = MSB(string[i].length) + 1;
	
	return &string[i];
}

int string_compare(const string_t *left, const string_t *right) {
	
	int i;
	int length = (left->length > right->length) ? right->length : left->length;

	for (i = 0; (i < length) && ((left->value[i] - right->value[i]) == 0); i++);

	return left->value[i] - right->value[i];
}
