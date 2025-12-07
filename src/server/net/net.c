#include "net.h"
#include "../../shared/net_const.h"
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/select.h>
#include <stdlib.h>

// ==== GESTIONE SOCKET ====

/*
 * Filde del socket di ascolto
 */
int listen_sock;

/*
 * Set master
 */
fd_set master_set;

/*
 * Set di lettura
 */
fd_set read_set;

/*
 * Filde massimo nei set
 */
int fdmax;

int configure_net() {
	// apri socket di ascolto
	listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(listen_sock < 0) {
		perror("Crezione socket di ascolto fallita");
		return -1;
	}

	// configura indirizzo socket di ascolto 
	struct sockaddr_in listen_addr;
	memset(&listen_addr, 0, sizeof(listen_addr));
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVER_ADDR, &listen_addr.sin_addr);

	// associa indirizzo al socket
	if(bind(listen_sock, (struct sockaddr*) &listen_addr, sizeof(listen_addr)) 
			< 0) {
		perror("Bind socket di ascolto fallita");
		close(listen_sock);
		return -1;
	}
	
	// metti il socket di ascolto, in ascolto
	if(listen(listen_sock, BACKLOG) < 0) {
		perror("Impossibile ascoltare su socket di ascolto");
		close(listen_sock);
		return -1;
	}

	// configura set master
	FD_ZERO(&master_set);

	// inserisci il socket di ascolto nel set master
	FD_SET(listen_sock, &master_set);
	fdmax = listen_sock;

	// configura set di lettura
	FD_ZERO(&read_set);

	// configura filde massimo
	fdmax = listen_sock;

	// configura callback
	set_reply_callback(reply_net_cmd);

	return 0;
}

// ==== GESTIONE CONNESSIONI ====

/*
 * Dimensione massima buffer letto da socket client
 */
#define NET_BUF_SIZE 100

/*
 * Rappresenta la connessione con un client, rappresentata da:
 * - socket della connessione (se è 0 la connessione è nulla)
 * - porta della connessione
 * - buffer di lettura
 * - caratteri letti in buffer
 */
struct connection {
	int sock;
	int port;
	char read_buf[NET_BUF_SIZE];
	int read_len;
};

/*
 * Vettore connessioni 
 */
static struct connection connections[MAX_CLIENTS] = {0};

/*
 * Registra una nuovo connessione nel vettore connessioni e restituisce il suo
 * indice
 */
int register_conn(int sock, int port) {
	if(port < CLIENT_MIN_PORT || port >= CLIENT_MAX_PORT) {
		printf("Client %d ha richiesto connessione con numero di porta invalido\n",
				port);
		return -1;
	}

	for(int i = 0; i < MAX_CLIENTS; i++) {
		if(connections[i].sock == 0) {
			printf("Client %d connesso\n", port);

			connections[i].sock = sock;
			connections[i].port = port;
			connections[i].read_len = 0;

			return i;
		}
	}

	printf("Client %d ha richiesto connessione con spazio esaurito\n", port);
	return -1; 
}

/*
 * Deregistra una connessione dal vettore connessioni 
 */
void unregister_conn(struct connection* conn) {
	printf("Client %d disconnesso\n", conn->port);
	conn->sock = 0;
}

/*
 * Trova la connessione con dato socket nel vettore connessioni. Restituisce 
 * NULL se non la trova
 */
struct connection* find_conn_by_sock(int sock) {
	for(int i = 0; i < MAX_CLIENTS; i++) {
		if(connections[i].sock == sock) {
			return &connections[i];
		}
	}

	return NULL;
}

/*
 * Trova la connessione con data porta nel vettore connessioni. Restituisce 
 * NULL se non la trova
 */
struct connection* find_conn_by_port(int port) {
	for(int i = 0; i < MAX_CLIENTS; i++) {
		if(connections[i].port == port) {
			return &connections[i];
		}
	}

	return NULL;
}

// ==== GESTIONE SERVER ====

/*
 * Gestisce una singola linea ottenuta da un client
 */
void handle_line(int port, char* buf, int len) {
	// stampa informazioni di debug
	printf("[%d] -> [kanban] : %.*s\n", port, len, buf);

	// prepara messaggio
	int argc;
	char* argv[MAX_NET_ARGS];

	// tokenizza la stringa
	argc = 0;
	char* token = strtok(buf, " ");
	while(token && argc < MAX_NET_ARGS + 1) {
		argv[argc++] = token;
  	token = strtok(NULL, " ");
  }

	// interpreta il comando
	parse_command(port, argc, argv);
}

/*
 * Gestisce un client, leggendo quanto ha scritto sul socket, concatenandolo al
 * suo buffer, e gestendo il buffer linea per linea
 */
int handle_client(struct connection* conn) {
	// leggi nel buffer
	int n = recv(conn->sock, conn->read_buf + conn->read_len, 
			NET_BUF_SIZE - conn->read_len, 0);

	if(n <= 0) return n;

	conn->read_len += n;

 // process full lines
	int proc = 0;
	for (int i = 0; i < conn->read_len; i++) {
		if (conn->read_buf[i] == '\n') {
			conn->read_buf[i] = '\0';
			int line_len = i - proc;
			
			// buf + proc, line_len è una linea
			handle_line(conn->port, conn->read_buf + proc, line_len);

			proc = i + 1;
		}
	}

	// sposta dati rimanenti 
	if (proc > 0) {
		memmove(conn->read_buf, conn->read_buf + proc, conn->read_len - proc);
		conn->read_len -= proc;
	}

	return n;
}

void listen_net() {
	while(1) {
		// copia set master nel set di ascolto 
		read_set = master_set;

		// scansiona con la select 
		select(fdmax + 1, &read_set, NULL, NULL, NULL);
		for(int i = 0; i < fdmax + 1; i++) {
			if(!FD_ISSET(i, &read_set)) continue;

			if(i == listen_sock) {
				// è il socket di ascolto, collega un nuovo client
				struct sockaddr_in client_addr;
				socklen_t client_len = sizeof(client_addr);

				int client_sock = accept(i, (struct sockaddr*) &client_addr, 
						&client_len);
				if(client_sock < 0) {
					perror("Accept fallita");
					continue;
				}

				int client_port = ntohs(client_addr.sin_port);
	
				// aggiorna lista client
				if(register_conn(client_sock, client_port) < 0) continue;
			
				// aggiorna master set
				FD_SET(client_sock, &master_set);
				if(client_sock > fdmax) fdmax = client_sock;
			} else {
				// è un client
				struct connection* conn = find_conn_by_sock(i);
				if(conn == NULL) {
					printf("Richiesta da socket non registrato\n");
					continue;
				}

				if(handle_client(conn) < 0) {
					FD_CLR(i, &master_set);
					unregister_conn(conn);
				}
			}
		}
	}
}

// ==== RICEZIONE E RISPOSTA ====

void reply_net_cmd(client_id cl, int argc, const char* argv[]) {
	// ottieni connessione
	struct connection* conn = find_conn_by_port(cl);
	if(conn == NULL) {
		printf("Risposta a socket non registrato\n");
		return;
	}
	
	// stampa informazioni di debug
	printf("[kanban] -> [%d] : ", cl);

	// rispondi
	for(int i = 0; i < argc; i++) {
		send(conn->sock, argv[i], strlen(argv[i]), 0);
		send(conn->sock, " ", 1, 0);
		
		printf("%s ", argv[i]);
	}

	send(conn->sock, "\n", 1, 0);
	
	printf("\n");
}
