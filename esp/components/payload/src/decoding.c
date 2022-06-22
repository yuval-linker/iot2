#include "decoding.h"

/*
Decoding functions
*/

/** @brief Function to extract a char value from a bytearray
    * @param pos The position in the buffer where the value is
    * @param bytes The buffer array with the bytes
    * @return The extracted char
**/
char from_bytearray_to_char(int pos, unsigned char *bytes) {
    return *(bytes + pos);
}

/** @brief Function to extract an unsigned short in from a bytearray
    * @param start The position in the buffer where the first byte of the value is
    * @param bytes The buffer array with the bytes
    * @return The extracted unsinged short int
**/
unsigned short int from_bytearray_to_unsigned_short_int(int start, unsigned char *bytes) {
    char ret[sizeof(unsigned short int)];
    for (int i = 0; i < sizeof(unsigned short int); i++) {
        ret[i] = *(bytes + start + i);
    }
    return *((unsigned short int *) ret);
}

/** @brief Function to extract an int value from a bytearray
    * @param start The position in the buffer where the first byte of the value is
    * @param bytes The buffer array with the bytes
    * @return The extracted int
**/
int from_bytearray_to_int(int start, unsigned char *bytes) {
    char ret[sizeof(int)];
    for (int i = 0; i < sizeof(int); i++) {
        ret[i] = *(bytes + start + i);
    }
    return *((int *) ret);
}

/** @brief Function to extract an unsigned int value from a bytearray
    * @param start The position in the buffer where the first byte of the value is
    * @param bytes The buffer array with the bytes
    * @return The extracted unsigned int
**/
unsigned int from_bytearray_to_unsigned_int(int start, unsigned char *bytes) {
    char ret[sizeof(unsigned int)];
    for (int i = 0; i < sizeof(int); i++) {
        ret[i] = *(bytes + start + i);
    }
    return *((unsigned int *) ret);
}

/** @brief Function to extract a float value from a bytearray
    * @param start The position in the buffer where the first byte of the value is
    * @param bytes The buffer array with the bytes
    * @return The extracted float
**/
float from_bytearray_to_float(int start, unsigned char *bytes) {
    char ret[sizeof(float)];
    for (int i = 0; i < sizeof(float); i++) {
        ret[i] = *(bytes + start + i);
    }
    return *((float *) ret);
}
