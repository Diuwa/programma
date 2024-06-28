#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h>
#include <strings.h>
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h> // read(), write(), close()
#include <arpa/inet.h>
#include <getopt.h>
#define MAX 2000 
#define PORT 8080 
#define SA struct sockaddr 

void receive_file(int connfd, const char* dest_directory) 
{ 
    char file_name[200]; 
    int bytes_received;
    // Riceve il nome del file
    bytes_received = read(connfd, file_name, sizeof(file_name)); 
    if (bytes_received <= 0) {
        printf("Errore nella ricezione del nome del file\n");
        close(connfd);
        return;
    }
	file_name[bytes_received] = '\0'; // Assicura la null-terminazione

    // Crea il percorso completo del file di destinazione
    char dest_path[256];
    snprintf(dest_path, sizeof(dest_path), "%s/%s", dest_directory, file_name);

    FILE *fp = fopen(dest_path, "wb");
    if (fp == NULL) {
        printf("Errore nell'apertura del file %s\n", dest_path);
        close(connfd);
        return;
    }

	char buffer[MAX];
    size_t bytes_written;
	bzero(buffer, sizeof(buffer)); // Azzera il buffer dopo ogni scrittura
    while ((bytes_received = read(connfd, buffer, sizeof(buffer))) > 0) {
        bytes_written = fwrite(buffer, 1, bytes_received, fp);
        if (bytes_written != bytes_received) {
            printf("Errore nella scrittura dei dati nel file %s\n", dest_path);
            break;
        }
		bzero(buffer, sizeof(buffer)); // Azzera il buffer dopo ogni scrittura
    }

    if (bytes_received < 0) {
        printf("Errore nella ricezione dei dati\n");
    } else {
        printf("File %s ricevuto con successo e salvato in %s\n", file_name, dest_path);
    }

    fclose(fp);
	// non compie la traduzione da binario a formato scritto, come risolvere???????
} 

// Function designed for chat between client and server. 
void func(int connfd) 
{ 

	char buff[MAX]; 

	
	int n; 
	// infinite loop for chat 
	
	bzero(buff, MAX); 

	// read the message from client and copy it in buffer 
	read(connfd, buff, sizeof(buff)); 
	// print buffer which contains the client contents 
	printf("From client: %s\t To client : ", buff); 
	bzero(buff, MAX); 
	n = 0; 
	// copy server message in the buffer 
	while ((buff[n++] = getchar()) != '\n');

	// and send that buffer to client 
	write(connfd, buff, sizeof(buff)); 

	// if msg contains "Exit" then server exit and chat ended. 
	if (strncmp("exit", buff, 4) == 0) { 
		printf("Server Exit...\n"); 
	}
} 

// Driver function 
int main(int argc, char *argv[]) 
{ 
	int opt;
	int sockfd, connfd, len; 
	struct sockaddr_in servaddr, cli; 

	char *server_ip = NULL;
    int port;
	char *dest_directory = NULL;

	while((opt = getopt(argc, argv, "a:p:d:")) != -1){
	switch (opt)
	{
	case 'a':
		server_ip = optarg;
		break;
	
	case 'p':
		port = atoi(optarg);
		break;
	case 'd':
		dest_directory = optarg;
		break;
	}
		
		
	}
	
	// socket create and verification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) { 
		printf("socket creation failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully created..\n"); 
	bzero(&servaddr, sizeof(servaddr)); 

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 

    //servaddr.sin_addr.s_addr = htonl(server_ip);
    if (inet_pton(AF_INET, server_ip, &servaddr.sin_addr) <= 0) {
        printf("Invalid address/ Address not supported \n");
        exit(0);
    }
	servaddr.sin_port = htons(port);

	// Binding newly created socket to given IP and verification 
	if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
		printf("socket bind failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully binded..\n"); 

	// Now server is ready to listen and verification 
	if ((listen(sockfd, 5)) != 0) { 
		printf("Listen failed...\n"); 
		exit(0); 
	} 
	else
		printf("Server listening..\n"); 
	len = sizeof(cli); 

	// Accept the data packet from client and verification 
	connfd = accept(sockfd, (SA*)&cli, &len); 
	if (connfd < 0) { 
		printf("server accept failed...\n"); 
		exit(0); 
	} 
	else
		printf("server accept the client...\n"); 

	// Function for chatting between client and server 
	receive_file(connfd, dest_directory);

	close(connfd);
	// After chatting close the socket 
	close(sockfd); 
}
