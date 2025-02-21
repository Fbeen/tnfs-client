#include "include/netw.h"

/* global variables */
int	timeout_time = 1000;	// the time that the we would like to wait on a respond from the server in milliseconds
int     client_fd;		// our file descriptor of the tcp or udp socket
struct  pollfd pfds[1];		// our poll file descriptor structure, used to determine if received data from the server

/* shows an error and quits */
void ExitError(char* errMessage)
{
    printf("\n%s\n", errMessage);
    exit(-1);
}

/* sets a new timeout time in milliseconds */
void setTimeoutTime(int t)
{
    timeout_time = t;
}

/* sends a package */
void netw_send(char* buffer, int length)
{
    if(send(client_fd, buffer, length, 0) == -1) {
    	perror("netw_send");
    }
}

/* Waits for a response from the server and reads the package */
int netw_receive(char* buffer, int buffer_size)
{
    int rpoll;
    int length;
    
    /* now wait for a response */
    rpoll = poll(pfds, 1, timeout_time);
    
    /* timeout */
    if(rpoll == 0) {
	// ExitError("Server timeout!");
	return NETW_ERR_TIMEOUT;
    }
    
    /* in case of an error or in case the server has closed the connection */
    if(rpoll > 0 && pfds[0].revents & (POLLERR | POLLHUP)) {
	ExitError("Socket was closed, server may be down!");
    }
    
    /* read the received data into the buffer */
    length = read(client_fd, buffer, buffer_size);
    
    if(length == -1) {
    	perror("netw_receive");
    }

    return length;
}

/* tries to distinguish an IP address from a domain name */
bool netw_isValidIpAddress(char *ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    return result != 0;
}

/* finds an IP address from a given domain name */
bool netw_getIpAddress(char* ip, char* hostname)
{
    struct hostent *hp = gethostbyname(hostname);

    if (hp == NULL)
       return false;
       
    unsigned int i=0;
    while ( hp -> h_addr_list[i] != NULL) {
        sprintf(ip, "%s ", inet_ntoa( *( struct in_addr*)( hp -> h_addr_list[i])));
        i++;
    }
    ip[strlen(ip)-1] = 0;
    
    return true;
}

/* Creates a socket and tries to connect to the server with TCP or UDP */
void netw_connect(char* host, int port, bool useTCP)
{
    char ip[16];
    int sockType = SOCK_STREAM;
    struct sockaddr_in serv_addr;
        
    if(netw_isValidIpAddress(host)) {
        strcpy(ip, host);
    } else {
    	if(!netw_getIpAddress(ip, host)) {
    	    ExitError("Could not find the IP address for given Hostname");
    	}
    }
    
    if(!useTCP) {
    	sockType = SOCK_DGRAM;
    }
    
    if ((client_fd = socket(AF_INET, sockType, 0)) < 0) {
        ExitError("Socket creation error");
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
        ExitError("Invalid address/ Address not supported");
    }

    if ((connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0) {
        ExitError("Connection Failed");
    }
    
    /* Initialize polling data that we will use later in the netw_receive function */ 
    pfds[0].events = POLLIN;
    pfds[0].fd = client_fd;
}

/* disconnect from the server */
void netw_disconnect()
{
    close(client_fd);
}

