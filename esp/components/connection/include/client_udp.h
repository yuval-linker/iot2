#pragma once

#include "lwip/sockets.h"
#include "protocols.h"

typedef struct socket_connection_t {
  int socket;
  struct sockaddr_in dest_addr;
} socket_connection_t;

socket_connection_t udp_connect(int host_ip, int port);
void udp_send(socket_connection_t* conn, unsigned char* payload, int payload_size, char protocol_id);
void udp_socket_close(int sock);