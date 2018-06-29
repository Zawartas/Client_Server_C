#ifndef FTSERVE_H
#define FTSERVE_H

#include "common.h"

/**
 * Open database file
 * and fill up tab_TOWAR table of TOWAR structures
 * and fills up database size.
 */
int open_database();

/**
 * Send list of products.
 */
int ftserve_list(int sock_data, int sock_control);

/**
 * Save database to a file.
 */
void save_database();

/**
 * Searching for a product by bar code.
    \param sock_data is a connection socket.
    \param sock_control is an accepted socket for connection.
    \return returns 0 after process is finished
 */
int ftserve_pokaz_towar(int sock_data, int sock_control, long int szukane_ID);

/**
 * Deleting product by a bar code.
 */
int ftserve_skasuj_towar(int sock_data, int sock_control, long int szukane_ID);

/**
 * Change product by a bar code.
 */
int ftserve_zmien_towar(int sock_data, int sock_control, long int szukane_ID, char *nowa_nazwa, double nowa_cena);

/**
 * Adding a product by a bar code.
 */
int ftserve_dodaj_towar(int sock_data, int sock_control, long int szukane_ID, char *nazwa, double price);

/**
 * Function that sends information about lack of admin_only accessed functions.
 */
int ftserve_brak_uprawnien(int sock_data, int sock_control);

/**
 * Open data connection to client
 * Returns: socket for data connection
 * or -1 on error.
 */
int ftserve_start_data_conn(int sock_control);



/**
 * Authenticate a user's credentials
 * Return 1 if authenticated, 0 if not.
 */
int ftserve_check_user(char*user, char*pass);



/**
 * Log in connected client.
 */
int ftserve_login(int sock_control);


/**
 * Wait for command from client and send response
 * Returns response code.
 */
int ftserve_recv_cmd(int sock_control, char*cmd, char*arg);



/**
 * Child process handles connection to client.
 */
void ftserve_process(int sock_control);


#endif
