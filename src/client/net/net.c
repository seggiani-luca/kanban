#include "net.h"
#include "../../shared/net_const.h" // costanti di rete
#include <arpa/inet.h>              // internet
#include <errno.h>                  // errno
#include <netinet/in.h>             // internet
#include <stdio.h>                  // perror, printf
#include <string.h>                 // utilità stringa
#include <sys/socket.h>             // socket
#include <sys/types.h>              // socket
#include <unistd.h>                 // primitive socket

// ==== GESTIONE CLIENT ====

int port;

/*
 * Socket del client
 */
int client_sock = 0;

int configure_net() {
  printf("[%d]\t: Inizializzazione del client\n", port);

  // apri socket del client
  client_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (client_sock < 0) {
    printf("[%d]\t: Creazione socket fallita: %s\n", port, strerror(errno));
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
  if (bind(client_sock, (struct sockaddr *)&client_addr, sizeof(client_addr)) <
      0) {
    printf("[%d]\t: Bind socket fallita: %s\n", port, strerror(errno));
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
  if (connect(client_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) <
      0) {
    printf("[%d]\t: Connessione al server fallita: %s\n", port,
           strerror(errno));
    close(client_sock);
    return -1;
  }

  return 0;
}

void close_net() { close(client_sock); }

// ==== TRASMISSIONE ====

void send_cmd(const cmd *cm) {
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
 * Buffer temporaneo per i comandi ricevuti
 */
char line_buf[NET_BUF_SIZE];

/*
 * Ottiene una stringa terminata da \n dal server. Permette di specificare un
 * timeout se è necessario fare recv. Se il timeout è 0, blocca finché non si
 * riceve qualcosa
 */
int recv_str(int flags) {
  while (1) {
    for (int i = 0; i < recv_len; i++) {
      if (recv_buf[i] == '\n') {
        recv_buf[i] = '\0';

        // line è una linea, sposta nel buffer temporaneo
        strcpy(line_buf, recv_buf);

        // sposta dati rimanenti all'inizio del buffer
        int rem = recv_len - (i + 1);
        if (rem > 0) {
          memmove(recv_buf, recv_buf + i + 1, rem);
        }
        recv_len = rem;

        return i;
      }
    }

    if (recv_len >= NET_BUF_SIZE)
      return -1; // overflow

    // leggi nel buffer
    int n =
        recv(client_sock, recv_buf + recv_len, NET_BUF_SIZE - recv_len, flags);
    if (n <= 0)
      return n;

    recv_len += n;
  }
}

int recv_cmd(cmd *cm, int flags) {
  // ricevi stringa
  if (recv_str(flags) <= 0) {
    return -1;
  }

  // deserializza la stringa
  buf_to_cmd(line_buf, cm);

  return 0;
}
