
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

int string_append_char(string_t *s, char c);
int string_init(string_t *s);
void string_free(string_t *s);
int string_copy_c(string_t *destination, const char *source);
int string_copy(string_t *destination, const string_t *source);
int string_copy_length_c(string_t *destination, const char *source, int length);
int string_index_of(string_t *s, const int index, const char c);
int string_substring(string_t *destination, const string_t *source, const int index, const int length);
int string_normalize(string_t *s);
int string_count(const string_t *s, const char c);

#endif
