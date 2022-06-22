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


/** @brief Initialize the TCP connection and creates a socket for the communication
    * @param host_ip The IPv4 address (as an int) of the HOST the device is connecting to 
    * @param port The TCP PORT of the HOST the device is connecting to

    * @return The socket if succesfull. A negative number if an error ocurred
**/
int tcp_client_connect(int host_ip, int port) {
  int addr_family = AF_INET;
  int ip_protocol = IPPROTO_IP;
  struct sockaddr_in dest_addr;
  dest_addr.sin_addr.s_addr = host_ip;
  dest_addr.sin_family = AF_INET;
  dest_addr.sin_port = htons(port);
  int sock =  socket(addr_family, SOCK_STREAM, ip_protocol);   // Create a socket for the communication
  if (sock < 0) {
      ESP_LOGE(TCP_TAG, "Unable to create socket: errno %d", errno);
      return -1;
  }

  // Need to set REUSEADDR to True to connect again to the same IP when disconnected
  BaseType_t xTrueValue = pdTRUE;
  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void *) &xTrueValue, sizeof(xTrueValue));
  ESP_LOGI(TCP_TAG, "Socket created, connecting to %s", inet_ntoa(dest_addr));

  // Connect to the HOST
  int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_in6));
  if (err != 0) {
      ESP_LOGE(TCP_TAG, "Socket unable to connect: errno %d", errno);
      return -1;
  }
  ESP_LOGI(TCP_TAG, "Successfully connected");
  return sock;
}

/** @brief Initialize the TCP connection as a server and start listening for connections
    * @param port The TCP PORT to be opened for clients to connecto to

    * @return The listening socket if succesfull. A negative number if an error ocurred
**/
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

  // Create a socket to listen for connections
  int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
  if (listen_sock < 0) {
      ESP_LOGE(TCP_TAG, "Unable to create socket: errno %d", errno);
      return listen_sock;
  }

  // Need to set REUSEADDR to True to connect again to the same IP when disconnected
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

  // Start listening for connections
  err = listen(listen_sock, 1);
  if (err != 0) {
      ESP_LOGE(TCP_TAG, "Error occurred during listen: errno %d", errno);
      tcp_socket_close(listen_sock);
      return -1;
  }

  return listen_sock;
}

/** @brief Use the listening socket to accept a new connection
    * @param listen_sock The listening socket

    * @return The socket for the communication if succesfull. A negative number if an error ocurred
**/
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

/** @brief Send a message using TCP connection
    * @param sock The socket with the connection
    * @param payload A buffer to populate with the message
    * @param payload_size The buffer size
    * @param status_id The status ID of the communication
    * @param protocol_id The protocol ID of the message
**/
void tcp_client_send(int sock, unsigned char* payload, int payload_size, char status_id, char protocol_id) {
  bzero(payload, payload_size); // Reset the buffer to only zeros
  send_payload(payload, status_id, protocol_id, (unsigned short) ID_DEVICE, MAC_DEVICE);
  int err = send(sock, payload, payload_size, 0);
  if (err < 0) {
      ESP_LOGE(TCP_TAG, "Error occurred during sending: errno %d", errno);
      return;
  }
  ESP_LOGI(TCP_TAG, "Message sent");
}

/** @brief Receive a message using a TCP Connection
    * @param sock The socket with the connection
    * @param buffer A buffer to populate with the recieved message
    * @param buffer_size The buffer size
**/
int tcp_client_recv(int sock, char* buffer, int buffer_size) {
  int len = recv(sock, buffer, buffer_size, 0);
  // Error occurred during receiving
  if (len < 0) {
      ESP_LOGE(TCP_TAG, "recv failed: errno %d", errno);
      return -1;
  }
  return len;
}

/** @brief Close the TCP Socket and free that port
    * @param sock The socket to be closed
**/
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