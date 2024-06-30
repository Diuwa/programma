#include <arpa/inet.h> // inet_addr()
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> // bzero()
#include <sys/socket.h>
#include <unistd.h> // read(), write(), close()
#define MAX 2000
#define PORT 8080
#define SA struct sockaddr

void send_file(int sockfd, const char* file_path)
{
    char file_name[256];
    FILE *fp = fopen(file_path, "rb");

    if (fp == NULL) {
        printf("Errore nell'apertura del file %s\n", file_path);
        close(sockfd);
        return;
    }

    // Invia il nome del file
    strncpy(file_name, strrchr(file_path, '/') ? strrchr(file_path, '/') + 1 : file_path, sizeof(file_name) - 1);
    file_name[sizeof(file_name) - 1] = '\0'; // Assicura la null-terminazione
    write(sockfd, file_name, strlen(file_name));  // Invia il nome del file senza carattere NULL finale

    // Invia il contenuto del file
    char buffer[MAX];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
    	printf("Bytes read: %zu\n", bytes_read); // Debug print
    	for (int i = 0; i < bytes_read; i++) {
        	printf("%02x ", (unsigned char)buffer[i]);
    	}
    	printf("\n");
    	write(sockfd, buffer, bytes_read);
	}

    printf("File %s inviato con successo.\n", file_name);
    fclose(fp);
    close(sockfd);
}


void func(int sockfd)
{
	char buff[MAX];
	int n;
	for (;;) {
		bzero(buff, sizeof(buff));
		printf("Enter the string : ");
		n = 0;
		while ((buff[n++] = getchar()) != '\n')
			;
		write(sockfd, buff, sizeof(buff));
		bzero(buff, sizeof(buff));
		read(sockfd, buff, sizeof(buff));
		printf("From Server : %s", buff);
		if ((strncmp(buff, "exit", 4)) == 0) {
			printf("Client Exit...\n");
			break;
		}
	}

	/*
	strcpy(buff, "1010101");

    if (write(sockfd, buff, strlen(buff)) < 0)
        printf("Errore nell'invio al server");
	else
    	printf("Dati inviati al server: %s\n", buff);
	*/
}

int main(int argc, char *argv[])
{
	int sockfd, connfd;
	struct sockaddr_in servaddr, cli;

	char *server_ip = argv[1];
    int port = atoi(argv[2]);
	char *file_path = argv[3];

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
	//servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	//servaddr.sin_port = htons(PORT);
	servaddr.sin_addr.s_addr = inet_addr(server_ip);
	servaddr.sin_port = htons(port);

	// connect the client socket to server socket
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr))
		!= 0) {
		printf("connection with the server failed...\n");
		exit(0);
	}
	else
		printf("connected to the server..\n");

	// function for chat
	send_file(sockfd, file_path);

	// close the socket
	close(sockfd);
}
