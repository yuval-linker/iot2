#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include "send_payload.h"
#include "client_tcp.h"

static const char *TCP_TAG = "ESP32 TCP";

RTC_DATA_ATTR unsigned int payload_type = 0;

int tcp_connect(int host_ip, int port) {
  int addr_family = AF_INET;
  int ip_protocol = IPPROTO_IP;
  struct sockaddr_in dest_addr;
  dest_addr.sin_addr.s_addr = inet_addr(host_ip);
  dest_addr.sin_family = AF_INET;
  dest_addr.sin_port = htons(port);
  int sock =  socket(addr_family, SOCK_STREAM, ip_protocol);
  if (sock < 0) {
      ESP_LOGE(TCP_TAG, "Unable to create socket: errno %d", errno);
      return;
  }
  ESP_LOGI(TCP_TAG, "Socket created, connecting to %s:%d", host_ip, port);

  int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_in6));
  if (err != 0) {
      ESP_LOGE(TCP_TAG, "Socket unable to connect: errno %d", errno);
      return;
  }
  ESP_LOGI(TCP_TAG, "Successfully connected");
  return sock;
}


void tcp_send(int sock, unsigned char* payload, int payload_size, char protocol_id) {
  bzero(payload, payload_size);
  send_payload((unsigned char *) payload, (char) protocol_id, (unsigned short) ID_DEVICE, MAC_DEVICE);
  int err = send(sock, payload, payload_size, 0);
  if (err < 0) {
      ESP_LOGE(TCP_TAG, "Error occurred during sending: errno %d", errno);
      return;
  }
  ESP_LOGI(TCP_TAG, "Message sent");
}

int tcp_recv(int sock, char* buffer, int buffer_size) {
  int len = recv(sock, buffer, sizeof(buffer_size) - 1, 0);
  // Error occurred during receiving
  if (len < 0) {
      ESP_LOGE(TCP_TAG, "recv failed: errno %d", errno);
      return -1;
  }
  // Data received
  else {
      buffer[len] = 0; // Null-terminate whatever we received and treat like a string
      ESP_LOGI(TCP_TAG, "Received %d bytes", len);
      ESP_LOGI(TCP_TAG, "Message: %s", buffer);
  }
  return len;
}

void tcp_socket_close(int sock) {
  ESP_LOGE(TCP_TAG, "Shutting down socket");
  shutdown(sock, 0);
  close(sock);
}