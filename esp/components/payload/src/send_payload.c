#include "../include/send_payload.h"
#include <math.h>

// writes 11 bytes
int set_header(int start, unsigned char *bytes, unsigned char id_protocol, unsigned short int id_device, unsigned char *mac) {
    int counter = start;
    from_unsigned_short_int_to_bytearray(counter, bytes, id_device);
    counter += 2;
    for (int i = 0; i < MAC_SIZE; i++) {
        from_char_to_bytearray(counter+i, bytes, mac[i]);
    }
    counter += 6;
    from_char_to_bytearray(counter, bytes, id_protocol);
    counter += 1;
    from_unsigned_short_int_to_bytearray(counter, bytes, MSG_LENS[id_protocol]);
    counter += 2;
    return counter - start;
}

// writes 6 bytes.
int set_basic_data(int start, unsigned char *bytes) {
    int counter = start;
    from_char_to_bytearray(counter, bytes, 1);
    counter += 1;
    from_char_to_bytearray(counter, bytes, Batt_sensor().value);
    counter += 1;
    from_unsigned_int_to_bytearray(counter, bytes, get_timestamp());
    counter += 4;
    return counter - start;
}


// writes 10 bytes
int set_thpc_sensor_data(int start, unsigned char *bytes) {
    int counter = start;
    thpc_sensor_st data = THPC_sensor();
    from_char_to_bytearray(start, bytes, data.temp);
    counter += 1;
    from_int_to_bytearray(start + 1, bytes, data.press);
    counter += 4;
    from_char_to_bytearray(start + 5, bytes, data.hum);
    counter += 1;
    from_float_to_bytearray(start + 6, bytes, data.co);
    counter += 4;
    return counter - start;
}

// writes 4 bytes or 28 bytes
int set_acc_kpi_data(int start, unsigned char *bytes, int only_rms) {
    int counter = start;
    acc_kpi_st data = Aceloremeter_kpi();
    from_float_to_bytearray(counter, bytes, data.rms);
    counter += 4;
    if (!only_rms) {
        from_float_to_bytearray(counter, bytes, data.amp_x);
        counter += 4;
        from_float_to_bytearray(counter, bytes, data.frec_x);
        counter += 4;
        from_float_to_bytearray(counter, bytes, data.amp_y);
        counter += 4;
        from_float_to_bytearray(counter, bytes, data.frec_y);
        counter += 4;
        from_float_to_bytearray(counter, bytes, data.amp_z);
        counter += 4;
        from_float_to_bytearray(counter, bytes, data.frec_z);
        counter += 4;
    }
    return counter - start;
}

// writes 19200 bytes
int set_acc_sensor_data(int start, unsigned char *bytes) {
    for (int i = 0; i < ACC_VECTOR_SIZE; i++) {
        from_float_to_bytearray(start + 4*i, bytes, (float) 2*sin(2*M_PI*0.001*i));
    }
    start = start + 4*ACC_VECTOR_SIZE;
    for (int i = 0; i < ACC_VECTOR_SIZE; i++) {
        from_float_to_bytearray(start + 4*i, bytes, (float) 3*cos(2*M_PI*0.001*i));
    }
    start = start + 4*ACC_VECTOR_SIZE;
    for (int i = 0; i < ACC_VECTOR_SIZE; i++) {
        from_float_to_bytearray(start + 4*i, bytes, (float) 10*sin(2*M_PI*0.001*i));
    }
    return 19200;
}

void send_payload(unsigned char *payload, unsigned char id_protocol, unsigned short int id_device, unsigned char *mac) {
    int start = 0;
    start += set_header(start, payload, id_protocol, id_device, mac);
    start += set_basic_data(start, payload);
    if (id_protocol >= 1) {
        start += set_thpc_sensor_data(start, payload);
        if (id_protocol == 2) {
            start += set_acc_kpi_data(start, payload, 1);
        }
        else if (id_protocol == 3) {
            start += set_acc_kpi_data(start, payload, 0);
        }
        else if (id_protocol == 4) {
            start += set_acc_sensor_data(start, payload);
        }
    }
}

