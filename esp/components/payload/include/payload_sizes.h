#pragma once

#define PAYLOAD_0_SIZE 13       // 12 + 1
#define PAYLOAD_1_SIZE 18       // 12 + 6
#define PAYLOAD_2_SIZE 28       // 12 + 16
#define PAYLOAD_3_SIZE 32       // 12 + 20
#define PAYLOAD_4_SIZE 56       // 12 + 44
#define PAYLOAD_5_SIZE 19228    // 12 + 19216
#define CONFIG_SIZE 98          // 34 + SSID + PASS

#define MSG_LENS (unsigned short int[]){1, 6, 16, 20, 44, 19216}
#define MAC_SIZE 6

#define SSID_SIZE 32
#define PASS_SIZE 32

#ifndef ACC_VECTOR_SIZE
#define ACC_VECTOR_SIZE 1600
#endif