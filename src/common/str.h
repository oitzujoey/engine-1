
#ifndef STR_H
#define STR_H

typedef struct {
    char *value;
    unsigned int length;
    unsigned int memsize;
} string_t;

/* To use:

// Declare a string.
string_t s;

// Then initialize it.
string_init(&s);

// Then use it.
string_copy_c(&s, "Hello, world!");
printf("String \"%s\" is %i characters long.\n", s.value, s.length);

// But be sure to free it.
string_free(&s);
*/

int string_realloc(string_t *s);
int string_append_char(string_t *s, char c);
int string_init(string_t *s);
void string_free(string_t *s);
int string_copy_c(string_t *destination, const char *source);
int string_copy(string_t *destination, const string_t *source);
int string_concatenate_c(string_t *destination, const char *source);
int string_concatenate(string_t *destination, const string_t *source);
int string_copy_length_c(string_t *destination, const char *source, int length);
/* index is the n-th character index starting at zero. */
int string_index_of(const string_t *s, const int index, const char c);
int string_substring(string_t *destination, const string_t *source, const int index, const int length);
/* Set the value, then call this to adjust the rest. */
int string_normalize(string_t *s);
int string_count_char(const string_t *s, const char c);
int string_removeLineComments(string_t *line, const char *linecomment);
int string_removeWhitespace(string_t *line, const char *config);
int string_print(string_t *s);
/*void string_format(string_t *out, const char *format, ...);*/
/* Convert a character array to a string. */
const string_t * const string_const(const char *char_array);
int string_compare(const string_t *left, const string_t *right);

#endif
