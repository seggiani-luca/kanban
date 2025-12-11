#include "core.h"
#include "../../shared/command/command.h" // tipo cmd
#include "../net/net.h"                   // gestione di rete client
#include <errno.h>                        // errno, EAGAIN, EWOULDBLOCK
#include <stdio.h>                        // printf
#include <stdlib.h>                       // utilità
#include <string.h>                       // utilità stringa
#include <unistd.h>                       // sleep

// ==== FUNZIONI HELPER ====

/*
 * Helper che risponde ad un pong della lavagna con un ping utente
 */
void reply_ping() {
  printf("[%d]\t: Rispondo al ping della lavagna\n", port);

  // invia ping
  cmd cm = {.type = PONG_LAVAGNA, .args = {"sono sempre in linea"}};
  send_cmd(&cm);
}

/*
 * Helper che:
 * - ottiene una risposta dal server
 * - valuta se la risposta ottenuta combacia con quella attesa
 * - eventualmente restituisce la risposta
 * Eventuali pong dalla lavagna in arrivo vengono gestiti.
 *
 * Il valore di errore qui e nelle funzioni sotto è:
 * -1: se la terminazione è data dal protocollo, e il socket è sempre vivo
 * -2: se si hanno errori di lettura dal socket (non si prova a chiudere la
 * connessione)
 */
int get_rep(cmd_type exp, cmd *got) {
  // gestisci ping finché ne arrivano
  while (1) {
    // ricevi comando
    if (recv_cmd(got, 0) < 0) {
      printf("[%d]\t: Errore di lettura dal socket\n", port);
      return -2;
    }

    // controlla se è ping
    if (got->type != PING_USER) {
      // è un comando vero, prosegui
			break;
    } else {
			reply_ping();
		}

    // ripulisci comando
    memset(got, 0, sizeof(cmd));
  }

  // controlla se la risposta corrisponde
  if (got->type != exp) {
    printf("[%d]\t: Server ha risposto %s, atteso %s\n", port,
           type_to_str(got->type), type_to_str(exp));
    return -1;
  }

  return 0;
}

/*
 * Helper che ottiene l'OK dal server
 */
int get_ack() {
  cmd rep = {0};
  return get_rep(OK, &rep);
}

/*
 * Helper che ottiene la lista utenti dal server (non effettua la richiesta)
 */
int get_user_list(int clients[MAX_CLIENTS], int *num_clients) {
  // ottieni lista client
  cmd cl_push = {0};

	int ret = get_rep(SEND_USER_LIST, &cl_push);
  if (ret < 0) {
    return ret;
	}

  printf("[%d]\t: Ho ottenuto la lista di client: ", port);

  // leggi la lista client
  *num_clients = 0;
  for (int i = 0; i < get_argc(&cl_push); i++) {
    int cl = atoi(cl_push.args[i]);
    if (cl != 0) {
			// riporta client
			clients[(*num_clients)++] = cl;

      printf("%d ", cl);
    }
  }

  printf("\n");
  return 0;
}

// ==== UTILITY ====
int ping_sleep(int wait) {
  for (int i = 0; i < wait; i++) {
    // aspetta un secondo 
    sleep(1);

    // ricevi comando
    cmd push = {0};
    if (recv_cmd(&push, 0) < 0) {
			// se avrebbe bloccato, prosegui 
      if (errno == EAGAIN && errno == EWOULDBLOCK) {
				continue;
			}

			// è un errore vero, esci
      printf("[%d]\t: Errore di lettura dal socket\n", port);
      return -2;
    }

    // controlla se è ping
    if (push.type != PING_USER) {
			// è un comando vero, non dobbiamo riceverne qui 
      printf("[%d]\t: Comando non di ping inaspettato\n", port);
      return -1;
    } else {
    	reply_ping();
		}
  }

  return 0;
}

// ==== RICHIESTE CLIENT ====

int create_card(const card *c) {
  printf("[%d]\t: Richiedo la creazione di una card al server\n", port);

  // invia la card
  char id_str[6];
  snprintf(id_str, 6, "%d", c->id);

  cmd cm = {
      .type = CREATE_CARD,
      .args = {id_str, "TO_DO", c->desc, "richiedo la creazione di una card"}};
  send_cmd(&cm);

  // attendi ack
  return get_ack();
}

int get_card(card *c, int clients[MAX_CLIENTS], int *num_clients) {
  printf("[%d]\t: Cerco di ottenere una nuova card\n", port);

  // ottieni card
  cmd c_push = {0};

  int ret = get_rep(HANDLE_CARD, &c_push);
  if (ret < 0)
    return ret;

  // leggi card
  c->id = atoi(c_push.args[0]);
  strncpy(c->desc, c_push.args[1], CARD_DESC_LEN - 1);
  c->desc[CARD_DESC_LEN - 1] = '\0';

  printf("[%d]\t: Ho ottenuto la card %d\n", port, c->id);

  // ottieni lista client
  ret = get_user_list(clients, num_clients);
  if (ret < 0)
    return ret;

  // fai l'ack
  printf("[%d]\t: Faccio ack per la card %d\n", port, c->id);

  char id_str[6];
  snprintf(id_str, 6, "%d", c->id);

  cmd ack = {.type = ACK_CARD,
             .args = {id_str, "effettuo l'ack per la card ottenuta"}};
  send_cmd(&ack);

  return 0;
}

int hello() {
  printf("[%d]\t: Richiedo la mia registrazione al server\n", port);

  // richiedi registrazione
  cmd cm = {.type = HELLO, .args = {"richiedo la mia registrazione"}};
  send_cmd(&cm);

  // attendi ack
  return get_ack();
}

int quit() {
  printf("[%d]\t: Richiedo la mia deregistrazione al server\n", port);

  // richiedi deregistrazione
  cmd cm = {.type = QUIT, .args = {"richiedo la mia deregistrazione"}};
  send_cmd(&cm);

  // attendi ack
  return get_ack();
}

int request_user_list(int clients[MAX_CLIENTS], int *num_clients) {
  printf("[%d]\t: Richiedo la lista dei client\n", port);

  // richiedi lista client
  cmd cm = {.type = REQUEST_USER_LIST, .args = {"richiedo la lista dei client"}};
  send_cmd(&cm);

  // ottieni lista client
  return get_user_list(clients, num_clients);
}

int card_done() {
  printf("[%d]\t: Segnalo di aver terminato di processare al server\n", port);

  // segnala di aver terminato
  cmd cm = {.type = CARD_DONE,
            .args = {"ho terminato di processare la mia card"}};
  send_cmd(&cm);

  // attendi ack
  return get_ack();
}
