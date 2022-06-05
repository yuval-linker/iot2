#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include "payload_sizes.h"
#include "client_udp.h"


static const char *UDP_TAG = "ESP32 UDP";

socket_connection_t udp_connect(int host_ip, int port) {
  socket_connection_t conn;
  int addr_family = AF_INET;
  int ip_protocol = IPPROTO_IP;
  struct sockaddr_in dest_addr;
  dest_addr.sin_addr.s_addr = inet_addr(host_ip);
  dest_addr.sin_family = AF_INET;
  dest_addr.sin_port = htons(port);
  int sock = socket(0, SOCK_DGRAM, ip_protocol);
  if (sock < 0) {
    ESP_LOGE(UDP_TAG, "Unable to create socket: errno %d", errno);
    return;
  }
  ESP_LOGI(UDP_TAG, "Socket created, sending to %s:%d", host_ip, port);
  conn.socket = sock;
  conn.dest_addr = dest_addr;
  return conn;
}

void udp_send(socket_connection_t* conn, unsigned char* payload, int payload_size, char protocol_id) {
  bzero(payload, PAYLOAD_4_SIZE);
  send_payload((unsigned char *) payload, (char) protocol_id, (unsigned short) ID_DEVICE, MAC_DEVICE);
  int err = sendto(conn->socket, payload, (size_t) payload_size, 0, (struct sockaddr *)&(conn->dest_addr), sizeof(conn->dest_addr));
  if (err < 0) {
    ESP_LOGE(UDP_TAG, "Error occurred during sending: errno %d", errno);
    return;
  }
  ESP_LOGI(UDP_TAG, "Message sent");
}


void udp_socket_close(int sock) {
  shutdown(sock, 0);
  close(sock);
  ESP_LOGI(UDP_TAG, "Socket closed");
}
