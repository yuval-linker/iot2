#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_sleep.h"
#include "esp_log.h"

#include "recv_payload.h"
#include "system_utils.h"

const char* UTILS_TAG = "UTILS";

/** @brief Writes a char to the Non-Volatile Memory
    * @param data The char to be saved
    * @param key Key name

    * @return 0 if succesful, -1 otherwise
**/
int write_int8_NVS(int8_t data, int key)
{
    esp_err_t err;
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
            case SYSTEM_STATUS:
                err = nvs_set_i8(my_handle, "sys_status", data);
                break;
            case ID_PROTOCOL:
                err = nvs_set_i8(my_handle, "id_protocol", data);
                break;
            default:
                ESP_LOGE(UTILS_TAG, "ERROR key");
                break;
        }
        err = nvs_commit(my_handle);
        if (err != ESP_OK) {
            ESP_LOGE(UTILS_TAG, "Failed commiting to NVS");
        }
        // Close
        nvs_close(my_handle);
    }
    return 0;
}

/** @brief Writes a 32-bit int to the Non-Volatile Memory
    * @param data The int to be saved
    * @param key Key name

    * @return 0 if succesful, -1 otherwise
**/
int write_int32_NVS(int32_t data, int key)
{
    esp_err_t err;

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
            case SYSTEM_STATUS:
                err = nvs_set_i32(my_handle, "sys_status", data);
                break;
            case ID_PROTOCOL:
                err = nvs_set_i32(my_handle, "id_protocol", data);
                break;
            case BMI270_SAMPLING:
                err = nvs_set_i32(my_handle, "270_sampling", data);
                break;
            case ACC_SEN:
                err = nvs_set_i32(my_handle, "Acc_Sen", data);
                break;
            case GYR_SEN:
                err = nvs_set_i32(my_handle, "Gyro_Sen", data);
                break;
            case BME688_SAMPLING:
                err = nvs_set_i32(my_handle, "688_sampling", data);
                break;
            case DISC_TIME:
                err = nvs_set_i32(my_handle, "disc_time", data);
                break;
            case TCP_PORT:
                err = nvs_set_i32(my_handle, "tcp_port", data);
                break;
            case UDP_PORT:
                err = nvs_set_i32(my_handle, "udp_port", data);
                break;
            case HOST_IP_ADDR:
                err = nvs_set_i32(my_handle, "host_ip_addr", data);
                break;
            default:
                ESP_LOGE(UTILS_TAG, "ERROR key");
                break;
        }
        err = nvs_commit(my_handle);
        if (err != ESP_OK) {
            ESP_LOGE(UTILS_TAG, "Failed commiting to NVS");
        }
        // Close
        nvs_close(my_handle);
    }
    return 0;
}

/** @brief Writes a string to the Non-Volatile Memory
    * @param data The string to be saved
    * @param key Key name

    * @return 0 if succesful, -1 otherwise
**/
int write_str_NVS(char *data, int key)
{
    esp_err_t err;

    // Open
    nvs_handle_t my_handle;
    err = nvs_open("Storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(UTILS_TAG, "Error (%d) opening NVS handle!", err);
        return -1;
    } else {
        size_t str_size = strlen(data) + 1;
        // Write
        switch (key)
        {
            case SSID:
                nvs_set_u32(my_handle, "ssid_len", str_size);
                nvs_set_str(my_handle, "ssid", data);
                break;
            case PASS:
                nvs_set_u32(my_handle, "pass_len", str_size);
                nvs_set_str(my_handle, "pass", data);
                break;
            default:
                ESP_LOGE(UTILS_TAG, "ERROR key");
                break;
        }
        err = nvs_commit(my_handle);
        if (err != ESP_OK) {
            ESP_LOGE(UTILS_TAG, "Failed commiting to NVS");
        }
        // Close
        nvs_close(my_handle);
    }
    return 0;
}

/** @brief Reads a char from Non-Volatile Memory
    * @param data The char where to write the written value
    * @param key Key name

    * @return 0 if succesful, -1 otherwise
**/
int read_int8_NVS(int8_t* data, int key)
{
    esp_err_t err;

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
            case SYSTEM_STATUS:
                err = nvs_get_i8(my_handle, "sys_status", data);
                break;
            case ID_PROTOCOL:
                err = nvs_get_i8(my_handle, "id_protocol", data);
                break;
            default:
                ESP_LOGE(UTILS_TAG, "ERROR key");
                break;
        }
        switch (err) {
            case ESP_OK:
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                ESP_LOGE(UTILS_TAG, "The value is not initialized yet!");
                nvs_close(my_handle);
                return -1;
            default :
                ESP_LOGE(UTILS_TAG, "Error (%d) reading!", err);
                nvs_close(my_handle);
                return -1;
        }
        // Close
        nvs_close(my_handle);
    }
    return 0;
}

/** @brief Reads a 32-bit int from Non-Volatile Memory
    * @param data The int where to write the written value
    * @param key Key name

    * @return 0 if succesful, -1 otherwise
**/
int read_int32_NVS(int32_t* data, int key)
{
    esp_err_t err;

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
            case BMI270_SAMPLING:
                err = nvs_get_i32(my_handle, "270_sampling", data);
                break;
            case ACC_SEN:
                err = nvs_get_i32(my_handle, "Acc_Sen", data);
                break;
            case GYR_SEN:
                err = nvs_get_i32(my_handle, "Gyro_Sen", data);
                break;
            case BME688_SAMPLING:
                err = nvs_get_i32(my_handle, "688_sampling", data);
                break;
            case DISC_TIME:
                err = nvs_get_i32(my_handle, "disc_time", data);
                break;
            case TCP_PORT:
                err = nvs_get_i32(my_handle, "tcp_port", data);
                break;
            case UDP_PORT:
                err = nvs_get_i32(my_handle, "udp_port", data);
                break;
            case HOST_IP_ADDR:
                err = nvs_get_i32(my_handle, "host_ip_addr", data);
                break;
            case SSID:
                err = nvs_get_i32(my_handle, "ssid", data);
                break;
            case PASS:
                err = nvs_get_i32(my_handle, "pass", data);
                break;

            default:
                ESP_LOGE(UTILS_TAG, "ERROR key");
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

/** @brief Reads a string from Non-Volatile Memory
    * @param data The char buffer where to write the written value
    * @param length A pointer to the length of the string to be read
    * @param key Key name

    * @return 0 if succesful, -1 otherwise
**/
int read_str_NVS(char* data, size_t *length, int key)
{
    esp_err_t err;

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
            case SSID:
                err = nvs_get_str(my_handle, "ssid", data, length);
                break;
            case PASS:
                err = nvs_get_str(my_handle, "pass", data, length);
                break;

            default:
                ESP_LOGE(UTILS_TAG, "ERROR key");
                break;
        }
        switch (err) {
            case ESP_OK:
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                ESP_LOGE(UTILS_TAG, "The value is not initialized yet!");
                break;
            default :
                ESP_LOGE(UTILS_TAG, "Error (%s) reading!", esp_err_to_name(err));
        }
        // Close
        nvs_close(my_handle);
    }
    return 0;
}

/** @brief Reads an unsigned 32-bit int from Non-Volatile Memory
    * @param data The unsigned int where to write the written value
    * @param key Key name

    * @return 0 if succesful, -1 otherwise
**/
int read_u32_NVS(uint32_t *data, int key)
{
    esp_err_t err;

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
            case SSID_LENGTH:
                nvs_get_u32(my_handle, "ssid_len", data);
                break;
            case PASS_LENGTH:
                nvs_get_u32(my_handle, "pass_len", data);
                break;
            default:
                ESP_LOGE(UTILS_TAG, "ERROR key");
                break;
        }
        switch (err) {
            case ESP_OK:
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                ESP_LOGE(UTILS_TAG, "The value is not initialized yet!");
                break;
            default:
                ESP_LOGE(UTILS_TAG, "Error (%d) reading!", err);
                break;
        }
        // Close
        nvs_close(my_handle);
    }
    return 0;
}

/** @brief Parses a message with a configuration and saves it into NVS
    * @param payload The bytes buffer with the encoded configuration
**/
void parse_and_save_config(unsigned char *payload)
{
    int err;
    config_data_st config;
    decode_config_data(payload, &config);

    err = write_int8_NVS(config.status, SYSTEM_STATUS);
    if (err < 0) {
        ESP_LOGE(UTILS_TAG, "Can't commit system status");
    }
    err = write_int8_NVS(config.id_protocol, ID_PROTOCOL);
    if (err < 0) {
        ESP_LOGE(UTILS_TAG, "Can't commit prtocol id");
    }
    err = write_int32_NVS(config.bmi270_sampling, BMI270_SAMPLING);
    if (err < 0) {
        ESP_LOGE(UTILS_TAG, "Can't commit bmi270 sampling");
    }
    err = write_int32_NVS(config.bmi270_acc_sensibility, ACC_SEN);
    if (err < 0) {
        ESP_LOGE(UTILS_TAG, "Can't commit bmi270 acc sensibility");
    }
    err = write_int32_NVS(config.bmi270_gyro_sensibility, GYR_SEN);
    if (err < 0) {
        ESP_LOGE(UTILS_TAG, "Can't commit bmi270 gyro sensibility");
    }
    err = write_int32_NVS(config.bme688_sampling, BME688_SAMPLING);
    if (err < 0) {
        ESP_LOGE(UTILS_TAG, "Can't commit bme688 sampling");
    }
    err = write_int32_NVS(config.discontinous_time, DISC_TIME);
    if (err < 0) {
        ESP_LOGE(UTILS_TAG, "Can't commit discontinous time");
    }
    err = write_int32_NVS(config.tcp_port, TCP_PORT);
    ESP_LOGI(UTILS_TAG, "TCP PORT CONFIG %d", config.tcp_port);
    if (err < 0) {
        ESP_LOGE(UTILS_TAG, "Can't commit TCP port");
    }
    err = write_int32_NVS(config.udp_port, UDP_PORT);
    if (err < 0) {
        ESP_LOGE(UTILS_TAG, "Can't commit UDP port");
    }
    ESP_LOGI(UTILS_TAG, "Recieved IP ADDR %u", config.host_ip_addr);
    err = write_int32_NVS(config.host_ip_addr, HOST_IP_ADDR);
    if (err < 0) {
        ESP_LOGE(UTILS_TAG, "Can't commit host ip address");
    }
    err = write_str_NVS((char *)config.ssid, SSID);
    if (err < 0) {
        ESP_LOGE(UTILS_TAG, "Can't commit ssid");
    }
    err = write_str_NVS((char *)config.pass, PASS);
    if (err < 0) {
        ESP_LOGE(UTILS_TAG, "Can't commit password");
    }
}

/** @brief Returns the message length for the protocol given
    * @param id_protocol The protocol ID

    * @return The length of the message for the given protocol. Returns -1 if protocol invalid
**/
int get_protocol_msg_length(char id_protocol)
{
    int msg_length;
    switch (id_protocol)
    {
    case 0:
        msg_length = PAYLOAD_0_SIZE;
        break;
    case 1:
        msg_length = PAYLOAD_1_SIZE;
        break;
    case 2:
        msg_length = PAYLOAD_2_SIZE;
        break;
    case 3:
        msg_length = PAYLOAD_3_SIZE;
        break;
    case 4:
        msg_length = PAYLOAD_4_SIZE;
        break;
    case 5:
        msg_length = PAYLOAD_5_SIZE;
        break;
    
    default:
        msg_length = -1;
        break;
    }
    return msg_length;
}

/** @brief Enter the device into deep sleep for the given duration
    * @param wakeup_time_min The time (in minutes) that the ESP has to wake up
**/
void deep_sleep_clk(int wakeup_time_min)
{
    ESP_LOGI(UTILS_TAG, "Enabling timer wakeup, %d", wakeup_time_min);
    esp_sleep_enable_timer_wakeup(wakeup_time_min*1000000*60); //* 1.000.000
    esp_deep_sleep_start();
}
