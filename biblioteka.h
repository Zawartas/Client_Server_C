#ifndef COMMON_H
#define COMMON_H

#include <arpa/inet.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>		// getaddrinfo()
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <termios.h>
#include <limits.h>
#include <math.h>



/* constants */
#define DEBUG				1
#define MAXSIZE 			1024 	// max buffer size
#define CLIENT_PORT_ID		30020
#define MAX_BUF 50


/**
 * Holds COMMAND code and arguments.
 */
struct COMMAND {
    char code[5];
	char arg[3][255];
};

/**
 * Holds data for a single product.
 */
struct TOWAR {
    long int kod_kreskowy; /**< product bar code */
    char nazwa[MAXSIZE]; /**< product name */
    double cena; /**< product price */
};


/**
 * Create listening socket on remote host
 * Returns -1 on error, socket fd on success.
 */
int socket_create(int port);


/**
 * Create new socket for incoming client connection request
 * Returns -1 on error, or fd of newly created socket
 */
int socket_accept(int sock_listen);


/**
 * Connect to remote host at given port
 * Returns socket fd on success, -1 on error.
 */
int socket_connect(int port, char *host);



/**
 * Receive data on sockfd
 * Returns -1 on error, number of bytes received 
 * on success.
 */
int recv_data(int sockfd, char* buf, int bufsize);


/**
 * Send resposne code on sockfd
 * Returns -1 on error, 0 on success.
 */
int send_response(int sockfd, int rc);

/** 
 * Read input from COMMAND line.
 */
void read_input(char* buffer, int size);


#endif








