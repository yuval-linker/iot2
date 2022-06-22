#include "encoding.h"

/*
Encoding functions
*/

/** @brief Function to insert a char value into a bytearray
    * @param pos The position in the buffer where to insert the value
    * @param bytes The buffer array where to insert the value
    * @param x The char value to be inserted
**/
void from_char_to_bytearray(int pos, unsigned char *bytes, char x) {
    unsigned char *ptr = (unsigned char *) &x;
    bytes[pos] = *ptr;
}

/** @brief Function to insert an unsigned short int value into a bytearray
    * @param start The starting position in the buffer where to insert the value
    * @param bytes The buffer array where to insert the value
    * @param x The unsigned short int value to be inserted
**/
void from_unsigned_short_int_to_bytearray(int start, unsigned char *bytes, unsigned short int x) {
    unsigned char *ptr = (unsigned char *) &x; 
    for (int i = 0; i < sizeof(x); i++) {
        bytes[start + i] = *(ptr + i);
    }
}

/** @brief Function to insert an int value into a bytearray
    * @param start The starting position in the buffer where to insert the value
    * @param bytes The buffer array where to insert the value
    * @param x The int value to be inserted
**/
void from_int_to_bytearray(int start, unsigned char *bytes, int x) {
    unsigned char *ptr = (unsigned char *) &x; 
    for (int i = 0; i < sizeof(x); i++) {
        bytes[start + i] = *(ptr + i);
    }
}

/** @brief Function to insert an unsigned int value into a bytearray
    * @param start The starting position in the buffer where to insert the value
    * @param bytes The buffer array where to insert the value
    * @param x The unsigned int value to be inserted
**/
void from_unsigned_int_to_bytearray(int start, unsigned char *bytes, unsigned int x) {
    unsigned char *ptr = (unsigned char *) &x; 
    for (int i = 0; i < sizeof(x); i++) {
        bytes[start + i] = *(ptr + i);
    }
}

/** @brief Function to insert a float value into a bytearray
    * @param start The starting position in the buffer where to insert the value
    * @param bytes The buffer array where to insert the value
    * @param x The float value to be inserted
**/
void from_float_to_bytearray(int start, unsigned char *bytes, float x) {
    unsigned char *ptr = (unsigned char *) &x; 
    for (int i = 0; i < sizeof(x); i++) {
        bytes[start + i] = *(ptr + i);
    }
}
