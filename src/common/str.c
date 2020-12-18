
#include "str.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static int getMSB(unsigned int n) {
    int i;
    
    for (i = 0; (1<<i) <= n; i++);
    
    return i-1;
}

int string_append_char(string_t *s, char c) {
    if (s->length+1 == (1<<s->memsize)) {
        s->memsize++;
        s->value = (char *) realloc(s->value, (1<<s->memsize) * sizeof(char));

        if (s->value == NULL)
            return 1;
    }
    s->value[s->length] = c;
    s->length++;
    s->value[s->length] = '\0';
    
    return 0;
}

int string_init(string_t *s, const char *init) {
    s->length = strlen(init);
    s->memsize = getMSB(s->length)+1;
    s->value = malloc((1<<s->memsize) * sizeof(char));
    if (s->value == NULL)
        return 1;
    strcpy(s->value, init);
    return 0;
}
