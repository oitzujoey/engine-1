#ifndef STR3_H
#define STR3_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define STR3(c_string) ((const uint8_t *) c_string), (sizeof(c_string) - 1)

bool str3_eq(const uint8_t *l, size_t l_length, const uint8_t *r, size_t r_length);
int str3_copyMalloc(uint8_t **destination, const uint8_t *source);

#endif // STR3_H
