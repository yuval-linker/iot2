#pragma once

#define PAYLOAD_0_SIZE 17       // 11 + 6
#define PAYLOAD_1_SIZE 27       // 11 + 16
#define PAYLOAD_2_SIZE 31       // 11 + 20
#define PAYLOAD_3_SIZE 55       // 11 + 44
#define PAYLOAD_4_SIZE 19227    // 11 + 19216

#define MSG_LENS (unsigned short int[]){6, 16, 20, 44, 19216}
#define MAC_SIZE 6

#ifndef ACC_VECTOR_SIZE
#define ACC_VECTOR_SIZE 1600
#endif