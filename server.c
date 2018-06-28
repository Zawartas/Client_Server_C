#include "server.h"

struct TOWAR tab_TOWAR[MAX_BUF];
int database_size;
int admin;

/**
 * \function
 * Main function
 * 
 */

int main(int argc, char *argv[])
{	
	int sock_listen, sock_control, port, pid;

	if (argc != 2) {
		printf("usage: %s port\n", argv[0]);
		exit(0);
	}

	port = atoi(argv[1]);

	// create socket
	if ((sock_listen = socket_create(port)) < 0 ) {
		perror("Error creating socket");
		exit(1);
	}		
	
	while(1) {	// wait for client request

		// create new socket for control connection
		if ((sock_control = socket_accept(sock_listen))	< 0 )
			break;			
		
		// create child process to do actual file transfer
		if ((pid = fork()) < 0) { 
			perror("Error forking child process");
		} else if (pid == 0) { 
			close(sock_listen);
			ftserve_process(sock_control);		
			close(sock_control);
			exit(0);
		}
			
		close(sock_control);
	}

	close(sock_listen);	

	return 0;
}

/**
 * \function
 * Open database file
 * and fill up tab_TOWAR table of TOWAR structures
 * and fills up database size
 * 
 */
int open_database()
{
	FILE * fp;
    	char * line = NULL;
    	char * token = NULL;
    	size_t len = 0;
    	ssize_t read;
    	int i = 0;

    	fp = fopen(".baza", "r+");
    	if (fp == NULL)
        	exit(EXIT_FAILURE);

    	while ((read = getline(&line, &len, fp)) != -1) 
    	{
            printf("rekord o indeksie %d: ", i);
            if ((token = strtok(line, ",")) == NULL)
                        exit(EXIT_FAILURE);
                printf("%s ", token);
                sscanf(token, "%ld", &tab_TOWAR[i].kod_kreskowy);
                token = strtok (NULL, ",");
                printf("%s ", token);
                sscanf(token, "%s", tab_TOWAR[i].nazwa);
                token = strtok (NULL, ",");
                printf("%s", token);
                sscanf(token, "%lg", &tab_TOWAR[i].cena);
            i++;
    	}
    	
	database_size = i;

    printf("[+]Otworzono BAZE DANYCH o %d elementach\n", database_size);
	
    	fclose(fp);
    	if (line) free(line);
    	
    	return database_size;
}

/**
 * \function
 * Save database to a file
 * 
 */
void save_database()
{
	FILE * fp;

    fp = fopen(".baza", "wb");
    for (int j = 0; j < database_size; j++)
    {
        if (strcmp(tab_TOWAR[j].nazwa, "do_skasowania") == 0) continue;
        fprintf (fp, "%ld,%s,%.2f\n", tab_TOWAR[j].kod_kreskowy, tab_TOWAR[j].nazwa, tab_TOWAR[j].cena);
    }
    fclose(fp);
}


/**
 * \function
 * Send list of files in current directory
 * over data connection
 * Return -1 on error, 0 on success
 * 
 */
int ftserve_list(int sock_data, int sock_control)
{
	FILE * fp;
    char * line = NULL;
    char data[MAXSIZE];

    size_t len = 0, read;
    
    fp = fopen(".baza", "r+");
    if (!fp)
        exit(EXIT_FAILURE);
    
    memset(data, 0, MAXSIZE);
    send_response(sock_control, 1); //starting

    while ((read = getline(&line, &len, fp)) != -1) 
    {              
        sprintf(data, "%s", line);
        if (send(sock_data, data, strlen(data), 0) < 0) 
        {
			perror("err");
		}
    }
    fclose(fp);
    if (line) free(line);

	send_response(sock_control, 226);	// send 226

	return 0;	
}


/**
 * \function
 * Searching for a product by bar code
 * 
 */
int ftserve_pokaz_towar(int sock_data, int sock_control, long int szukane_ID)
{
	FILE * fp;
    char * line = NULL;
    char * token = NULL;
    /**
     * \var table reserved for data being prepared to send to Client
     */
    char data[MAXSIZE];

    size_t len = 0;
    ssize_t read;
    /**
     * \var variable informing whether product has been found 0 - no, 1 - yes
     */
    int znaleziono = 0;
    
    struct TOWAR baza;

    fp = fopen(".baza", "r+");
    if (!fp)
        exit(EXIT_FAILURE);
    else
    {
        printf("[+]Przeszukiwanie bazy danych\n");
        printf("[+]Poszukiwanie ID: %ld\n", szukane_ID);
    }
    

    while ((read = getline(&line, &len, fp)) != -1) 
    {
        if ((token = strtok(line, ",")) == NULL)
                    exit(EXIT_FAILURE);

            sscanf(token, "%ld", &baza.kod_kreskowy);
            token = strtok (NULL, ",");
    
            sscanf(token, "%s", baza.nazwa);
            token = strtok (NULL, ",");

            sscanf(token, "%lg", &baza.cena);
        
                if (baza.kod_kreskowy == szukane_ID)
                {
                    memset(data, 0, MAXSIZE);
                    
                    sprintf(data, "Znaleziono produkt: %ld %s %lg\n", baza.kod_kreskowy, baza.nazwa, baza.cena);
                    printf("Znaleziono na serwerze produkt: %s\n", data);
                    znaleziono = 1;
                    break;
                }
    }
	
	if (!znaleziono) 
    {
        printf("Nie znaleziono produktu o zadanym kodzie kreskowym\n");
        sprintf(data, "Nie znaleziono produktu o zadanym kodzie kreskowym\n");
    }
	
    fclose(fp);
    if (line) free(line);

	send_response(sock_control, 1); //starting

		if (send(sock_data, data, strlen(data), 0) < 0) 
        {
			perror("err");
		}

	fclose(fp);

	send_response(sock_control, 226);	// send 226

	return 0;	
}


/**
 * \function
 * Deleting product by a bar code
 * 
 */
int ftserve_skasuj_towar(int sock_data, int sock_control, long int szukane_ID)
{
	//int database_size;
    	char data[MAXSIZE];
    	int znaleziono = 0, i;
    	i = open_database();

        printf("[+]Poszukiwanie ID: %ld\n", szukane_ID);

	memset(data, 0, MAXSIZE);
    while (--i) 
    {   
        if (tab_TOWAR[i].kod_kreskowy == szukane_ID)
        {
            memset(data, 0, MAXSIZE);
            
            sprintf(data, "Skasowano produkt: %ld %s %lg\n", tab_TOWAR[i].kod_kreskowy, tab_TOWAR[i].nazwa, tab_TOWAR[i].cena);
            strcpy(tab_TOWAR[i].nazwa, "do_skasowania");
            printf("Skasowano produkt o ID: %ld\n", szukane_ID);
            znaleziono = 1;
            break;
        }
    }
	
	if(!znaleziono)
    {
        sprintf(data, "Nie znaleziono produktu o ID: %ld\n", tab_TOWAR[i].kod_kreskowy);
        printf("Znaleziono na serwerze produkt: %s\n", data);
    }
	
	send_response(sock_control, 1); //starting

    if (send(sock_data, data, strlen(data), 0) < 0) 
    {
        perror("err");
    }
		
    save_database();

	send_response(sock_control, 226);	// send 226 NIE WIEM PO CO TO

	return 0;	
}


/**
 * \function
 * Change product by a bar code
 * 
 */
int ftserve_zmien_towar(int sock_data, int sock_control, long int szukane_ID, char *nowa_nazwa, double nowa_cena)
{
	//int database_size;
    	char data[MAXSIZE];
    	int znaleziono = 0, i;
    	i = open_database();

        printf("[+]Poszukiwanie ID: %ld\n", szukane_ID);

	memset(data, 0, MAXSIZE);
    while (--i) 
    {   
        printf("[+]FUKAMY\n");
        if (tab_TOWAR[i].kod_kreskowy == szukane_ID)
        {
            printf("[+]MAMY!!!!\n");
           
            sprintf(data, "Odnaleziony produkt: %ld %s %lg zmieniamy na: %s %lg\n", 
                    tab_TOWAR[i].kod_kreskowy, tab_TOWAR[i].nazwa, tab_TOWAR[i].cena, nowa_nazwa, nowa_cena);
            strcpy(tab_TOWAR[i].nazwa, nowa_nazwa);
            tab_TOWAR[i].cena = nowa_cena;
            printf("Zmieniamy produkt o ID: %ld\n", szukane_ID);
            znaleziono = 1;
            break;
        }
        i--;
    }
	
	if(!znaleziono)
    {
        sprintf(data, "Nie znaleziono produktu o ID: %ld\n", tab_TOWAR[i].kod_kreskowy);
        printf("Nie znaleziono na serwerze produktu o ID: %ld\n", szukane_ID);
    }
	
	send_response(sock_control, 1); //starting

    if (send(sock_data, data, strlen(data), 0) < 0) 
    {
        perror("err");
    }
		
    save_database();

	send_response(sock_control, 266);	// send 226 NIE WIEM PO CO TO

	return 0;	
}


/**
 *  \function
 * Adding a product by a bar code
 * 
 */
int ftserve_dodaj_towar(int sock_data, int sock_control, long int szukane_ID, char *nazwa, double price)
{
	FILE * fp;
    char * line = NULL;
    char * token = NULL;
    char data[MAXSIZE];

    size_t len = 0;
    ssize_t read;
    int znaleziono = 0;
    
    struct TOWAR baza;

    fp = fopen(".baza", "r");
    if (!fp)
        exit(EXIT_FAILURE);
    else
    {
        printf("[+]Przeszukiwanie bazy danych\n");
        printf("[+]Poszukiwanie ID: %ld\n", szukane_ID);
    }
    
    while ((read = getline(&line, &len, fp)) != -1) 
    {
        if ((token = strtok(line, ",")) == NULL)
                    exit(EXIT_FAILURE);

            sscanf(token, "%ld", &baza.kod_kreskowy);
            token = strtok (NULL, ",");
    
            sscanf(token, "%s", baza.nazwa);
            token = strtok (NULL, ",");

            sscanf(token, "%lg", &baza.cena);
        
                if (baza.kod_kreskowy == szukane_ID)
                {
                    memset(data, 0, MAXSIZE);
                    
                    sprintf(data, "Znaleziono produkt: %ld %s %lg\nUzyj komendy 'zmie' w celu zmiany produktu lub go 'usun', a nastepnie dodaj nowy", 
                            baza.kod_kreskowy, baza.nazwa, baza.cena);
                    printf("Znaleziono na serwerze produkt: %s\n", data);
                    znaleziono = 1;
                    fclose(fp);
                    break;
                }
    }
	
	if (!znaleziono) 
    {
        fp = fopen(".baza", "a");
        int status;
        printf("Dodajemy produkt o zadanym kodzie kreskowym\n");
        sprintf(data, "Dodajemy produkt o zadanych danych do bazy\n");
        if ((status = fprintf(fp, "%ld,%s,%lg\n", szukane_ID, nazwa, price)) <= 0)
            printf("[-]Blad zapisu nowego produktu do bazy\n");
        fclose(fp);
    }
	
    
    if (line) free(line);

	send_response(sock_control, 1); //starting

		if (send(sock_data, data, strlen(data), 0) < 0) 
        {
			perror("err");
		}

	send_response(sock_control, 226);	// send 226

	return 0;	
}


/**
 *  \function
 * Adding a product by a bar code
 * 
 */
int ftserve_brak_uprawnien(int sock_data, int sock_control)
{
    char data[MAXSIZE];
    memset(data, 0, MAXSIZE);
                    
    sprintf(data, "Brak uprawnien uzytkownika\n");

	send_response(sock_control, 1); //starting

		if (send(sock_data, data, strlen(data), 0) < 0) 
        {
			perror("err");
		}

	send_response(sock_control, 666);	// send 226

	return 0;	
}

/**
 * \function
 * Open data connection to client 
 * Returns: socket for data connection
 * or -1 on error
 */
int ftserve_start_data_conn(int sock_control)
{
	char buf[1024];	   
	int wait, sock_data;

	// Wait for go-ahead on control conn
	if (recv(sock_control, &wait, sizeof wait, 0) < 0 ) {
		perror("Error while waiting");
		return -1;
	}

	// Get client address
	struct sockaddr_in client_addr;
	socklen_t len = sizeof client_addr;
	getpeername(sock_control, (struct sockaddr*)&client_addr, &len);
	inet_ntop(AF_INET, &client_addr.sin_addr, buf, sizeof(buf));

	// Initiate data connection with client
	if ((sock_data = socket_connect(CLIENT_PORT_ID, buf)) < 0)
		return -1;

	return sock_data;		
}


/**
 * \function
 * Authenticate a user's credentials
 * Return 1 if authenticated, 0 if not
 * 
 */
int ftserve_check_user(char*user, char*pass)
{
	char username[MAXSIZE];
	char password[MAXSIZE];
	char *pch;
	char buf[MAXSIZE];
	char *line = NULL;
	size_t num_read;									
	size_t len = 0;
	FILE *fd;
	
	fd = fopen(".auth", "r");
	if (fd == NULL) {
		perror("file not found");
		exit(1);
	}	

	while ((num_read = getline(&line, &len, fd)) != -1) {
		memset(buf, 0, MAXSIZE);
		strcpy(buf, line);
		
		pch = strtok (buf," ");
		strcpy(username, pch);

		if (pch != NULL) {
			pch = strtok (NULL, " ");
			strcpy(password, pch);
		}

		// remove end of line and whitespace
		trimstr(password, (int)strlen(password));
		
		if ((strcmp(user,"admin")==0) && (strcmp(pass,"admin")==0))
		{
			admin = 1;
			printf("Admin logged in.\n");
			break;
		}
		else if ((strcmp(user,username)==0) && (strcmp(pass,password)==0)) 
		{
			admin = 0;
			printf("User logged in.\n");
			break;
		}		
	}
	free(line);	
	fclose(fd);	
	return admin;
}


/** 
 * \function
 * Log in connected client
 * 
 */
int ftserve_login(int sock_control)
{	
	char buf[MAXSIZE];
	char user[MAXSIZE];
	char pass[MAXSIZE];	
	memset(user, 0, MAXSIZE);
	memset(pass, 0, MAXSIZE);
	memset(buf, 0, MAXSIZE);
	
	// Wait to recieve username
	if ( (recv_data(sock_control, buf, sizeof(buf)) ) == -1) {
		perror("recv error\n"); 
		exit(1);
	}	

	int i = 5;
	int n = 0;
	while (buf[i] != 0)
		user[n++] = buf[i++];
	
	// tell client we're ready for password
	send_response(sock_control, 331);					
	
	// Wait to recieve password
	memset(buf, 0, MAXSIZE);
	if ( (recv_data(sock_control, buf, sizeof(buf)) ) == -1) {
		perror("recv error\n"); 
		exit(1);
	}
	
	i = 5;
	n = 0;
	while (buf[i] != 0) {
		pass[n++] = buf[i++];
	}
	
	return (ftserve_check_user(user, pass));
}


/**
 * \function
 * Wait for command from client and fill up cmd and arg
 * and
 * send response
 * Returns response code
 * 
 */
int ftserve_recv_cmd(int sock_control, char*cmd, char*arg)
{	
	int rc = 200;
	char buffer[MAXSIZE];
	
	memset(buffer, 0, MAXSIZE);
	memset(cmd, 0, 5);
	memset(arg, 0, MAXSIZE);
		
	// Wait to recieve command
	if ((recv_data(sock_control, buffer, sizeof(buffer)) ) == -1) {
		perror("recv error\n"); 
		return -1;
	}
	
	strncpy(cmd, buffer, 4);
	char *tmp = buffer + 5;
	strcpy(arg, tmp);
	
	printf("Komenda otrzymana od klienta: %s\n", cmd);
	printf("Argumenty do komendy: %s\n", arg);
	
	if (strcmp(cmd, "QUIT")==0) 
	{
		rc = 221;
	} 
	else if((strcmp(cmd, "USER")==0) || (strcmp(cmd, "PASS")==0) 
            || (strcmp(cmd, "LIST")==0) || (strcmp(cmd, "RETR")==0) 
            || (strcmp(cmd, "POKA")==0) || (strcmp(cmd, "ADD_")==0)
            || (strcmp(cmd, "DEL_")==0) || (strcmp(cmd, "CHNG")==0))
	{
		rc = 200;
	} 
	else 
	{ //invalid command
		rc = 502;
	}

	send_response(sock_control, rc);	
	return rc;
}


/** 
 * \function
 * Child process handles connection to client
 * 
 */
void ftserve_process(int sock_control)
{
	int sock_data;
	char cmd[5];
	char arg[MAXSIZE];

	// Send welcome message
	send_response(sock_control, 220);

	// Authenticate user
	int k = ftserve_login(sock_control);
	if (k == 0) 
	{
		send_response(sock_control, 230);
	}
	else if (k == 1)
	{	
		send_response(sock_control, 330);
	}
	else 
	{
		send_response(sock_control, 430);	
		exit(0);
	}	
	
	while (1) {
		// Wait for command
		int rc = ftserve_recv_cmd(sock_control, cmd, arg);
		
		printf("SERWER loop otrzymalo ftserve_recv_cmd RC: %d.\n", rc);
        printf("SERWER loop otrzymalo ftserve_recv_cmd cmd->code: %s.\n", cmd);
        
        /* wypisujemy i zapisujemy ewentualne argumenty komendy */
        char *argument;
        char arg_array[3][50];

        if (strlen(arg)>0)
        {
            printf("Obslugiwane i zapisywane argumenty komendy...\n");    
            argument = strtok(arg, " ");
            strncpy(arg_array[0], argument, sizeof(arg_array[0]));
            printf("ftserve_recv_cmd cmd->argument_1: %s\n", arg_array[0]);
		    int i = 1;
		    while ((argument = strtok (NULL, " ")) != NULL)
		    {
		        strncpy(arg_array[i], argument, sizeof(arg_array[i]));
		        printf("ftserve_recv_cmd cmd->argument_%d: %s\n", i+1, arg_array[i]);
		        i++;
		    }
        }
 
        /* kiedy ftserve_recv_cmd zwrocilo quit lub jakis blad */
		if ((rc < 0) || (rc == 221)) {
			break;
		}
		
		
		/* kiedy ftserve_recv_cmd zwrocilo jakas komende */
		if (rc == 200 ) 
        {
			printf("[+]Otrzymano komende i nastepuje jej procesowanie...\n");
            // Open data connection with client
			if ((sock_data = ftserve_start_data_conn(sock_control)) < 0) {
				close(sock_control);
				exit(1); 
			}

			// Execute command
			if (strcmp(cmd, "LIST")==0) 
            { // Do list
                printf("[+]Wykonowywana komenda 'list' przez SERWER\n");
				ftserve_list(sock_data, sock_control);
			} else if (strcmp(cmd, "POKA")==0) {
                printf("[+]Wykonowywana komenda 'pokaz' przez SERWER\n");
                printf("[+]Szukany kod kreskowy char: %s long int: %ld \n",arg_array[0], char_to_long(arg_array[0]));
				ftserve_pokaz_towar(sock_data, sock_control, char_to_long(arg_array[0]));
			} else if (strcmp(cmd, "ADD_")==0 && admin) {
                printf("[+]Wykonowywana komenda 'doda' przez SERWER\n");
                printf("[+]Dodajemy towar: %ld, %s, %lg \n", char_to_long(arg_array[0]), arg_array[1], atof(arg_array[2]));
				ftserve_dodaj_towar(sock_data, sock_control, char_to_long(arg_array[0]), arg_array[1], atof(arg_array[2]));
			} else if (strcmp(cmd, "DEL_")==0 && admin) {
                printf("[+]Wykonowywana komenda 'kasu' przez SERWER\n");
                printf("[+]Kasujemy towar o kodzie kreskowym: %ld \n", char_to_long(arg_array[0]));
				ftserve_skasuj_towar(sock_data, sock_control, char_to_long(arg_array[0]));
			} else if (strcmp(cmd, "CHNG")==0 && admin) {
                printf("[+]Wykonowywana komenda 'zmie' przez SERWER\n");
                printf("[+]Kasujemy towar o kodzie kreskowym: %ld \n", char_to_long(arg_array[0]));
				ftserve_zmien_towar(sock_data, sock_control, char_to_long(arg_array[0]), arg_array[1], atof(arg_array[2]));
			} else {
                printf("Brak uprawnien do wykonywania komendy\n");
                ftserve_brak_uprawnien(sock_data, sock_control);
            }
		
			// Close data connection
			close(sock_data);
		} 
	}
}



