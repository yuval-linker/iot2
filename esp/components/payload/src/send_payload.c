#include "send_payload.h"
#include <math.h>

/** @brief Sets the header of the payload with the information provided. Writes 12 bytes
    * @param start The position in the buffer where to start inserting the bytes
    * @param bytes The buffer array with the bytes
    * @param id_status The Status ID of the device
    * @param id_protocol The protocol id of the device
    * @param id_device The Device ID
    * @param mac The Device MAC

    * @return How many bytes where inserted in the buffer
**/
int set_header(int start, unsigned char *bytes, unsigned char id_status, unsigned char id_protocol, unsigned short int id_device, unsigned char *mac) {
    int counter = start;
    from_unsigned_short_int_to_bytearray(counter, bytes, id_device);
    counter += 2;
    for (int i = 0; i < MAC_SIZE; i++) {
        from_char_to_bytearray(counter+i, bytes, mac[i]);
    }
    counter += 6;
    from_char_to_bytearray(counter, bytes, id_status);
    counter += 1;
    from_char_to_bytearray(counter, bytes, id_protocol);
    counter += 1;
    from_unsigned_short_int_to_bytearray(counter, bytes, MSG_LENS[id_protocol]);
    counter += 2;
    return counter - start;
}

/** @brief Sets the basic data of the payload with random sensor information. Writes 6 bytes
    * @param start The position in the buffer where to start inserting the bytes
    * @param bytes The buffer array with the bytes

    * @return How many bytes where inserted in the buffer
**/
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

/** @brief Sets THPC sensor (random) data in the payload. Writes 10 bytes
    * @param start The position in the buffer where to start inserting the bytes
    * @param bytes The buffer array with the bytes

    * @return How many bytes where inserted in the buffer
**/
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

/** @brief Sets the Aceloremeter KPI (random) data on the payload. Writes 4 or 28 bytes
    * @param start The position in the buffer where to start inserting the bytes
    * @param bytes The buffer array with the bytes
    * @param only_rms If True only includes the RMS data. Otherwise includes Amplitude and Frequency

    * @return How many bytes where inserted in the buffer
**/
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

/** @brief Sets the Aceloremeter sensor data on the payload with the information provided. Writes 19200 bytes
    * @param start The position in the buffer where to start inserting the bytes
    * @param bytes The buffer array with the bytes

    * @return How many bytes where inserted in the buffer
**/
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

/** @brief Writes bytes of the message into the payload. This includes Header and Data.
    * @param payload The buffer array with the bytes
    * @param id_status The Status ID of the device
    * @param id_protocol The protocol id of the device
    * @param id_device The Device ID
    * @param mac The Device MAC
**/
void send_payload(unsigned char *payload, unsigned char id_status, unsigned char id_protocol, unsigned short int id_device, unsigned char *mac) {
    int start = 0;
    start += set_header(start, payload, id_status, id_protocol, id_device, mac);
    if (id_protocol == 0) {
        from_char_to_bytearray(start, payload, 1);
    } else {
        start += set_basic_data(start, payload);
        if (id_protocol >= 2) {
            start += set_thpc_sensor_data(start, payload);
            if (id_protocol == 3) {
                start += set_acc_kpi_data(start, payload, 1);
            }
            else if (id_protocol == 4) {
                start += set_acc_kpi_data(start, payload, 0);
            }
            else if (id_protocol == 5) {
                start += set_acc_sensor_data(start, payload);
            }
        }
    } 
}

