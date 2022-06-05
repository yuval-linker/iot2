#pragma once

#include "protocols.h"

int tcp_connect(int host_ip, int port);
void tcp_send(int sock, unsigned char* payload, int payload_size, char protocol_id);
void tcp_socket_close(int sock);