#pragma once

#define SYSTEM_STATUS 1
#define ID_PROTOCOL 2
#define BMI270_SAMPLING 3
#define ACC_SEN 4
#define GYR_SEN 5
#define BME688_SAMPLING 6
#define DISC_TIME 7
#define TCP_PORT 8
#define UDP_PORT 9
#define HOST_IP_ADDR 10
#define SSID 11
#define SSID_LENGTH 12
#define PASS 13
#define PASS_LENGTH 14

int write_int8_NVS(int8_t data, int key);
int write_int32_NVS(int32_t data, int key);
int write_str_NVS(char *data, int key);
int read_int8_NVS(int8_t* data, int key);
int read_int32_NVS(int32_t* data, int key);
int read_u32_NVS(uint32_t *data, int key);
int read_str_NVS(char* data, size_t *length, int key);
void parse_and_save_config(unsigned char *payload);
void int_ip_to_str(int ip, unsigned char *ip_str);
int get_protocol_msg_length(char id_protocol);
void deep_sleep_clk(int wake_up_time);
