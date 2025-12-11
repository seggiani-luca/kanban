#include "net.h"
#include "../../shared/core_const.h" // costanti core
#include "../../shared/net_const.h"  // costanti di rete
#include "../log/log.h"              // logging
#include <arpa/inet.h>               // internet
#include <errno.h>                   // errno
#include <netinet/in.h>              // internet
#include <stdio.h>                   // perror, log_event
#include <string.h>                  // utilità stringa
#include <sys/select.h>              // primitive select, fd_set
#include <sys/socket.h>              // socket
#include <sys/types.h>               // socket
#include <unistd.h>                  // primitive socket

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
 * Filde massimo nei set master e di lettura
 */
int fdmax;

/*
 * Helper che ricalcola fdmax dopo che si è chiuso un socket
 */
int recalc_fdmax(fd_set *set) {
  // scansiona l'fd_set dall'alto, cercando il più grande impostato
  for (int i = FD_SETSIZE - 1; i >= 0; i--) {
    if (FD_ISSET(i, set))
      return i;
  }

  // se sei qui l'fd_set è vuoto
  return -1;
}

// ==== GESTIONE CONNESSIONI ====

/*
 * Rappresenta la connessione con un client, rappresentata da:
 * - socket della connessione (se è 0 la connessione è nulla)
 * - porta della connessione
 * - buffer di lettura
 * - caratteri letti in buffer
 */
typedef struct {
  int sock;
  int port;
  char read_buf[NET_BUF_SIZE];
  int read_len;
} connection;

/*
 * Vettore connessioni
 */
connection connections[MAX_CLIENTS] = {0};

/*
 * Connessione della shell
 */
connection admin_conn = {0};

/*
 * Registra una nuova connessione nel vettore connessioni e restituisce il suo
 * indice. Inserisce anche nel fd_set
 */
int register_conn(int sock, int port) {
  // controlla che la porta sia valida
  if (port < CLIENT_MIN_PORT || port >= CLIENT_MAX_PORT) {
    log_event("[kanban]\t: Client %d ha richiesto connessione con numero di "
              "porta invalido\n",
              port);
    return -1;
  }

  // cerca uno slot libero per la connessione
  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (connections[i].sock == 0) {
      log_event("[kanban]\t: Client %d connesso\n", port);

      // registra la connessione
      connections[i].sock = sock;
      connections[i].port = port;
      connections[i].read_len = 0;

      // aggiorna master set
      FD_SET(sock, &master_set);
      if (sock > fdmax)
        fdmax = sock;

      return i;
    }
  }

  // se sei qui non ci sono slot liberi, stampa errore
  log_event("[kanban]\t: Client %d ha richiesto connessione con spazio "
            "esaurito\n",
            port);
  return -1;
}

/*
 * Deregistra una connessione dal vettore connessioni, e la rimuove dai fd_set
 */
void unregister_conn(connection *conn) {
  int sock = conn->sock;
  log_event("[kanban]\t: Client %d disconnesso\n", conn->port);

  // deregistra connessione impostando sock a 0
  conn->sock = 0;

  FD_CLR(sock, &master_set);
  close(sock);
  fdmax = recalc_fdmax(&master_set);
}

/*
 * Trova la connessione con dato socket nel vettore connessioni. Restituisce
 * NULL se non la trova
 */
connection *find_conn_by_sock(int sock) {
  // scansiona connessioni per socket
  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (connections[i].sock == sock) {
      return &connections[i];
    }
  }

  // se sei qui non hai trovato nulla
  return NULL;
}

/*
 * Trova la connessione con data porta nel vettore connessioni. Restituisce
 * NULL se non la trova
 */
connection *find_conn_by_port(int port) {
  // scansiona connessioni per porta (controllando socket non 0)
  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (connections[i].sock != 0 && connections[i].port == port) {
      return &connections[i];
    }
  }

  // se sei qui non hai trovato nulla
  return NULL;
}

// ==== GESTIONE SERVER ====

int configure_net() {
  log_event("[kanban]\t: Inizializzazione del server\n");

  // apri socket di ascolto
  listen_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_sock < 0) {
    printf("[kanban]\t: Creazione socket di ascolto fallita: %s\n",
           strerror(errno));
    return -1;
  }

  // configura indirizzo socket di ascolto
  struct sockaddr_in listen_addr;
  memset(&listen_addr, 0, sizeof(listen_addr));
  listen_addr.sin_family = AF_INET;
  listen_addr.sin_port = htons(SERVER_PORT);
  inet_pton(AF_INET, SERVER_ADDR, &listen_addr.sin_addr);

  // rendi il socket di ascolto riutilizzabile (per debugging più veloce)
  int yes = 1;
  setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

  // associa indirizzo al socket
  if (bind(listen_sock, (struct sockaddr *)&listen_addr, sizeof(listen_addr)) <
      0) {
    printf("[kanban]\t: Bind socket di ascolto fallita: %s\n", strerror(errno));
    close(listen_sock);
    return -1;
  }

  // metti il socket di ascolto, in ascolto
  if (listen(listen_sock, BACKLOG) < 0) {
    log_event("[kanban]\t: Impossibile ascoltare su socket di ascolto: %s\n",
              strerror(errno));
    close(listen_sock);
    return -1;
  }

  // configura set master
  FD_ZERO(&master_set);

  // inserisci il socket di ascolto nel set master
  FD_SET(listen_sock, &master_set);
  fdmax = listen_sock;

  // isnerisci stdin nel set master
  FD_SET(STDIN_FILENO, &master_set);

  // configura set di lettura
  FD_ZERO(&read_set);

  // configura filde massimo
  fdmax = listen_sock;

  return 0;
}

/*
 * Accetta la connessione di un nuovo client
 */
void accept_client() {
  // prepara indirizzo
  struct sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);

  // accetta connessione
  int client_sock =
      accept(listen_sock, (struct sockaddr *)&client_addr, &client_len);
  if (client_sock < 0) {
    log_event("[kanban]\t: Accept fallita: %s\n", strerror(errno));
    return;
  }

  // ottieni porta
  int client_port = ntohs(client_addr.sin_port);

  // aggiorna lista client
  if (register_conn(client_sock, client_port) < 0)
    return;
}

/*
 * Gestisce una singola linea ottenuta da un client
 */
void handle_line(int port, char *buf) {
  // stampa informazioni di debug
  log_event("[%d] -> [kanban]\t: %s\n", port, buf);

  // prepara comando
  cmd cm = {0};
  buf_to_cmd(buf, &cm);

  // esegui il comando
  exec_command(port, &cm);
}

/*
 * Gestisce un client, leggendo quanto ha scritto sul socket, concatenandolo al
 * suo buffer, e gestendo il buffer linea per linea
 */
int handle_client(connection *conn) {
  // calcola spazio rimanente e gestisci overflow
  int size = NET_BUF_SIZE - conn->read_len;
  if (size <= 0) {
    log_event("[kanban]\t: Buffer di client %d pieno, disconnetto", conn->port);
    return -1;
  }

  // leggi nel buffer: si fa una read perché conn->sock potrebbe essere un
  // socket come stdin
  int n = read(conn->sock, conn->read_buf + conn->read_len, size);

  // gestisci errori di lettura
  if (n <= 0)
    return n;

  // aggiungi i byte letti
  conn->read_len += n;

  // elabora intere linee, separando con \n
  int proc = 0;
  for (int i = 0; i < conn->read_len; i++) {
    // se trovi \n è una linea
    if (conn->read_buf[i] == '\n') {
      conn->read_buf[i] = '\0';
      handle_line(conn->port, conn->read_buf + proc);

      proc = i + 1;
    }
  }

  // sposta dati rimanenti all'inizio del buffer
  if (proc > 0) {
    memmove(conn->read_buf, conn->read_buf + proc, conn->read_len - proc);
    conn->read_len -= proc;
  }

  return n;
}

/*
 * Gestisce un client a partire dal suo socket, trovando la sua connessione e
 * chiamando handle_client()
 */
void handle_client_sock(int i) {
  // ottieni la connessione del client
  connection *conn = find_conn_by_sock(i);
  if (conn == NULL) {
    log_event("[kanban]\t: Richiesta da socket %d, non registrato\n", i);
    return;
  }

  // gestisci la richiesta del client
  int ret = handle_client(conn);
  if (ret <= 0) {
    // se restituisce meno o uguale a zero, il client si è disconnesso
    unregister_conn(conn);
  }
}

/*
 * Chiede al core di gestire il timer relativo al client associato ad una certa
 * connessione
 */
int handle_timer(int sock) {
  if (sock == listen_sock || sock == STDIN_FILENO)
    return 0;

  // ottieni la connessione del client
  connection *conn = find_conn_by_sock(sock);
  if (conn == NULL) {
    log_event(
        "[kanban]\t: Impossibile gestire timer di socket %d, non registrato\n",
        sock);
    return 0;
  }

  // controlla timer
  int ret = check_timer(conn->port);
  if (ret < 0)
    unregister_conn(conn);

  return ret;
}

int listen_net() {
  int updated = 0;

  // gestisci i timer
  for (int i = 0; i <= fdmax; i++) {
    if (!FD_ISSET(i, &master_set))
      continue;

    updated = (handle_timer(i) != 0);
  }

  // copia set master nel set di ascolto
  read_set = master_set;

  // scansiona con la select
  struct timeval tv = {.tv_sec = 1};
  select(fdmax + 1, &read_set, NULL, NULL, &tv);
  for (int i = 0; i <= fdmax; i++) {
    // controlla che si qualcosa da leggere
    if (!FD_ISSET(i, &read_set))
      continue;

    if (i == listen_sock) // accetta nuovo client
      accept_client();
    else if (i == STDIN_FILENO) // gestisci console amminisratore
      handle_client(&admin_conn);
    else // gestisci client
      handle_client_sock(i);

    updated = 1;
  }

  return updated;
}

void close_net() {
  printf("^C -> [kanban]\t: Chiusura dei socket aperti\n");

  // chiudi tutti i socket
  for (int i = 0; i <= fdmax; i++) {
    if (FD_ISSET(i, &master_set)) {
      FD_CLR(i, &master_set);
      close(i);
    }
  }
}

// ==== TRASMISSIONE ====

void send_cmd(client_id cl_id, const cmd *cm) {
  // serializza comando
  char buf[NET_BUF_SIZE];
  cmd_to_buf(cm, buf);

  if (cl_id != 0) {
    // ottieni connessione
    connection *conn = find_conn_by_port(cl_id);
    if (conn == NULL) {
      log_event("[kanban]\t: Richiesto invio a socket non registrato\n");
      return;
    }

    // invia comando
    send(conn->sock, buf, strlen(buf), 0);
    send(conn->sock, "\n", 1, 0);
  }

  // stampa informazioni di debug
  log_event("[kanban] -> [%d]\t: %s\n", cl_id, buf);
}
