#include "recv_payload.h"

// NOT USED
char decode_payload_header(unsigned char *payload, payload_header_st *header) {
    int start = 0;
    header->id_device = from_bytearray_to_unsigned_short_int(start, payload);
    start += 2;
    for (int i = 0; i < MAC_SIZE; i++) {
        header->mac[i] = from_bytearray_to_char(start + i, payload);
    }
    start += 6;
    header->id_protocol = from_bytearray_to_char(start, payload);
    start += 1;
    header->len_msg = from_bytearray_to_unsigned_short_int(start, payload);
    return header->id_protocol;
}

// NOT USED
void decode_payload_data(unsigned char *payload, unsigned char id_protocol, payload_data_st *data) {
    int start = 11;
    data->val = from_bytearray_to_char(start, payload);
    start += 1;
    data->batt_level = from_bytearray_to_char(start, payload);
    start += 1; 
    data->timestamp = from_bytearray_to_unsigned_int(start, payload);
    start += 4;
    
    if (id_protocol == 0) 
        return;
    
    data->temp = from_bytearray_to_char(start, payload);
    start += 1; 
    data->press = from_bytearray_to_int(start, payload); 
    start += 4; 
    data->hum = from_bytearray_to_char(start, payload); 
    start += 1; 
    data->co = from_bytearray_to_float(start, payload); 
    start += 4;

    if (id_protocol == 1)
        return;
    
    if (id_protocol == 2 || id_protocol == 3) {
        data->rms = from_bytearray_to_float(start, payload);

        if (id_protocol == 2) 
            return;
        start += 4;

        data->amp_x = from_bytearray_to_float(start, payload);
        start += 4;
        data->frec_x = from_bytearray_to_float(start, payload);
        start += 4;
        data->amp_y = from_bytearray_to_float(start, payload);
        start += 4;
        data->frec_y = from_bytearray_to_float(start, payload);
        start += 4;
        data->amp_z = from_bytearray_to_float(start, payload);
        start += 4;
        data->frec_z = from_bytearray_to_float(start, payload);
        start += 4;
    }
    else if (id_protocol == 4) {
        for (int i = 0; i < ACC_VECTOR_SIZE; i++) {
            data->acc_x[i] = from_bytearray_to_float(start + i, payload);
            data->acc_y[i] = from_bytearray_to_float(start + 6400 + i, payload);
            data->acc_z[i] = from_bytearray_to_float(start + 2*6400 + i, payload);
        }
    }
}

// NOT USED
void recv_payload(unsigned char *payload, payload_st *decoded_payload) {
    payload_header_st header;
    unsigned char id_protocol = decode_payload_header(payload, &header);
    

    payload_data_st data;
    decode_payload_data(payload, id_protocol, &data);

    decoded_payload->header = header;
    decoded_payload->data = data;
}

/** @brief Function to decode the incoming message with the configuration data
    * @param payload The buffer with the bytes from the incoming message
    * @param data The config structure to populate with the decoded values
**/
void decode_config_data(unsigned char *payload, config_data_st *data) {
    int start = 0;
    data->status = from_bytearray_to_char(start, payload);
    start += 1;
    data->id_protocol = from_bytearray_to_char(start, payload);
    start += 1;
    data->bmi270_sampling = from_bytearray_to_int(start, payload);
    start += 4;
    data->bmi270_acc_sensibility = from_bytearray_to_int(start, payload);
    start += 4;
    data->bmi270_gyro_sensibility = from_bytearray_to_int(start, payload);
    start += 4;
    data->bme688_sampling = from_bytearray_to_int(start, payload);
    start += 4;
    data->discontinous_time = from_bytearray_to_int(start, payload);
    start += 4;
    data->tcp_port = from_bytearray_to_int(start, payload);
    start += 4;
    data->udp_port = from_bytearray_to_int(start, payload);
    start += 4;
    data->host_ip_addr = from_bytearray_to_int(start, payload);
    start += 4;

    // 32
    for (int i = 0; i < SSID_SIZE; i++) {
        data->ssid[i] = from_bytearray_to_char(start, payload);
        start += 1;
    }

    // 32
    for (int i = 0; i < PASS_SIZE; i++) {
        data->pass[i] = from_bytearray_to_char(start, payload);
        start += 1;
    }
}