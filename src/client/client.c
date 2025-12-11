#include "core/core.h" // logica client
#include "net/net.h"   // gestione di rete client
#include "rev/rev.h"   // gestione review
#include <signal.h>    // segnali
#include <stdio.h>     // printf
#include <stdlib.h>    // exit
#include <unistd.h>    // sleep

/*
 * Il tempo massimo di attesa del client
 */
#define MAX_WAIT 10

// ==== FUNZIONI HELPER ====

/*
 * Handler predisposto alla gestione di SIGINT per la chisura pulita della
 * connessione
 */
void int_handler() {
  printf(" -> [%d]\t: Termino chiudendo la connessione\n", port);

  quit();
  close_net();
  exit(0);
}

/*
 * Helper che aspetta un tempo casuale
 */
int rand_wait() {
  // ottieni un numero casuale fra 1 e MAX_WAIT
  srand(getpid());
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
    int clients[MAX_CLIENTS];
    int num_clients;

    int ret = get_card(&c, clients, &num_clients);
    if (ret < 0)
      return ret;

    // aspetta un tempo casuale
    ret = rand_wait();
    if (ret < 0)
      return ret;

    // chiedi review
    // for(int i = 0; i < num_clients; i++) {
    // 	int client = clients[i];

    // 	if(req_review(client) < 0) {
    // 		printf("[%d]\t: Ottenuto responso negativo da peer\n", port);
    // 		return -1;
    // 	}
    // }

    // processa la card
    ret = card_done();
    if (ret < 0)
      return ret;
  }

  return 0;
}

int main(int argc, char *argv[]) {
  // aspetta un po' di tempo per lasciare che gli altri client vengano avviati
  sleep(1);

  // controlla argomenti
  if (argc < 2) {
    printf("Troppi pochi argomenti\n");
    return 1;
  }

  // imposta segnale SIGINT
  signal(SIGINT, int_handler);

  // configura connessione
  port = atoi(argv[1]);
  if (configure_net() < 0)
    return 1;

  // registrati al server
  int ret = hello();

  // se registrato, inizia a fare richieste
  if (ret >= 0) {
    ret = client_loop();
  }

  // -2: socket rotto, -1: errori sulla connessione
  if (ret > -2) {
    // chiudi la connessione
    printf(" -> [%d]\t: Termino dopo errore chiudendo la connessione\n", port);
    quit();
  } else {
    // segnala di non voler chiudere la connessione
    printf("[%d]\t: Attenzione: termino dopo errore senza chiudere la "
           "connessione\n",
           port);
  }

  // chiudi il socket
  close_net();
}
