#include "klient.h"
	

int sock_control;


int read_reply(){
	int retcode = 0;
    /*The recv(), recvfrom(), and recvmsg() calls are used to receive
       messages from a socket.  They may be used to receive data on both
       connectionless and connection-oriented sockets.  This page first
       describes common features of all three system calls, and then
       describes the differences between the calls.*/
    /*sock_control is a global variable*/
	if (recv(sock_control, &retcode, sizeof retcode, 0) < 0) {
		perror("client: error reading message from server\n");
		return -1;
	}
	return ntohl(retcode);
}


void print_reply(int rc) 
{
	switch (rc) {
		case 220:
			printf("220 Welcome, server ready.\n");
			break;
		case 221:
			printf("221 Goodbye!\n");
			break;
		case 226:
			printf("226 Closing data connection. Requested action successful.\n");
			break;
	}
	
}


int ftclient_read_COMMAND(char* buf, int size, struct COMMAND *cstruct)
{
	/* cleaning structure for COMMAND with arguments */
    memset(cstruct->code, 0, sizeof(cstruct->code));
    for (int i = 0; i < 3; i++) 
    {
        memset(cstruct->arg[i], 0, sizeof(cstruct->arg[i]));
    }
    
	printf("server> ");	// prompt for input		
	fflush(stdout); 	
    
	// wait for user to enter a COMMAND
    read_input(buf, size);

    
    //COMMAND
	char *argument = NULL;
	argument = strtok (buf," ");
    strncpy(cstruct->code, argument, sizeof(cstruct->code));
    
    //arguments
    int there_are_arguments = 0, i = 0;
    
    while ((argument = strtok (NULL, " ")) != NULL)
    {
        there_are_arguments = 1;
        strncpy(cstruct->arg[i], argument, strlen(argument));
        i++;
    }
    
    
	// buf = COMMAND
	if (strcmp(cstruct->code, "list") == 0) {
		strcpy(cstruct->code, "LIST");		
	}
	else if (strcmp(cstruct->code, "get") == 0) {
		strcpy(cstruct->code, "RETR");		
	}
	else if (strcmp(cstruct->code, "quit") == 0) {
		strcpy(cstruct->code, "QUIT");		
	}
	else if (strcmp(cstruct->code, "poka") == 0) {
		strcpy(cstruct->code, "POKA");		
	}
	else if (strcmp(cstruct->code, "doda") == 0) {
		strcpy(cstruct->code, "ADD_");		
	}
	else if (strcmp(cstruct->code, "usun") == 0) {
		strcpy(cstruct->code, "DEL_");
    }
	else if (strcmp(cstruct->code, "zmin") == 0) {
		strcpy(cstruct->code, "CHNG");
    }
	else {//invalid
		return -1;
	}

	// store code in beginning of buffer
	memset(buf, 0, 400);
	strcpy(buf, cstruct->code);

	// if there's an arg, append it to the buffer
    if (there_are_arguments)
    {
        for (int j = 0; j < i; j++) 
        {
            strcat(buf, " ");
            strncat(buf, cstruct->arg[j], sizeof(cstruct->arg[j]));
        }
    }
	return 0;
}


int ftclient_open_conn(int sock_con)
{
	int sock_listen = socket_create(CLIENT_PORT_ID);

	// send an ACK on control conn
	int ack = 1;
	if ((send(sock_con, (char*) &ack, sizeof(ack), 0)) < 0) {
		printf("client: ack write error :%d\n", errno);
		exit(1);
	}		

	int sock_conn = socket_accept(sock_listen);
	close(sock_listen);
	return sock_conn;
}


int ftclient_odbierz_info_po_komendzie(int sock_data, int sock_con)
{
	size_t num_recvd;			// number of bytes received with recv()
	char buf[MAXSIZE];			// hold a message received from server
	int tmp = 0;

	// Wait for server starting message
	if (recv(sock_con, &tmp, sizeof tmp, 0) < 0) {
		perror("client: error reading message from server\n");
		return -1;
	}
	
	memset(buf, 0, sizeof(buf));
	while ((num_recvd = recv(sock_data, buf, MAXSIZE, 0)) > 0) 
    {
        printf("%s", buf);
		memset(buf, 0, sizeof(buf));
	}
	
	if (num_recvd < 0) 
    {
	        perror("error");
	}

	// Wait for server done message
	if (recv(sock_con, &tmp, sizeof tmp, 0) < 0) {
		perror("client: error reading message from server\n");
		return -1;
	}
	return 0;
}


int ftclient_send_cmd(struct COMMAND *cmd)
{
	char buffer[MAXSIZE];
	int rc;

    sprintf(buffer, "%s %s", cmd->code, cmd->arg[0]);
    
	// Send COMMAND string to server
	rc = send(sock_control, buffer, (int)strlen(buffer), 0);	
	if (rc < 0) {
		perror("Error sending COMMAND to server");
		return -1;
	}
	
	return 0;
}


char *get_pass(char *sign) {
    
    printf(sign);
	static char *buf = NULL;
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);

	struct termios term;
	tcgetattr(1, &term); /*reads parameters from 1st terminal and saves them in term*/
	term.c_lflag &= ~ECHO; /*we aren't echoing characters*/
	tcsetattr(1, TCSANOW, &term); /*sets new parameters to terminal 1, from term, and TCSANOW measn immediately*/

	int c, len = 256, pos = 0;
	buf = realloc(buf, len);
	buf[0] = '\0';
	while ((c=fgetc(stdin)) != '\n') {
		buf[pos++] = (char) c;
		if (pos >= len)
			buf = realloc(buf, (len += 256));
	}
	buf[pos] = '\0';

	term.c_lflag |= ECHO; /*we give back ECHOing characters to terminal*/
	tcsetattr(1, TCSANOW, &term); /* and we set them back*/
    
    signal(SIGINT, SIG_DFL);
	signal(SIGTERM, SIG_DFL);
	return buf;
}


void ftclient_login()
{
	struct COMMAND cmd;
	char user[255];
	memset(user, 0, 255);

	// Get username from user
	printf("Name: ");	
	fflush(stdout); 		
	read_input(user, 255);

	// Send USER COMMAND to server
	strcpy(cmd.code, "USER");
    strncpy(cmd.arg[0], user, sizeof(cmd.arg[0]));

	ftclient_send_cmd(&cmd);
	
	// Wait for go-ahead to send password
	int wait;
	recv(sock_control, &wait, sizeof wait, 0);

	// Get password from user
	fflush(stdout);	
	char *pass = get_pass("Password: ");	

	// Send PASS COMMAND to server
	strcpy(cmd.code, "PASS");
	strcpy(cmd.arg[0], pass);
	ftclient_send_cmd(&cmd);
	
	// wait for response
	int retcode = read_reply();

	switch (retcode) {
		case 430:
			printf("\nInvalid username/password.\n");
			exit(0);
		case 230:
			printf("\nSuccessful USER login.\n");
			break;
		case 330:
			printf("\nSuccessful ADMIN login.\n");
			break;
		default:
			perror("error reading message from server");
			exit(1);		
			break;
	}
}


int main(int argc, char* argv[]) 
{		
	int data_sock, retcode, s;
	char buffer[MAXSIZE];
	struct COMMAND cmd;	
	struct addrinfo hints, *res, *rp;

	if (argc != 3) {
		printf("usage: %s [hostname] [port]\n", argv[0]);
		exit(0);
	}

	char *host = argv[1]; /*!< holds host name/value. */ 
	char *port = argv[2]; /*!< Holds port value. */ 

	// Get matching addresses
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	
    /* The getaddrinfo() function allocates and initializes a linked list of addrinfo structures, 
        one for each network address that matches node and service, subject to any restrictions imposed by hints, 
        and returns a pointer to the start of the list in res. 
        The items in the linked list are linked by the ai_next field. */
    
    /* getaddrinfo() returns a list of address structures.
        Try each address until we successfully bind(2).
        If socket(2) (or bind(2)) fails, we (close the socket
        and) try the next address. */
    
    /*There are several reasons why the linked list may have more than one
        addrinfo structure, including: the network host is multihomed, acces‐
        sible over multiple protocols (e.g., both AF_INET and AF_INET6); or
        the same service is available from multiple socket types (one
        SOCK_STREAM address and another SOCK_DGRAM address, for example).
        Normally, the application should try using the addresses in the order
        in which they are returned.  The sorting function used within getad‐
        drinfo() is defined in RFC 3484; the order can be tweaked for a par‐
        ticular system by editing /etc/gai.conf (available since glibc 2.5).*/
    
	s = getaddrinfo(host, port, &hints, &res);
	if (s != 0) { 
        /*The gai_strerror() function translates these error codes 
        to a human readable string, suitable for error reporting.*/
		printf("getaddrinfo() error %s", gai_strerror(s));
		exit(1);
	}
	
	// Find an address to connect to & connect
	for (rp = res; rp != NULL; rp = rp->ai_next) 
    {
        /*sock_control ia a integer var set at the beginning of code*/
		sock_control = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

		if (sock_control < 0)
			continue;

		if(connect(sock_control, res->ai_addr, res->ai_addrlen)==0) {
			break;
		} else {
			perror("connecting stream socket");
			exit(1);
		}
		close(sock_control);
	}
	/*The freeaddrinfo() function frees the memory that was allocated for
       the dynamically allocated linked list res.*/
	freeaddrinfo(rp);


	// Get connection, welcome messages
	printf("Connected to %s.\n", host);
	print_reply(read_reply()); 
	

	/* Get name and password and send to server */
	ftclient_login();

	while (1) { // loop until user types quit

		// Get a COMMAND from user
        memset(buffer, 0, sizeof(buffer));
		if ( ftclient_read_COMMAND(buffer, sizeof(buffer), &cmd) < 0) 
        {
			printf("Invalid COMMAND\n");
			continue;	// loop back for another COMMAND
		}

		// Send COMMAND to server
		if (send(sock_control, buffer, (int)strlen(buffer), 0) < 0 ) 
        {
			close(sock_control);
			exit(1);
		}
		
		retcode = read_reply();	
		if (retcode == 221) {
			/* If COMMAND was quit, just exit */
			print_reply(221);		
			break;
		}
		
		if (retcode == 502) 
        	{
			// If invalid COMMAND, show error message
			printf("%d Invalid COMMAND.\n", retcode);
		}
		else 
        	{			
			// Command is valid (RC = 200), process COMMAND
		
			// open data connection
			if ((data_sock = ftclient_open_conn(sock_control)) < 0) {
				perror("Error opening socket for data connection");
				exit(1);
			}			
			
			// execute COMMAND and receive DATA back
			if (strcmp(cmd.code, "LIST") == 0 || strcmp(cmd.code, "POKA") == 0
                || strcmp(cmd.code, "ADD_") == 0 || strcmp(cmd.code, "DEL_") == 0
                || strcmp(cmd.code, "CHNG") == 0) 
            {
				ftclient_odbierz_info_po_komendzie(data_sock, sock_control);
                print_reply(226);
			}
			
			close(data_sock);
		}
	} // loop back to get more user input
    
	// Close the socket (control connection)
	close(sock_control);
    return 0;  
}

