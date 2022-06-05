#pragma once

/*
Decoding functions
*/

char from_bytearray_to_char(int pos, unsigned char *bytes);

unsigned short int from_bytearray_to_unsigned_short_int(int start, unsigned char *bytes);

int from_bytearray_to_int(int start, unsigned char *bytes);

unsigned int from_bytearray_to_unsigned_int(int start, unsigned char *bytes);

float from_bytearray_to_float(int start, unsigned char *bytes);
