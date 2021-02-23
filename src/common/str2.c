
#include "str2.h"
#include "common.h"
#include "log.h"

// str2_t str2_init(void) {
// 	return (str2_t) {
// 		.value = NULL,
// 		.length = 0
// 	};
// }

// int str2_copy_c(str2_t *destination, const char *source) {
// 	int error = ERR_CRITICAL;

// 	destination->value = malloc((strlen(source) + 1) * sizeof(char));
// 	destination->length = strlen(source);
// 	if (destination->value == NULL) {
// 		critical_error("Out of memory.", "");
// 		error = ERR_CRITICAL;
// 		goto cleanup_l;
// 	}
// 	strcpy(destination->value, source);
	
// 	error = ERR_OK;
// 	cleanup_l:
	
// 	return error;
// }

int str2_realloc(char *string, size_t length) {
	
	string = realloc(string, (length + 1) * sizeof(char));
	if (string == NULL) {
		critical_error("Out of memory.", "");
		return ERR_CRITICAL;
	}
	return ERR_OK;
}

int str2_copy(char *destination, const char *source) {
	strcpy(destination, source);
	return ERR_OK;
}

int str2_copyLength(char *destination, const char *source, size_t length) {
	strncpy(destination, source, length);
	return ERR_OK;
}

int str2_concatenate(char *destination, const char *source) {
	strcat(destination, source);
	return ERR_OK;
}

int str2_copyMalloc(char *destination, const char *source) {
	int error = ERR_CRITICAL;

	destination = malloc((strlen(source) + 1) * sizeof(char));
	// destination->length = source->length;
	if (destination == NULL) {
		critical_error("Out of memory.", "");
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	str2_copy(destination, source);
	
	error = ERR_OK;
	cleanup_l:
	
	return error;
}

int str2_copyLengthMalloc(char *destination, const char *source, size_t length) {
	int error = ERR_CRITICAL;
	
	size_t destination_length = strlen(source);
	if (destination_length > length) {
		destination_length = length;
	}
	
	destination = malloc((destination_length + 1) * sizeof(char));
	// destination->length = source->length;
	if (destination == NULL) {
		critical_error("Out of memory.", "");
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	str2_copyLength(destination, source, destination_length);
	
	error = ERR_OK;
	cleanup_l:
	
	return error;
}

int str2_concatenateMalloc(char *destination, const char *source) {
	int error = ERR_CRITICAL;
	
	error = str2_realloc(destination, strlen(destination) + strlen(source));
	if (error) {
		goto cleanup_l;
	}
	
	str2_concatenate(destination, source);
	
	error = ERR_OK;
	cleanup_l:
	
	return error;
}

void str2_tokenize(char **tokens, int *length, char *string) {
	
	// length = 1;
	// tokens[0] = string;
	
	// for (int i = 1;; i++) {
	// 	strto
	// }
}

// void str2_free(str2_t * string) {
// 	insane_free(string->value);
// 	string->length = 0;
// }

/* Remove comments */
int str2_removeLineComments(char *line, const char *linecomment) {
	// int error = ERR_OK;
	
	char *commentStart = strstr(line, linecomment);
	if (commentStart == NULL) {
		return ERR_OK;
	}

	/* Remove comments. */
	// Don't require a free.
	line[commentStart - line] = '\0';
	// error = string_substring(line, line, 0, commentStart - line->value);
	// if (error > 2) {
	// 	return error;
	// }

	return ERR_OK;
}

/* Remove whitespace */
int str2_removeWhitespace(char *line, const char *config) {

	int error = 0;
	int gap;
	const bool leading = strchr(config, 'l') != NULL;
	const bool middle = strchr(config, 'm') != NULL;
	const bool trailing = strchr(config, 't') != NULL;
	const bool extra = strchr(config, 'e') != NULL;
	int startindex = 0;
	int endlength = strlen(line);
	bool deletespaces;
	bool sawspace = false;

	/* Remove leading space. */
	gap = 0;
	deletespaces = true;
	for (int i = 0; i < strlen(line); i++) {
		if (isspace(line[i]) && deletespaces) {
			gap++;
		}
		else {
			deletespaces = false;
			if (leading) {
				line[i - gap] = line[i];
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
		if (isspace(line[i]) && deletespaces) {
			gap++;
		}
		else if (trailing) {
			deletespaces = false;
		}
	}
	// if (!trailing) {
		endlength = strlen(line) - gap;
	// }

	/* Remove middle whitespace. */
	gap = 0;
	if (middle) {
		for (int i = startindex; i < endlength; i++) {
			if (isspace(line[i])) {
				if (sawspace || !extra) {
					gap++;
				}
				else {
					sawspace = true;
				}
			}
			else {
				line[i - gap] = line[i];
				sawspace = false;
			}
		}
	}
	
	/* Normalize the resulting string since we did a major surgery on it. */
	line[endlength] = '\0';
	
	// Don't require a free.
	// line->length = strlen(line->value);
	// error = string_normalize(line);
	
	return error;		
}

// const str2_t *str2_const(const char *chars) {
// 	static int i = 0;
// 	static str2_t strings[5];
// 	strings[i] = (str2_t) {
// 		.value = (char *) chars,
// 		.length = strlen(chars)
// 	};
// 	return &strings[i++];
// }
