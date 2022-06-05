#pragma once

#define SYSTEM_STATUS 1
#define SAMP_FREQ 2
#define TIME_SEG 3
#define ACC_SEN 4
#define GYR_SEN 5
#define ACC_ANY 6
#define RF_CAL 7
#define SEL_ID 8

int Write_NVS(int32_t data, int key);
int Read_NVS(int32_t* data,int key);
void deep_sleep_clk(int wake_up_time);