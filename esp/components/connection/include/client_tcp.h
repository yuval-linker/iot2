#pragma once

#include "protocols.h"

#define TCP_RECV_BUFFER_SIZE 1024

int tcp_connect(unsigned char* host_ip, int port);
void tcp_send(int sock, unsigned char* payload, int payload_size, char status_id, char protocol_id);
void tcp_socket_close(int sock);
int tcp_recv(int sock, char* buffer, int buffer_size);