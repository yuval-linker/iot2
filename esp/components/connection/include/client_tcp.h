#pragma once

#include "protocols.h"

#define TCP_RECV_BUFFER_SIZE 1024

int tcp_client_connect(int host_ip, int port);
void tcp_client_send(int sock, unsigned char* payload, int payload_size, char status_id, char protocol_id);
void tcp_socket_close(int sock);
int tcp_client_recv(int sock, char* buffer, int buffer_size);
int tcp_server_start(int port);
int tcp_server_accept(int listen_sokcet);