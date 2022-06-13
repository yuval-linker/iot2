#pragma once

#define TEMP_INF 5
#define TEMP_SUP 30

#define HUM_INF 30 
#define HUM_SUP 80

#define PRESS_INF 1000
#define PRESS_SUP 1200

#define CO_INF 30.0
#define CO_SUP 200.0

#define BATT_INF 1 
#define BATT_SUP 100

#define AMP_X_INF 0.0059
#define AMP_X_SUP 0.12

#define AMP_Y_INF 0.0041
#define AMP_Y_SUP 0.11

#define AMP_Z_INF 0.008
#define AMP_Z_SUP 0.15

#define FREC_X_INF 29.0
#define FREC_X_SUP 31.0

#define FREC_Y_INF 59.0
#define FREC_Y_SUP 61.0

#define FREC_Z_INF 89.0
#define FREC_Z_SUP 91.0

#define ACC_VECTOR_SIZE 1600

/*
Return data structs
*/

typedef struct acc_sensor_s{
    float acc_x[1600];
    float acc_y[1600];
    float acc_z[1600];
} acc_sensor_st;

typedef struct thpc_sensor_s{
    char temp, hum;     // temp char or float??
    int press;
    float co;
} thpc_sensor_st;

typedef struct batt_sensor_s{
    char value;
} batt_sensor_st;

typedef struct acc_kpi_s{
    float rms;
    float amp_x, amp_y, amp_z;
    float frec_x, frec_y, frec_z;
} acc_kpi_st;

/*
Auxiliary functions
*/

unsigned int get_timestamp();

float rms(float amp_x, float amp_y, float amp_z);

int randint(int start, int end);

float randfloat(float start, float end);


/*
Simulated sensor functions
*/

acc_sensor_st Accelerometer_sensor();

thpc_sensor_st THPC_sensor();

batt_sensor_st Batt_sensor();

acc_kpi_st Aceloremeter_kpi();
#pragma once

