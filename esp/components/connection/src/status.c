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

/* Global Task variables */
SemaphoreHandle_t mutex = NULL;
static char send_msgs = 1;
static char task_errors = 0;
static unsigned char payload[PAYLOAD_5_SIZE];


/* BLE Variables */
extern bool is_Aconnected;
extern bool notificationA_enable;
extern bool keep_bt;
extern bool write_EVT;
extern uint8_t *rv_data;
extern struct gatts_profile_inst gl_profile_tab[PROFILE_NUM];

/** @brief Recieves the Configuration through BLE
 * 
 * Waits until the configuration has been written in its characteristic
 * Then it saves it in NVS and deactivates Bluetooth
**/
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

/** @brief Recieves the Configuration through TCP
**/
void tcp_config_task() {
  size_t ssid_len, password_len;
  char recv_buffer[CONFIG_SIZE];
  int port, ip_addr;

  // Read CONFIG for WIFI settings
  read_u32_NVS(&ssid_len, SSID_LENGTH);
  read_u32_NVS(&password_len, PASS_LENGTH);

  char ssid[ssid_len], password[password_len];

  read_str_NVS(ssid, &ssid_len, SSID);
  read_str_NVS(password, &password_len, PASS);
  read_int32_NVS(&ip_addr, HOST_IP_ADDR);
  read_int32_NVS(&port, TCP_PORT);
  
  wifi_init(ssid, password); // Connect to wifi

  int socket = tcp_client_connect(ip_addr, port); // Connect with TCP
  if (socket < 0) {
    ESP_LOGE(STATUS_TAG, "Failed to open TCP socket");
    return;
  }

  // Recieve the configuration
  int len = tcp_client_recv(socket, recv_buffer, TCP_RECV_BUFFER_SIZE);
  if (len < 0) {
    ESP_LOGE(STATUS_TAG, "Failed to receive config through TCP");
    tcp_socket_close(socket);
    return;
  }

  ESP_LOGI(STATUS_TAG, "Recieved %d bytes", len);

  tcp_socket_close(socket);

  // Save the configuration to NVS
  parse_and_save_config((unsigned char *)recv_buffer);
  ESP_LOGI(STATUS_TAG, "Config recieved and saved through TCP");
}

/** @brief Continously send messages through TCP
 * 
 * It connects to the server and sends messages
 * until it recieves a message different from STATUS_OK
 * Then it saves the new status recieved in the message to the NVS
**/
void tcp_continous_task() {
  size_t ssid_len, password_len;
  char recv_buffer;
  int port, ip_addr;
  signed char id_protocol;

  // Read CONFIG for WIFI settings
  read_u32_NVS(&ssid_len, SSID_LENGTH);
  read_u32_NVS(&password_len, PASS_LENGTH);

  char ssid[ssid_len], password[password_len];

  read_str_NVS(ssid, &ssid_len, SSID);
  read_str_NVS(password, &password_len, PASS);
  read_int32_NVS(&ip_addr, HOST_IP_ADDR);
  read_int32_NVS(&port, TCP_PORT);

  // Read CONFIG for message settings
  read_int8_NVS(&id_protocol, ID_PROTOCOL);
  
  wifi_init(ssid, password); // Connect to wifi

  int socket = tcp_client_connect(ip_addr, port); // Connect with TCP
  if (socket < 0) {
    ESP_LOGE(STATUS_TAG, "Failed to open TCP socket");
    return;
  }

  const int payload_len = get_protocol_msg_length(id_protocol);
  char same_status = true;

  while (same_status) {
    tcp_client_send(socket, payload, payload_len, TCP_CONTINOUS_STATUS, id_protocol);
    tcp_client_recv(socket, &recv_buffer, 1);
    same_status = (recv_buffer == STATUS_OK);
  }

  tcp_socket_close(socket);
  write_int8_NVS(recv_buffer, SYSTEM_STATUS); // Write new status
}

/** @brief Send a message through TCP and then go into deep sleep
 * 
 * Connects to the server and sends a message
 * Then it waits for an ACK message
 * If the ACK is STATUS_OK then it goes to sleep
 * for the discontinous time in the NVS minutes
 * Otherwise it saves the new status recieved in the ACK
**/
void tcp_discontinous_task() {
  size_t ssid_len, password_len;
  char recv_buffer;
  int port, ip_addr, sleep_time;
  signed char id_protocol;

  // Read CONFIG for WIFI settings
  read_u32_NVS(&ssid_len, SSID_LENGTH);
  read_u32_NVS(&password_len, PASS_LENGTH);

  char ssid[ssid_len], password[password_len];

  read_str_NVS(ssid, &ssid_len, SSID);
  read_str_NVS(password, &password_len, PASS);
  read_int32_NVS(&ip_addr, HOST_IP_ADDR);
  read_int32_NVS(&port, TCP_PORT);
  read_int32_NVS(&sleep_time, DISC_TIME);

  // Read CONFIG for message settings
  read_int8_NVS(&id_protocol, ID_PROTOCOL);
  
  wifi_init(ssid, password); // Connect to wifi

  int socket = tcp_client_connect(ip_addr, port); // Connect with TCP
  if (socket < 0) {
    ESP_LOGE(STATUS_TAG, "Failed to open TCP socket");
    return;
  }

  const int payload_len = get_protocol_msg_length(id_protocol);

  tcp_client_send(socket, payload, payload_len, TCP_DISCONTINOUS_STATUS, id_protocol);
  tcp_client_recv(socket, &recv_buffer, 1);

  if (recv_buffer == STATUS_OK) {
    deep_sleep_clk(sleep_time);
  } else {
    write_int8_NVS(recv_buffer, SYSTEM_STATUS);
  }

  tcp_socket_close(socket);
}

/** @brief Starts a server for recieving a message to stop the UDP stream
 * 
 * Opens a listening socket and accepts connections
 * If a connection is made then it recieves a message
 * Changes the status in NVS to the recieved message
 * 
 * If there are task errors in the UDP Task then it finishes
**/
void tcp_change_status_task()
{
  int ip_addr; 
  char recv_buffer;

  read_int32_NVS(&ip_addr, HOST_IP_ADDR);

  int socket, listen_socket;
  // Try opening a socket until it can
  do {
    listen_socket = tcp_server_start(CHANGE_STATUS_TCP_PORT); // Start TCP Server
    xSemaphoreTake(mutex, portMAX_DELAY);
    if (task_errors) {
      xSemaphoreGive(mutex);
      tcp_socket_close(listen_socket);
      vTaskDelete(NULL);
      return;
    }
    xSemaphoreGive(mutex);
  } while (listen_socket < 0);

  // Try accepting a new connection until it does
  do {
    socket = tcp_server_accept(listen_socket); // Start TCP Server
    xSemaphoreTake(mutex, portMAX_DELAY);
    if (task_errors) {
      xSemaphoreGive(mutex);
      tcp_socket_close(socket);
      tcp_socket_close(listen_socket);
      vTaskDelete(NULL);
      return;
    }
    xSemaphoreGive(mutex);
  } while (socket < 0);
  
  // Try to recieve a message until it does
  while (1) {
    xSemaphoreTake(mutex, portMAX_DELAY);
    if (task_errors) {
      tcp_socket_close(socket);
      tcp_socket_close(listen_socket);
      vTaskDelete(NULL);
      return;
    }
    xSemaphoreGive(mutex);
    int len = tcp_client_recv(socket, &recv_buffer, 1);
    if (len >= 0) {
      break;
    }
  }

  xSemaphoreTake(mutex, portMAX_DELAY);
  // Stop UDP stream
  send_msgs = 0;
  xSemaphoreGive(mutex);

  tcp_socket_close(socket);
  tcp_socket_close(listen_socket);

  write_int8_NVS(recv_buffer, SYSTEM_STATUS);
  vTaskDelete(NULL);
}

/** @brief Sends UDP stream messages
 * 
 * Starts a UDP socket and starts sending messages
 * until the TCP task sets send_messages to 0
 * 
 * If it fails it stops the TCP Task
**/
void udp_task()
{
  int port, ip_addr;
  signed char id_protocol;

  read_int32_NVS(&ip_addr, HOST_IP_ADDR);
  read_int32_NVS(&port, UDP_PORT);

  // Read CONFIG for message settings
  read_int8_NVS(&id_protocol, ID_PROTOCOL);

  int socket = udp_client_connect(); // Connect with UDP
  if (socket < 0) {
    ESP_LOGE(STATUS_TAG, "Failed to open UDP socket");
    xSemaphoreTake(mutex, portMAX_DELAY);
    task_errors = 1;
    xSemaphoreGive(mutex);
    return;
  }

  const int payload_len = get_protocol_msg_length(id_protocol);

  while (1) {
    xSemaphoreTake(mutex, portMAX_DELAY);
    if (!send_msgs || task_errors) {
        xSemaphoreGive(mutex);
        break;
    }
    xSemaphoreGive(mutex);
    udp_client_send(socket, ip_addr, port, payload, payload_len, UDP_STATUS, id_protocol);
    vTaskDelay(pdMS_TO_TICKS(100));
  }

  udp_socket_close(socket);
}

/** @brief Starts the TCP server task and UDP task to stream messages
 * 
 * It starts the TCP server task that recieves a message from the server to
 * stop the UDP stream
 * It also starts the UDP stream task
 * Waits until both of those functions end
**/
void udp_continous_task() {
  size_t ssid_len, password_len;

  // Read CONFIG for WIFI settings
  read_u32_NVS(&ssid_len, SSID_LENGTH);
  read_u32_NVS(&password_len, PASS_LENGTH);

  char ssid[ssid_len], password[password_len];

  read_str_NVS(ssid, &ssid_len, SSID);
  read_str_NVS(password, &password_len, PASS);
  
  wifi_init(ssid, password); // Connect to wifi

  mutex = xSemaphoreCreateMutex();
  if (mutex != NULL) {
    // Start TCP task
    xTaskCreate(tcp_change_status_task, "tcp_status_client", 4096, NULL, 5, NULL);
    // Start UDP task
    udp_task();

    // Reset global values
    xSemaphoreTake(mutex, portMAX_DELAY);
    task_errors = 0;
    send_msgs = 1;
    xSemaphoreGive(mutex);
  }
}

/** @brief Continously sends messages through BLE notifications
**/
void ble_continous_task() {
  ble_server_init(500);
  static bool inicio = true;
  signed char id_protocol;

  // Read CONFIG for message settings
  read_int8_NVS(&id_protocol, ID_PROTOCOL);

  while (keep_bt) {
    vTaskDelay(pdMS_TO_TICKS(1000)); // need to delay otherwise CPU watchdogs are triggered
    if(is_Aconnected && inicio){
      ESP_LOGI(STATUS_TAG, "BLE Connection established");
      inicio = false;
    }

    if(is_Aconnected && notificationA_enable){
      ble_send_full_payload(BLE_CONTINOUS_STATUS, id_protocol, false);
      ESP_LOGI(STATUS_TAG, "Sent data through BLE");
    }
  }

  ESP_LOGI(STATUS_TAG, "Stopped continous BLE");

  // Saved the status written to NVS
  write_int8_NVS(*rv_data, SYSTEM_STATUS);
  free(rv_data);

  ble_server_deinit();
}

/** @brief Send a message through BLE and then go into deep sleep
 * 
 * Starts Bluetooth services
 * Then it waits for an ACK message
 * If the ACK is STATUS_OK then it goes to sleep
 * for the discontinous time in the NVS minutes
 * Otherwise it saves the new status recieved in the ACK
**/
void ble_discontinous_task() {
  ble_server_init(500);
  int sleep_time;
  signed char id_protocol;

  // Read CONFIG for message settings
  read_int8_NVS(&id_protocol, ID_PROTOCOL);
  read_int32_NVS(&sleep_time, DISC_TIME);

  // Wait for connection
  while (true) {
    if (is_Aconnected) {
      // Send messages until confirmation has been recieved
      ESP_LOGI(STATUS_TAG, "BLE Connection established");
      ble_send_full_payload(BLE_DISCONTINOUS_STATUS, id_protocol, false);
      ESP_LOGI(STATUS_TAG, "Sent data through BLE");
    }
    if (write_EVT) {
      break;
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }

  // Read ACK message
  if (*rv_data == STATUS_OK) {
    free(rv_data);
    deep_sleep_clk(sleep_time);
  } else {
    write_int8_NVS(*rv_data, SYSTEM_STATUS);
    free(rv_data);
  }

  ble_server_deinit();
}

/** @brief Switches to the status in NVS and starts its corresponding task
 * 
 * Reads the status from NVS and starts the task of that corresponding status
 * If the status is invalid then it goes to BLE_CONFIG_STATUS
**/
void switch_status()
{
  signed char status;
  int err = read_int8_NVS(&status, SYSTEM_STATUS);
  if (err < 0) {
    status = BLE_CONFIG_STATUS;
    write_int8_NVS(BLE_CONFIG_STATUS, SYSTEM_STATUS);
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
    write_int8_NVS(BLE_CONFIG_STATUS, SYSTEM_STATUS);
    break;
  }
}