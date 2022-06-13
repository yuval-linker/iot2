#pragma once

/*
Encoding functions
*/

void from_char_to_bytearray(int pos, unsigned char *bytes, char x);

void from_unsigned_short_int_to_bytearray(int start, unsigned char *bytes, unsigned short int x);

void from_int_to_bytearray(int start, unsigned char *bytes, int x);

void from_unsigned_int_to_bytearray(int start, unsigned char *bytes, unsigned int x);

void from_float_to_bytearray(int start, unsigned char *bytes, float x);
