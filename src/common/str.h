
#ifndef STR_H
#define STR_H

typedef struct {
    char *value;
    unsigned int length;
    unsigned int memsize;
} string_t;

int string_append_char(string_t *s, char c);
int string_init(string_t *s, const char *init);

#endif
