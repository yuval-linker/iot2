#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include "payload_sizes.h"
#include "send_payload.h"
#include "client_udp.h"

static const char *UDP_TAG = "ESP32 UDP";

/** @brief Initialize the UDP connection and creates a socket for the communication
    * @return The socket if succesfull, a negative number if an error ocurred
**/
int udp_client_connect() {
  int ip_protocol = IPPROTO_IP;
  int sock = socket(AF_INET, SOCK_DGRAM, ip_protocol);
  if (sock < 0) {
    ESP_LOGE(UDP_TAG, "Unable to create socket: errno %d", errno);
    return sock;
  }
  ESP_LOGI(UDP_TAG, "Socket created, %d", sock);
  return sock;
}

/** @brief Send a message using UDP connection
    * @param sock The socket with the connection
    * @param host_ip The IPv4 address (as an int) of the HOST the device is connecting to 
    * @param port The UDP PORT of the HOST the device is connecting to
    * @param payload A buffer to populate with the message
    * @param payload_size The buffer size
    * @param status_id The status ID of the communication
    * @param protocol_id The protocol ID of the message
**/
void udp_client_send(int sock, int host_ip, int port, unsigned char* payload, int payload_size, char status_id, char protocol_id) {
  struct sockaddr_in dest_addr;
  dest_addr.sin_addr.s_addr = host_ip;
  dest_addr.sin_family = AF_INET;
  dest_addr.sin_port = htons(port);
  bzero(payload, payload_size);
  send_payload(payload, status_id, protocol_id, (unsigned short) ID_DEVICE, MAC_DEVICE);
  int err = sendto(sock, payload, (size_t) payload_size, 0, (struct sockaddr *)&(dest_addr), sizeof(dest_addr));
  if (err < 0) {
    ESP_LOGE(UDP_TAG, "Error occurred during sending: errno %d", errno);
    return;
  }
  ESP_LOGI(UDP_TAG, "Message sent to %d %d", host_ip, port);
}

/** @brief Close the UDP Socket and free that port
    * @param sock The socket to be closed
**/
void udp_socket_close(int sock) {
  shutdown(sock, 0);
  close(sock);
  ESP_LOGI(UDP_TAG, "Socket closed");
}
