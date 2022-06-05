#pragma once

#include "encoding.h"
#include "sensors.h"
#include "payload_sizes.h"

int set_header(int start, unsigned char *bytes, unsigned char id_protocol, unsigned short int id_device, unsigned char *mac);
int set_basic_data(int start, unsigned char *bytes);
int set_thpc_sensor_data(int start, unsigned char *bytes);
int set_acc_kpi_data(int start, unsigned char *bytes, int only_rms);
int set_acc_sensor_data(int start, unsigned char *bytes);
void send_payload(unsigned char *payload, unsigned char id_protocol, unsigned short int id_device, unsigned char *mac);
