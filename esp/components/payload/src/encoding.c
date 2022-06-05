#include "../include/encoding.h"

/*
Encoding functions
*/

void from_char_to_bytearray(int pos, unsigned char *bytes, char x) {
    unsigned char *ptr = (unsigned char *) &x;
    bytes[pos] = *ptr;
}

void from_unsigned_short_int_to_bytearray(int start, unsigned char *bytes, unsigned short int x) {
    unsigned char *ptr = (unsigned char *) &x; 
    for (int i = 0; i < sizeof(x); i++) {
        bytes[start + i] = *(ptr + i);
    }
}

void from_int_to_bytearray(int start, unsigned char *bytes, int x) {
    unsigned char *ptr = (unsigned char *) &x; 
    for (int i = 0; i < sizeof(x); i++) {
        bytes[start + i] = *(ptr + i);
    }
}

void from_unsigned_int_to_bytearray(int start, unsigned char *bytes, unsigned int x) {
    unsigned char *ptr = (unsigned char *) &x; 
    for (int i = 0; i < sizeof(x); i++) {
        bytes[start + i] = *(ptr + i);
    }
}

void from_float_to_bytearray(int start, unsigned char *bytes, float x) {
    unsigned char *ptr = (unsigned char *) &x; 
    for (int i = 0; i < sizeof(x); i++) {
        bytes[start + i] = *(ptr + i);
    }
}
