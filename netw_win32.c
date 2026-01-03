#include "include/netw.h"

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

/* global variables */
static int timeout_time = 1000;   // milliseconds
static SOCKET client_fd = INVALID_SOCKET;

/* shows an error and quits */
static void ExitError(const char* errMessage)
{
    fprintf(stderr, "\n%s\n", errMessage);
    WSACleanup();
    exit(-1);
}

/* sets a new timeout time in milliseconds */
void setTimeoutTime(int t)
{
    timeout_time = t;
}

/* sends a packet */
void netw_send(const uint8_t* buffer, int length)
{
    int sent = send(client_fd, (const char*)buffer, length, 0);
    if (sent == SOCKET_ERROR) {
        fprintf(stderr, "netw_send failed: %d\n", WSAGetLastError());
    }
}

/* Waits for a response from the server and reads the packet */
int netw_recv(uint8_t* buffer, int buffer_size)
{
    fd_set readfds;
    struct timeval tv;
    int ret;

    FD_ZERO(&readfds);
    FD_SET(client_fd, &readfds);

    tv.tv_sec  = timeout_time / 1000;
    tv.tv_usec = (timeout_time % 1000) * 1000;

    ret = select(0, &readfds, NULL, NULL, &tv);

    /* timeout */
    if (ret == 0) {
        return NETW_ERR_TIMEOUT;
    }

    /* error */
    if (ret == SOCKET_ERROR) {
        fprintf(stderr, "select failed: %d\n", WSAGetLastError());
        return NETW_ERR_TIMEOUT;
    }

    /* socket ready */
    ret = recv(client_fd, (char*)buffer, buffer_size, 0);
    if (ret == SOCKET_ERROR) {
        fprintf(stderr, "recv failed: %d\n", WSAGetLastError());
        return -1;
    }

    return ret;
}

/* tries to distinguish an IP address from a domain name */
bool netw_isValidIpAddress(char* ipAddress)
{
    struct sockaddr_in sa;
    return InetPtonA(AF_INET, ipAddress, &sa.sin_addr) == 1;
}

/* finds an IP address from a given domain name */
bool netw_getIpAddress(char* ip, char* hostname)
{
    struct addrinfo hints;
    struct addrinfo* result = NULL;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(hostname, NULL, &hints, &result) != 0) {
        return false;
    }

    struct sockaddr_in* addr = (struct sockaddr_in*)result->ai_addr;
    InetNtopA(AF_INET, &addr->sin_addr, ip, 16);

    freeaddrinfo(result);
    return true;
}

/* Creates a socket and tries to connect to the server with TCP or UDP */
void netw_connect(char* host, int port, bool useTCP)
{
    WSADATA wsa;
    struct sockaddr_in serv_addr;
    char ip[16];
    int sockType = useTCP ? SOCK_STREAM : SOCK_DGRAM;

    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        ExitError("WSAStartup failed");
    }

    if (netw_isValidIpAddress(host)) {
        strncpy(ip, host, sizeof(ip)-1);
        ip[sizeof(ip)-1] = '\0';
    } else {
        if (!netw_getIpAddress(ip, host)) {
            ExitError("Could not resolve hostname");
        }
    }

    client_fd = socket(AF_INET, sockType, 0);
    if (client_fd == INVALID_SOCKET) {
        ExitError("Socket creation error");
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port   = htons((u_short)port);

    if (InetPtonA(AF_INET, ip, &serv_addr.sin_addr) != 1) {
        ExitError("Invalid IP address");
    }

    if (connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
        ExitError("Connection failed");
    }
}

/* disconnect from the server */
void netw_disconnect()
{
    if (client_fd != INVALID_SOCKET) {
        closesocket(client_fd);
        client_fd = INVALID_SOCKET;
    }
    WSACleanup();
}

#endif /* _WIN32 */
