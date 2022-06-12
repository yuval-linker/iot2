#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"

#include "payload_sizes.h"
#include "system_utils.h"
#include "wifi.h"
#include "client_tcp.h"
#include "client_udp.h"
#include "ble_server.h"
#include "status.h"

const char *STATUS_TAG = "STATUS";
SemaphoreHandle_t mutex = NULL;
static char send_msgs = 1;
static char task_errors = 0;

/* BLE Variables */
extern bool is_Aconnected;
extern bool notificationA_enable;
extern bool keep_bt;
extern bool write_EVT;
extern uint8_t *rv_data;
extern struct gatts_profile_inst gl_profile_tab[PROFILE_NUM];

void ble_config_task() {
  ble_server_init(500);
  static bool inicio = true;

  while (keep_bt) {
    vTaskDelay(pdMS_TO_TICKS(10));
    if(is_Aconnected && inicio){
      ESP_LOGI(STATUS_TAG, "BLE Connection established");
      inicio = false;
    }
  }
  ble_server_deinit();
}

void tcp_config_task() {
  size_t *ssid_len, *password_len;
  char *ssid, *password, recv_buffer[CONFIG_SIZE];
  unsigned char string_ip_addr[4];
  int port, ip_addr;

  // Read CONFIG for WIFI settings
  read_u32_NVS(ssid_len, SSID_LENGTH);
  read_u32_NVS(password_len, PASS_LENGTH);
  read_str_NVS(ssid, ssid_len, SSID);
  read_str_NVS(password, password_len, PASS);
  read_int32_NVS(&ip_addr, HOST_IP_ADDR);
  read_int32_NVS(&port, TCP_PORT);
  int_ip_to_str(ip_addr, string_ip_addr);
  
  wifi_init(ssid, password); // Connect to wifi

  int socket = tcp_connect(string_ip_addr, port); // Connect with TCP
  if (socket < 0) {
    ESP_LOGE(STATUS_TAG, "Failed to open TCP socket");
    return;
  }

  int len = tcp_recv(socket, recv_buffer, TCP_RECV_BUFFER_SIZE);
  if (len < 0) {
    ESP_LOGE(STATUS_TAG, "Failed to receive config through TCP");
    return;
  }

  tcp_socket_close(socket);

  parse_and_save_config(recv_buffer);
  ESP_LOGI(STATUS_TAG, "Config recieved and saved through TCP");
}

void tcp_continous_task() {
  size_t *ssid_len, *password_len;
  char *ssid, *password, *recv_buffer;
  unsigned char string_ip_addr[4];
  int port, ip_addr;
  char id_protocol;

  // Read CONFIG for WIFI settings
  read_u32_NVS(ssid_len, SSID_LENGTH);
  read_u32_NVS(password_len, PASS_LENGTH);
  read_str_NVS(ssid, ssid_len, SSID);
  read_str_NVS(password, password_len, PASS);
  read_int32_NVS(&ip_addr, HOST_IP_ADDR);
  read_int32_NVS(&port, TCP_PORT);
  int_ip_to_str(ip_addr, string_ip_addr);

  // Read CONFIG for message settings
  read_int8_NVS(&id_protocol, ID_PROTOCOL);
  
  wifi_init(ssid, password); // Connect to wifi

  int socket = tcp_connect(string_ip_addr, port); // Connect with TCP
  if (socket < 0) {
    ESP_LOGE(STATUS_TAG, "Failed to open TCP socket");
    return;
  }

  const int payload_len = get_protocol_msg_length(id_protocol);
  unsigned char payload[payload_len];
  char same_status = 1;

  while (same_status) {
    tcp_send(socket, payload, payload_len, TCP_CONTINOUS_STATUS, id_protocol);
    tcp_recv(socket, recv_buffer, 1);
    same_status = (*recv_buffer == STATUS_OK);
  }

  tcp_socket_close(socket);

  write_int8_NVS(*recv_buffer, SYSTEM_STATUS);
}

void tcp_discontinous_task() {
  size_t *ssid_len, *password_len;
  char *ssid, *password, *recv_buffer;
  unsigned char string_ip_addr[4];
  int port, ip_addr, sleep_time;
  char id_protocol;

  // Read CONFIG for WIFI settings
  read_u32_NVS(ssid_len, SSID_LENGTH);
  read_u32_NVS(password_len, PASS_LENGTH);
  read_str_NVS(ssid, ssid_len, SSID);
  read_str_NVS(password, password_len, PASS);
  read_int32_NVS(&ip_addr, HOST_IP_ADDR);
  read_int32_NVS(&port, TCP_PORT);
  read_int32_NVS(&sleep_time, DISC_TIME);
  int_ip_to_str(ip_addr, string_ip_addr);

  // Read CONFIG for message settings
  read_int8_NVS(&id_protocol, ID_PROTOCOL);
  
  wifi_init(ssid, password); // Connect to wifi

  int socket = tcp_connect(string_ip_addr, port); // Connect with TCP
  if (socket < 0) {
    ESP_LOGE(STATUS_TAG, "Failed to open TCP socket");
    return;
  }

  const int payload_len = get_protocol_msg_length(id_protocol);
  unsigned char payload[payload_len];

  tcp_send(socket, payload, payload_len, TCP_CONTINOUS_STATUS, id_protocol);
  tcp_recv(socket, recv_buffer, 1);

  if (*recv_buffer == STATUS_OK) {
    deep_sleep_clk(sleep_time);
  } else {
    write_int8_NVS(*recv_buffer, SYSTEM_STATUS);
  }

  tcp_socket_close(socket);
}

void tcp_change_status_task()
{
  int port, ip_addr;
  char id_protocol, *recv_buffer;
  unsigned char string_ip_addr[4];

  read_int32_NVS(&ip_addr, HOST_IP_ADDR);
  read_int32_NVS(&port, TCP_PORT);
  int_ip_to_str(ip_addr, string_ip_addr);

  // Read CONFIG for message settings
  read_int8_NVS(&id_protocol, ID_PROTOCOL);
  int socket;
  socket = tcp_connect(string_ip_addr, port); // Connect with TCP
  if (socket < 0) {
    xSemaphoreTake(mutex, portMAX_DELAY);
    task_errors = 1;
    xSemaphoreGive(mutex);
    ESP_LOGE(STATUS_TAG, "Failed to open TCP socket");
    return;
  }
  
  while (1) {
    xSemaphoreTake(mutex, portMAX_DELAY);
    if (task_errors) {
      tcp_socket_close(socket);
      return;
    }
    xSemaphoreGive(mutex);
    int len = tcp_recv(socket, recv_buffer, 1);
    if (len >= 0) {
      break;
    }
    ESP_LOGE(STATUS_TAG, "Failed to receive status change through TCP");
  }

  xSemaphoreTake(mutex, portMAX_DELAY);
  send_msgs = 0;
  xSemaphoreGive(mutex);

  tcp_socket_close(socket);

  write_int8_NVS(*recv_buffer, SYSTEM_STATUS);
}

void udp_task()
{
  int port, ip_addr;
  char id_protocol, *recv_buffer;
  unsigned char string_ip_addr[4];

  read_int32_NVS(&ip_addr, HOST_IP_ADDR);
  read_int32_NVS(&port, TCP_PORT);
  int_ip_to_str(ip_addr, string_ip_addr);

  // Read CONFIG for message settings
  read_int8_NVS(&id_protocol, ID_PROTOCOL);

  socket_connection_t connection = udp_connect(string_ip_addr, port); // Connect with UDP
  if (connection.socket < 0) {
    ESP_LOGE(STATUS_TAG, "Failed to open UDP socket");
    xSemaphoreTake(mutex, portMAX_DELAY);
    task_errors = 1;
    xSemaphoreGive(mutex);
    return;
  }

  const int payload_len = get_protocol_msg_length(id_protocol);
  unsigned char payload[payload_len];

  while (1) {
    xSemaphoreTake(mutex, portMAX_DELAY);
    if (!send_msgs || task_errors) {
        xSemaphoreGive(mutex);
        break;
    }
    xSemaphoreGive(mutex);
    udp_send(&connection, payload, payload_len, id_protocol);
  }

  udp_socket_close(connection.socket);
}

void udp_continous_task() {
  size_t *ssid_len, *password_len;
  char *ssid, *password, *recv_buffer;

  // Read CONFIG for WIFI settings
  read_u32_NVS(ssid_len, SSID_LENGTH);
  read_u32_NVS(password_len, PASS_LENGTH);
  read_str_NVS(ssid, ssid_len, SSID);
  read_str_NVS(password, password_len, PASS);
  
  wifi_init(ssid, password); // Connect to wifi

  mutex = xSemaphoreCreateMutex();
  if (mutex != NULL) {
    xTaskCreate(tcp_change_status_task, "tcp_status_client", 4096, NULL, 5, NULL);
    udp_task();
    xSemaphoreTake(mutex, portMAX_DELAY);
    task_errors = 0;
    send_msgs = 1;
    xSemaphoreGive(mutex);
  }
}

void ble_continous_task() {
  ble_server_init(500);
  static bool inicio = true;
  unsigned char *payload;
  char id_protocol;

  // Read CONFIG for message settings
  read_int8_NVS(&id_protocol, ID_PROTOCOL);

  const int payload_len = get_protocol_msg_length(id_protocol);
  unsigned char payload[payload_len];

  while (keep_bt) {
    vTaskDelay(pdMS_TO_TICKS(10));
    if(is_Aconnected && inicio){
      ESP_LOGI(STATUS_TAG, "BLE Connection established");
      inicio = false;
    }

    if(is_Aconnected && notificationA_enable){
      ESP_ERROR_CHECK(ble_send_payload(payload, payload_len,  BLE_CONTINOUS_STATUS, id_protocol, false));
      ESP_LOGI(STATUS_TAG, "Sent data through BLE");
    }
  }
  ble_server_deinit();
}

void ble_discontinous_task() {
  ble_server_init(500);
  int sleep_time;
  unsigned char *payload;
  char id_protocol;

  // Read CONFIG for message settings
  read_int8_NVS(&id_protocol, ID_PROTOCOL);
  read_int32_NVS(&sleep_time, DISC_TIME);

  const int payload_len = get_protocol_msg_length(id_protocol);
  unsigned char payload[payload_len];

  // Wait for connection
  while (!is_Aconnected) {
    vTaskDelay(pdMS_TO_TICKS(10));
  }
  ESP_LOGI(STATUS_TAG, "BLE Connection established");

  ESP_ERROR_CHECK(ble_send_payload(payload, payload_len,  BLE_CONTINOUS_STATUS, id_protocol, false));
  ESP_LOGI(STATUS_TAG, "Sent data through BLE");

  // Wait for response
  while (!write_EVT) {
    vTaskDelay(pdMS_TO_TICKS(10));
  }

  if (*rv_data == STATUS_OK) {
    deep_sleep_clk(sleep_time);
  } else {
    write_int8_NVS(*rv_data, SYSTEM_STATUS);
  }

  ble_server_deinit();
}

void switch_status()
{
  char status;
  int err = read_int8_NVS(&status, SYSTEM_STATUS);
  if (err < 0) {
    status = BLE_CONFIG_STATUS;
  }
  switch (status)
  {
  case BLE_CONFIG_STATUS:
    ESP_LOGI(STATUS_TAG, "Config through BLE");
    ble_config_task();
    break;
  case TCP_CONFIG_STATUS:
    ESP_LOGI(STATUS_TAG, "Config through TCP");
    tcp_config_task();
    break;
  case TCP_CONTINOUS_STATUS:
    ESP_LOGI(STATUS_TAG, "TCP Continous communication");
    tcp_continous_task();
    break;
  case TCP_DISCONTINOUS_STATUS:
    ESP_LOGI(STATUS_TAG, "TCP Discontinous communication");
    tcp_discontinous_task();
    break;
  case UDP_STATUS:
    ESP_LOGI(STATUS_TAG, "UDP Communication");
    udp_continous_task();
    break;
  case BLE_CONTINOUS_STATUS:
    ESP_LOGI(STATUS_TAG, "BLE Continous communication");
    ble_continous_task();
    break;
  case BLE_DISCONTINOUS_STATUS:
    ESP_LOGI(STATUS_TAG, "BLE Discontinous communication");
    ble_discontinous_task();
    break;
  
  default:
    ESP_LOGE(STATUS_TAG, "Invalid status");
    break;
  }
}