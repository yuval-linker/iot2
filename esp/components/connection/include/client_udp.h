#pragma once

#include "lwip/sockets.h"
#include "protocols.h"

int udp_client_connect();
void udp_client_send(int sock, int host_ip, int port, unsigned char* payload, int payload_size, char status_id, char protocol_id);
void udp_socket_close(int sock);