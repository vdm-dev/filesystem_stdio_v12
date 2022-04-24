#pragma once


size_t ex_strlcpy(char* destination, const char* source, size_t dsize);
size_t ex_strlcat(char* destination, const char* source, size_t dsize);
size_t ex_strlen(const char* str);
size_t ex_strnlen(const char* string, size_t limit);
void* ex_memrev(void* pointer, size_t size);

int exs_strcpy(char* destination, size_t dmax, const char* source);
int exs_strncpy(char* destination, size_t dmax, const char* source,
    size_t count);
int exs_strcat(char* destination, size_t dmax, const char* source);
int exs_strncat(char* destination, size_t dmax, const char* source,
    size_t count);

unsigned int ex_crc32_more(unsigned int initial, const void* data, size_t size);
unsigned int ex_crc32(const void* data, size_t size);
