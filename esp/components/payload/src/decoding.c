#include "decoding.h"

/*
Decoding functions
*/

char from_bytearray_to_char(int pos, unsigned char *bytes) {
    return *(bytes + pos);
}

unsigned short int from_bytearray_to_unsigned_short_int(int start, unsigned char *bytes) {
    char ret[sizeof(unsigned short int)];
    for (int i = 0; i < sizeof(unsigned short int); i++) {
        ret[i] = *(bytes + start + i);
    }
    return *((unsigned short int *) ret);
}

int from_bytearray_to_int(int start, unsigned char *bytes) {
    char ret[sizeof(int)];
    for (int i = 0; i < sizeof(int); i++) {
        ret[i] = *(bytes + start + i);
    }
    return *((int *) ret);
}

unsigned int from_bytearray_to_unsigned_int(int start, unsigned char *bytes) {
    char ret[sizeof(unsigned int)];
    for (int i = 0; i < sizeof(int); i++) {
        ret[i] = *(bytes + start + i);
    }
    return *((unsigned int *) ret);
}

float from_bytearray_to_float(int start, unsigned char *bytes) {
    char ret[sizeof(float)];
    for (int i = 0; i < sizeof(float); i++) {
        ret[i] = *(bytes + start + i);
    }
    return *((float *) ret);
}
