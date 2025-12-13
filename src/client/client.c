#include "core/core.h" // logica client
#include "net/net.h"   // gestione di rete client
#include "rev/rev.h"   // gestione review
#include <signal.h>    // segnali
#include <stdio.h>     // printf
#include <stdlib.h>    // exit
#include <time.h>      // randomizzazione
#include <unistd.h>    // sleep

/*
 * Il tempo massimo di attesa del client
 */
#define MAX_WAIT 10

/*
 * Flag di arresto
 */
volatile sig_atomic_t stop_flag = 0;

// ==== FUNZIONI HELPER ====

/*
 * Handler predisposto alla gestione di SIGINT per la chisura pulita della
 * connessione. Si limita ad impostare il flag di arresto
 */
void int_handler(int sig __attribute__((unused))) { stop_flag = 1; }

/*
 * Helper che aspetta un tempo casuale
 */
int rand_wait() {
  // ottieni un numero casuale fra 1 e MAX_WAIT
  int r = rand() % MAX_WAIT + 1;

  // aspetta per quel numero di secondi
  return ping_sleep(r);
}

// ==== FUNZIONI CLIENT ====

/*
 * Implementa il loop di esecuzione principale del client
 */
int client_loop() {
  while (1) {
    // ottieni una card
    card c = {0};
    unsigned short clients[MAX_CLIENTS];
    int num_clients;

    int ret = get_card(&c, clients, &num_clients);
    if (ret < 0) {
      return ret;
    }

    // esci se si è richiesto
    if (stop_flag) {
      return 0;
    }

    // aspetta un tempo casuale
    ret = rand_wait();
    if (ret < 0) {
      return ret;
    }

    // chiedi review
    for (int i = 0; i < num_clients; i++) {
      unsigned short client = clients[i];

      printf("[%d]\t: Chiedo valutazione a peer %d\n", port, client);
      if (req_review(client) <= 0) {
        printf("[%d]\t: Ottenuto responso negativo da peer %d\n", port, client);
        return ERR_PROTOCOL;
      }
    }

    // processa la card
    ret = card_done();
    if (ret < 0) {
      return ret;
    }
  }
}

int main(int argc, char *argv[]) {
  // imposta seed del generatore casuale
  srand(getpid() ^ time(NULL));

  // aspetta un po' di tempo per lasciare che gli altri client vengano avviati
  // (per debugging più pulito)
  sleep(1);

  // controlla argomenti
  if (argc < 2) {
    printf("Uso: ./client [numero di porta]\n");
    return 1;
  }

  // imposta gestore segnale SIGINT
  signal(SIGINT, int_handler);

  // configura socket
  port = atoi(argv[1]);
  if (configure_net() < 0) {
    return 1;
  }

  // registrati al server
  int ret = hello();

  // se registrato, inizia a fare richieste
  if (ret >= 0) {
    ret = client_loop();
  }

  if (ret != ERR_TCP_SOCKET) {
    // chiudi la connessione
    printf("[%d]\t: Termino chiudendo la connessione\n", port);
    quit();
  } else {
    // segnala di non voler chiudere la connessione
    printf("[%d]\t: Attenzione: termino dopo errore senza chiudere la "
           "connessione\n",
           port);
  }

  // chiudi socket
  close_net();

  return 0;
}
