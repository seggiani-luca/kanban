#include "net.h"
#include "../../shared/net_const.h" // costanti di rete
#include "../rev/rev.h"             // gestione revisioni
#include "../watch/watch.h"         // gestione watchdog lavagna
#include <arpa/inet.h>              // internet
#include <errno.h>                  // errno
#include <netinet/in.h>             // internet
#include <stdio.h>                  // perror, printf
#include <string.h>                 // utilità stringa
#include <sys/select.h>             // select
#include <sys/socket.h>             // socket
#include <sys/types.h>              // socket
#include <unistd.h>                 // primitive socket

// ==== GESTIONE SOCKET ====

int port;

/*
 * Socket TCP del client, usato per la comunicazione col server
 */
int tcp_sock = 0;

/*
 * Socket del client, usato per la comunicazione coi peer
 */
int udp_sock = 0;

/*
 * Set master
 */
fd_set master_set;

/*
 * Set di lettura
 */
fd_set read_set;

/*
 * Filde massimo nei set master e di lettura
 */
int fdmax;

// ==== GESTIONE CLIENT =====

/*
 * Indirizzo di questo client
 */
struct sockaddr_in client_addr;

/*
 * Indirizzo del generico peer, varia solo il numero di porta
 */
struct sockaddr_in peer_addr;

int configure_tcp_sock() {
  // apri socket TCP
  tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (tcp_sock < 0) {
    printf("[%d]\t: Creazione socket TCP fallita: %s\n", port, strerror(errno));
    return -1;
  }

  // rendi il socket TCP riutilizzabile (per debugging più veloce)
  int yes = 1;
  setsockopt(tcp_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

  // associa indirizzo al socket TCP
  if (bind(tcp_sock, (struct sockaddr *)&client_addr, sizeof(client_addr)) <
      0) {
    printf("[%d]\t: Bind socket TCP fallita: %s\n", port, strerror(errno));
    close(tcp_sock);
    return -1;
  }

  // configura indirizzo socket del server
  struct sockaddr_in serv_addr;
  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(SERVER_PORT);
  inet_pton(AF_INET, SERVER_ADDR, &serv_addr.sin_addr);

  // connetti al server
  if (connect(tcp_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    printf("[%d]\t: Connessione al server fallita: %s\n", port,
           strerror(errno));
    close(tcp_sock);
    return -1;
  }

  return 0;
}

int configure_udp_sock() {
  // apri socket UDP
  udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (udp_sock < 0) {
    printf("[%d]\t: Creazione socket peer fallita: %s\n", port,
           strerror(errno));
    return -1;
  }

  // associa indirizzo al socket UDP
  if (bind(udp_sock, (struct sockaddr *)&client_addr, sizeof(client_addr)) <
      0) {
    printf("[%d]\t: Bind socket peer fallita: %s\n", port, strerror(errno));
    close(udp_sock);
    return -1;
  }

  // configura indirizzo del generico peer
  memset(&peer_addr, 0, sizeof(peer_addr));
  peer_addr.sin_family = AF_INET;
  inet_pton(AF_INET, CLIENT_ADDR, &peer_addr.sin_addr);

  return 0;
}

int configure_net() {
  printf("[%d]\t: Inizializzazione del client\n", port);

  // configura indirizzo del client
  memset(&client_addr, 0, sizeof(client_addr));
  client_addr.sin_family = AF_INET;
  client_addr.sin_port = htons(port);
  inet_pton(AF_INET, CLIENT_ADDR, &client_addr.sin_addr);

  // configura socket TCP e UDP
  if (configure_tcp_sock() < 0)
    return -1;
  if (configure_udp_sock() < 0)
    return -1;

  // configura set master
  FD_ZERO(&master_set);

  // inserisci i socket TCP e UDP nel set master
  FD_SET(tcp_sock, &master_set);
  FD_SET(udp_sock, &master_set);
  fdmax = tcp_sock > udp_sock ? tcp_sock : udp_sock;

  return 0;
}

void close_net() {
  // chiudi socket TCP e UDP
  close(tcp_sock);
  close(udp_sock);
}

// ==== TRASMISSIONE ====

void send_server(const cmd *cm) {
  // serializza comando
  char buf[CMD_BUF_SIZE];

  cmd_to_buf(cm, buf);
  int len = strlen(buf);
  buf[len++] = '\n';

  // invia comando, mantieni byte inviati
  int sent = 0;

  while (sent < len) {
    // invia n byte
    int n = send(tcp_sock, buf + sent, len - sent, 0);

    // gestisci errori
    if (n < 0) {
      printf("[%d]\t: Errore invio al server: %s\n", port, strerror(errno));
      return;
    }

    // gestisci disconnessione server
    if (n == 0) {
      printf("[%d]\t: Server disconnesso\n", port);
      return;
    }

    // conta i byte inviati e prosegui
    sent += n;
  }
}

void send_peer(unsigned short who, const cmd *cm) {
  // imposta porta del peer
  peer_addr.sin_port = htons(who);

  // serializza comando
  char buf[CMD_BUF_SIZE];

  cmd_to_buf(cm, buf);
  int len = strlen(buf);

  // invia comando, tutto o niente
  if (sendto(udp_sock, buf, len, 0, (struct sockaddr *)&peer_addr,
             sizeof(peer_addr)) < 0) {
    printf("[%d]\t: Errore invio al peer %d: %s\n", port, who, strerror(errno));
  }
}

/*
 * Buffer di ricezione per il socket TCP
 */
char recv_buf[CMD_BUF_SIZE];

/*
 * Dimensione stringa in buffer di ricezione TCP
 */
int recv_len = 0;

/*
 * Ottiene una stringa terminata da \n dal server
 */
int recv_line(char *buf) {
  while (1) {
    for (int i = 0; i < recv_len; i++) {
      if (recv_buf[i] == '\n') {
        recv_buf[i] = '\0';

        // line è una linea, sposta nel buffer temporaneo
        strcpy(buf, recv_buf);

        // sposta dati rimanenti all'inizio del buffer
        int rem = recv_len - (i + 1);
        if (rem > 0) {
          memmove(recv_buf, recv_buf + i + 1, rem);
        }
        recv_len = rem;

        return i;
      }
    }

    // controlla per overflow
    if (recv_len >= CMD_BUF_SIZE) {
      printf("[%d]\t: Overflow nel buffer di ricezione da server\n", port);
      return -1;
    }

    // leggi nel buffer
    int n = recv(tcp_sock, recv_buf + recv_len, CMD_BUF_SIZE - recv_len, 0);

    // gestisci errori
    if (n <= 0) {
      return n;
    }

    // conta i byte ricevuti
    recv_len += n;
  }
}

/*
 * Riceve un comando al server
 */
int recv_server(cmd *cm) {
  // ricevi stringa
  static char buf[CMD_BUF_SIZE];
  int n = recv_line(buf);

  // gestisci errori
  if (n <= 0) {
    return n;
  }

  // deserializza la stringa
  buf_to_cmd(buf, cm);

  return n;
}

/*
 * Riceve un comando da un peer
 */
int recv_peer(unsigned short *who, cmd *cm) {
  // configura peer
  struct sockaddr_in in;
  socklen_t in_len = sizeof(in);

  // ricevi stringa
  char buf[CMD_BUF_SIZE];
  int n = recvfrom(udp_sock, buf, CMD_BUF_SIZE - 1, 0, (struct sockaddr *)&in,
                   &in_len);

  // gstisci errori
  if (n < 0) {
    printf("[%d]\t: Errore ricezione da peer %d: %s\n", port,
           ntohs(in.sin_port), strerror(errno));
    return n;
  }
  *who = ntohs(in.sin_port);

  buf[n] = '\0';

  // deserializza la stringa
  buf_to_cmd(buf, cm);

  return n;
}

/*
 * Helper di recv_multio() che gestisce un singolo messaggio ricevuto.
 * Ottiene anche la porta sorgente nel caso di messaggi da peer
 */
int handle_cmd(unsigned int who, const cmd *cm) {
  mod_type mod = type_to_mod(cm->type);
  switch (mod) {
  case CORE:
    return 1;

  case WATCH:
    handle_ping();
    break;

  case PEER:
    handle_rev(who);
    break;
  }

  return 0;
}

int recv_multi(cmd *cm, int block) {
  while (1) {
    // se c'è qualcosa in sospeso sul buffer TCP, prendi subito
    if (recv_len > 0) {
      if (recv_server(cm) <= 0) {
        return ERR_TCP_SOCKET;
      }

      if (handle_cmd(0, cm) > 0) {
        return 1; // è core, esci
			}
    }

    // copia set master nel set di ascolto
    read_set = master_set;

    // imposta intervallo di un 1 secondo per la select
    struct timeval tv = {.tv_sec = 1, .tv_usec = 0};

    // scansiona con la select
    if (select(fdmax + 1, &read_set, NULL, NULL, block ? NULL : &tv) <= 0) {
      return 0;
    }

    // c'è qualcosa sul socket UDP?
    unsigned short who;
    if (FD_ISSET(udp_sock, &read_set)) {
      if (recv_peer(&who, cm) <= 0) {
        return ERR_UDP_SOCKET;
      }

      if (handle_cmd(who, cm) > 0) {
        return 1; // è core, esci
			}
    }

    // c'è qualcosa sul socket TCP?
    if (FD_ISSET(tcp_sock, &read_set)) {
      if (recv_server(cm) <= 0) {
        return ERR_TCP_SOCKET;
      }

      if (handle_cmd(0, cm) > 0)
        return 1; // è core, esci
    }
  }
}
