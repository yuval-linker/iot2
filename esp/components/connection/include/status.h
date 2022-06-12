#pragma once

#define BLE_CONFIG_STATUS 0
#define TCP_CONFIG_STATUS 20
#define TCP_CONTINOUS_STATUS 21
#define TCP_DISCONTINOUS_STATUS 22
#define UDP_STATUS 23
#define BLE_CONTINOUS_STATUS 30
#define BLE_DISCONTINOUS_STATUS 31

#define STATUS_OK 0x6

void switch_status();