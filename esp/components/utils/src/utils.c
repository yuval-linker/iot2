/*
* Copyright (C) 2019 Center for Industry 4.0
* All Rights Reserved
*
* Center_for_Industry_4.0_LICENSE_PLACEHOLDER
* Desarrolladores: Enrique Germany, Luciano Radrigan
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_sleep.h"
#include "esp_log.h"
#include "utils.h"

const char* UTILS_TAG = "UTILS";

int Write_NVS(int32_t data, int key)
{
    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );

    // Open
    nvs_handle_t my_handle;
    err = nvs_open("Storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(UTILS_TAG, "Error (%d) opening NVS handle!", err);
        return -1;
    } else {
        // Write
        switch (key)
        {
            case 1:
                err = nvs_set_i32(my_handle, "sys_sta", data);
                break;
            case 2:
                err = nvs_set_i32(my_handle, "Samp_Freq", data);
                break;
            case 3:
                err = nvs_set_i32(my_handle, "T_s", data);
                break;
            case 4:
                err = nvs_set_i32(my_handle, "Acc_Sen", data);
                break;
            case 5:
                err = nvs_set_i32(my_handle, "Gyro_Sen", data);
                break;
            case 6:
                err = nvs_set_i32(my_handle, "Acc_Any", data);
                break;
            case 7:
                err = nvs_set_i32(my_handle, "Rf_Cal", data);
                break;
            case 8:
                err = nvs_set_i32(my_handle, "SEL_ID", data);
                break;

            default:
                ESP_LOGE(UTILS_TAG, "ERROR key");
                break;
        }
        err = nvs_commit(my_handle);
        if (err != ESP_OK) {
            ESP_LOGE(UTILS_TAG, "Failed commiting to NVS")
        }
        // Close
        nvs_close(my_handle);
    }
    return 0;
}

int Read_NVS(int32_t* data, int key)
{
    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    ESP_ERROR_CHECK( err );

    // Open
    nvs_handle_t my_handle;
    err = nvs_open("Storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(UTILS_TAG, "Error (%d) opening NVS handle!\n", err);
        return -1;
    } else {
        // Read
        switch (key)
        {
            case 1:
                err = nvs_get_i32(my_handle, "sys_sta", data);
                break;
            case 2:
                err = nvs_get_i32(my_handle, "Samp_Freq", data);
                break;
            case 3:
                err = nvs_get_i32(my_handle, "T_s", data);
                break;
            case 4:
                err = nvs_get_i32(my_handle, "Acc_Sen", data);
                break;
            case 5:
                err = nvs_get_i32(my_handle, "Gyro_Sen", data);
                break;
            case 6:
                err = nvs_get_i32(my_handle, "Acc_Any", data);
                break;
            case 7:
                err = nvs_get_i32(my_handle, "Rf_Cal", data);
                break;
            case 8:
                err = nvs_get_i32(my_handle, "SEL_ID", data);
                break;

            default:
                printf("ERROR key");
                break;
        }
        switch (err) {
            case ESP_OK:
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                ESP_LOGE(UTILS_TAG, "The value is not initialized yet!");
                break;
            default :
                ESP_LOGE(UTILS_TAG, "Error (%d) reading!", err);
        }
        // Close
        nvs_close(my_handle);
    }
    return 0;
}

void deep_sleep_clk(int wakeup_time_sec) {
    ESP_LOGI(UTILS_TAG, "Enabling timer wakeup, %lld", wakeup_time_sec);
    esp_sleep_enable_timer_wakeup(wakeup_time_sec*1000000); //* 1.000.000
    esp_deep_sleep_start();
}
