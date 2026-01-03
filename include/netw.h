#ifndef __netw_h__
#define __netw_h__

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#define NETW_ERR_TIMEOUT -2   // timeout error code

#ifdef _WIN32

/* =========================
 * Windows / Winsock
 * ========================= */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#else

/* =========================
 * POSIX / Linux
 * ========================= */
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <poll.h>

#endif

/* function prototypes */
void setTimeoutTime(int t);
void netw_send(const uint8_t* buffer, int length);
int  netw_recv(uint8_t* buffer, int buffer_size);
bool netw_isValidIpAddress(char* ipAddress);
bool netw_getIpAddress(char* ip, char* hostname);
void netw_connect(char* host, int port, bool useTCP);
void netw_disconnect();

#endif /* __netw_h__ */
