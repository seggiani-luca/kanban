#include "net.h"
#include "../../shared/net_const.h"	// costanti di rete
#include <string.h>									// utilità stringa
#include <stdio.h>									// perror, printf
#include <sys/types.h>							// socket internet
#include <sys/socket.h>							// ...
#include <arpa/inet.h>							// ...
#include <netinet/in.h>							// ...
#include <unistd.h>									// primitive socket

// ==== GESTIONE CLIENT ====

/*
 * Socket del client
 */
int client_sock = 0;

int configure_net(int port) {
	// apri socket del client
	client_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(client_sock < 0) {
		perror("Creazione socket fallita");
		return -1;
	}

	// configura indirizzo socket del client 
	struct sockaddr_in client_addr;
	memset(&client_addr, 0, sizeof(client_addr));
	client_addr.sin_family = AF_INET;
	client_addr.sin_port = htons(port);
	inet_pton(AF_INET, CLIENT_ADDR, &client_addr.sin_addr);
	
	// rendi il socket del client riutilizzabile (per debugging più veloce)
	int yes = 1;
	setsockopt(client_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
	
	// associa indirizzo al socket del client
	if(bind(client_sock, (struct sockaddr*) &client_addr, sizeof(client_addr)) 
			< 0) {
		perror("Bind socket fallita");
		close(client_sock);
		return -1;
	}
	
	// configura indirizzo socket del server
	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVER_ADDR, &serv_addr.sin_addr);

	// connetti al server
	if(connect(client_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) 
			< 0) {
		perror("Connessione al server fallita");
		close(client_sock);
		return -1;
	}

	return 0;
}

void close_net() {
	close(client_sock);
}

// ==== TRASMISSIONE ====

void send_cmd(const cmd* cm) {
	// serializza comando 
	char buf[NET_BUF_SIZE];
	cmd_to_buf(cm, buf);

	// invia comando 
	send(client_sock, buf, strlen(buf), 0);
	send(client_sock, "\n", 1, 0);
}

/*
 * Buffer di ricezione
 */
char recv_buf[NET_BUF_SIZE];

/*
 * Dimensione in buffer di ricezione
 */
int recv_len = 0;

/*
 * Ottiene una stringa terminata da \n dal server
 */
int recv_str(char** line) {
	while (1) {
		for (int i = 0; i < recv_len; i++) {
			if (recv_buf[i] == '\n') {
				recv_buf[i] = '\0';
				
				// line è una linea
				*line = recv_buf;

				// sposta dati rimanenti 
				int rem = recv_len - (i + 1);
				if (rem > 0) {
						memmove(recv_buf, recv_buf + i + 1, rem);
				}
				recv_len = rem;

				return i;
			}
		}

		// leggi nel buffer
		int n = recv(client_sock, recv_buf + recv_len, NET_BUF_SIZE - recv_len, 0);
		if (n <= 0) return n;

		recv_len += n;
	}
}

int recv_cmd(cmd* cm) {
	// ricevi stringa 
	char* line;
	if(recv_str(&line) <= 0) return -1;
	
	// deserializza la stringa
	buf_to_cmd(line, cm);

	return 0;
}
