#include "payload_sizes.h"
#include "decoding.h"

typedef struct payload_header_s {
    unsigned short int id_device;
    unsigned char mac[MAC_SIZE];
    unsigned char id_protocol;
    unsigned short int len_msg;
} payload_header_st;

typedef struct payload_data_s {
    char val; 
    char batt_level; 
    unsigned int timestamp; 
    char temp; 
    int press; 
    char hum; 
    float co; 
    float rms; 
    float amp_x;
    float frec_x;
    float amp_y;
    float frec_y;
    float amp_z;
    float frec_z;
    float acc_x[ACC_VECTOR_SIZE]; 
    float acc_y[ACC_VECTOR_SIZE];
    float acc_z[ACC_VECTOR_SIZE];
} payload_data_st;

typedef struct payload_s {
    payload_header_st header;
    payload_data_st data;
} payload_st;

char decode_payload_header(unsigned char *payload, payload_header_st *header);
void decode_payload_data(unsigned char *payload, unsigned char id_protocol, payload_data_st *data);
void recv_payload(unsigned char *payload, payload_st *decoded_payload);
