#include "esp_log.h"
#include "esp_netif.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include "send_payload.h"
#include "client_tcp.h"

static const char *TCP_TAG = "ESP32 TCP";

RTC_DATA_ATTR unsigned int payload_type = 0;

int tcp_client_connect(int host_ip, int port) {
  int addr_family = AF_INET;
  int ip_protocol = IPPROTO_IP;
  struct sockaddr_in dest_addr;
  dest_addr.sin_addr.s_addr = host_ip;
  dest_addr.sin_family = AF_INET;
  dest_addr.sin_port = htons(port);
  int sock =  socket(addr_family, SOCK_STREAM, ip_protocol);
  if (sock < 0) {
      ESP_LOGE(TCP_TAG, "Unable to create socket: errno %d", errno);
      return -1;
  }
  BaseType_t xTrueValue = pdTRUE;
  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void *) &xTrueValue, sizeof(xTrueValue));
  ESP_LOGI(TCP_TAG, "Socket created, connecting to %s", inet_ntoa(dest_addr));

  int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_in6));
  if (err != 0) {
      ESP_LOGE(TCP_TAG, "Socket unable to connect: errno %d", errno);
      return -1;
  }
  ESP_LOGI(TCP_TAG, "Successfully connected");
  return sock;
}

int tcp_server_start(int port)
{
  int addr_family = AF_INET;
  int ip_protocol = 0;
  struct sockaddr_storage dest_addr;
  struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
  dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
  dest_addr_ip4->sin_family = AF_INET;
  dest_addr_ip4->sin_port = htons(port);
  ip_protocol = IPPROTO_IP;

  int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
  if (listen_sock < 0) {
      ESP_LOGE(TCP_TAG, "Unable to create socket: errno %d", errno);
      return listen_sock;
  }
  int opt = 1;
  setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  ESP_LOGI(TCP_TAG, "Socket created");

  int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
  if (err != 0) {
      ESP_LOGE(TCP_TAG, "Socket unable to bind: errno %d", errno);
      ESP_LOGE(TCP_TAG, "IPPROTO: %d", addr_family);
      tcp_socket_close(listen_sock);
      return -1;
  }
  ESP_LOGI(TCP_TAG, "Socket bound, port %d", port);

  err = listen(listen_sock, 1);
  if (err != 0) {
      ESP_LOGE(TCP_TAG, "Error occurred during listen: errno %d", errno);
      tcp_socket_close(listen_sock);
      return -1;
  }

  return listen_sock;
}


int tcp_server_accept(int listen_sock)
{
  int keepAlive = 1;
  int keepIdle = 20;
  int keepInterval = 20;
  int keepCount = 5;

  ESP_LOGI(TCP_TAG, "Socket listening");

  struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
  socklen_t addr_len = sizeof(source_addr);
  int sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
  if (sock < 0) {
      ESP_LOGE(TCP_TAG, "Unable to accept connection: errno %d", errno);
      return -1;
  }

  // Set tcp keepalive option
  setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));
  setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
  setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int));
  setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));

  return sock;
}

void tcp_client_send(int sock, unsigned char* payload, int payload_size, char status_id, char protocol_id) {
  bzero(payload, payload_size);
  send_payload(payload, status_id, protocol_id, (unsigned short) ID_DEVICE, MAC_DEVICE);
  int err = send(sock, payload, payload_size, 0);
  if (err < 0) {
      ESP_LOGE(TCP_TAG, "Error occurred during sending: errno %d", errno);
      return;
  }
  ESP_LOGI(TCP_TAG, "Message sent");
}

int tcp_client_recv(int sock, char* buffer, int buffer_size) {
  int len = recv(sock, buffer, buffer_size, 0);
  // Error occurred during receiving
  if (len < 0) {
      ESP_LOGE(TCP_TAG, "recv failed: errno %d", errno);
      return -1;
  }
  return len;
}

void tcp_socket_close(int sock) {
  int err;
  ESP_LOGI(TCP_TAG, "Shutting down socket");
  err = shutdown(sock, 0);
  if (err < 0) {
    ESP_LOGE(TCP_TAG, "Error occurred during shutting down: errno %d", errno);
  }
  err = close(sock);
  if (err < 0) {
    ESP_LOGE(TCP_TAG, "Error occurred during shutting down: errno %d", errno);
  }
}