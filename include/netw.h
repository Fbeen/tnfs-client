#ifndef __netw_h__
#define __netw_h__

#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <stdbool.h>

#define NETW_ERR_TIMEOUT -2	// timeout error code when waiting on a response from the server

/* function prototypes */
void setTimeoutTime(int t);
void netw_send(char* buffer, int length);
int  netw_receive(char* buffer, int buffer_size);
void netw_exitError(char* errMessage);
bool netw_isValidIpAddress(char *ipAddress);
bool netw_getIpAddress(char* ip, char* hostname);
void netw_connect(char* host, int port, bool useTCP);
void netw_disconnect();

#endif

