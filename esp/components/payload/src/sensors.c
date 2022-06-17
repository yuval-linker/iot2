#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "sensors.h"

/*
Auxiliary functions
*/

unsigned int get_timestamp() {
    return (unsigned int) time(NULL);
}

float rms(float amp_x, float amp_y, float amp_z) {
    return sqrt(amp_x * amp_x + amp_y * amp_y + amp_z * amp_z);
}


int randint(int start, int end) {
    srand(time(NULL));
    return (rand() % (end - start + 1)) + start;
}

float randfloat(float start, float end) {
    float delta  = end - start;
    srand(time(NULL));
    return ((float) rand() / (float) (RAND_MAX)) * delta + start;
}

/*
Simulated sensor functions
*/

acc_sensor_st Accelerometer_sensor() {
    acc_sensor_st acc_data;
    for (int i = 0; i < ACC_VECTOR_SIZE; i++) {
        acc_data.acc_x[i] = (float) 2*sin(2*M_PI*0.001*i);
        acc_data.acc_y[i] = (float) 3*cos(2*M_PI*0.001*i);
        acc_data.acc_z[i] = (float) 10*sin(2*M_PI*0.001*i);
    }
    return acc_data;
}

thpc_sensor_st THPC_sensor() {
    thpc_sensor_st thpc_data = {
        .temp = randint(TEMP_INF, TEMP_SUP),
        .hum = randint(HUM_INF, HUM_SUP),
        .press = randint(PRESS_INF, PRESS_SUP),
        .co = randfloat(CO_INF, CO_SUP)
    };
    return thpc_data;
}

batt_sensor_st Batt_sensor() {
    batt_sensor_st batt_data = {.value = (char) randint(BATT_INF, BATT_SUP)};
    return batt_data;
}

acc_kpi_st Aceloremeter_kpi() {
    float amp_x = randfloat(AMP_X_INF, AMP_X_SUP);
    float amp_y = randfloat(AMP_Y_INF, AMP_Y_SUP);
    float amp_z = randfloat(AMP_Z_INF, AMP_Z_SUP);
    acc_kpi_st acc_kpi_data = {
        .rms = rms(amp_x, amp_y, amp_z),
        .amp_x = amp_x,
        .frec_x = randfloat(FREC_X_INF, FREC_X_SUP),
        .amp_y = amp_y,
        .frec_y = randfloat(FREC_Y_INF, FREC_Y_SUP),
        .amp_z = amp_z,
        .frec_z = randfloat(FREC_Z_INF, FREC_Z_SUP)
    };
    return acc_kpi_data;
}
