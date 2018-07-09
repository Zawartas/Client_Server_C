/** klient.h
 * 
 * Client side of TCP file transfer implementation, runs with custom server. 
 * Receives COMMANDs from input like list of products, or adding/changing/deleting
 * a product. 
 * 
 * Valid COMMANDs: 
 *    list (list of products)
 *    quit
 *    poka (show)
 *    zmin (change)
 *    doda (add)
 *    usun (delete)
 *    
 * Usage: 
 *    ./k.exe SERVER_HOSTNAME PORT#
 */

#ifndef FTCLIENT_H
#define FTCLIENT_H

#include "biblioteka.h"


/**
 * Receive a response from server
 * Returns -1 on error, return code on success.
 */
int read_reply();


/**
 * Print response message.
 */
void print_reply(int rc);


/**
 * Parse COMMAND in cstruct
 */ 
int ftclient_read_COMMAND(char* buf, int size, struct COMMAND *cstruct);


/**
 * Open data connection.
 */
int ftclient_open_conn(int sock_con);


/** 
 * Receive data from list COMMAND.
 */
int ftclient_list(int sock_data, int sock_con);

/** 
 * Receive data from server after a COMMAND has been processed.
 */
int ftclient_odbierz_info_po_komendzie(int sock_data, int sock_con);

/**
 * Input: cmd struct with an a code and an arg
 * Concats code + arg into a string and sends to server.
 */
int ftclient_send_cmd(struct COMMAND *cmd);


/**
 * Function that allows to write password - hidden.
 */
char *get_pass();


/**
 * Get login details from user and
 * send to server for authentication
 */
void ftclient_login();


#endif

