#include "core.h"
#include "../../shared/cmd/cmd.h" // tipo cmd
#include "../net/net.h"           // gestione di rete client
#include <signal.h>               // sig_atomic_t
#include <stdio.h>                // printf
#include <stdlib.h>               // utilità
#include <string.h>               // utilità stringa
#include <sys/socket.h>           // MSG_DONTWAIT
#include <unistd.h>               // sleep

// ==== FUNZIONI HELPER ====

/*
 * Helper che ottiene una risposta aspettata dal server
 */
int get_rep(cmd_type exp, cmd *got) {
  // ricevi dal server
  int ret = recv_multi(got, BLOCK);
  if (ret < 0) {
    return ret;
  }

  // controlla se la risposta corrisponde
  if (got->type != exp) {
    printf("[%d]\t: Server ha risposto %s, atteso %s\n", port,
           type_to_str(got->type), type_to_str(exp));
    return ERR_PROTOCOL;
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
int get_user_list(unsigned short clients[MAX_CLIENTS], int *num_clients) {
  // ottieni lista client
  cmd cl_push = {0};

  int ret = get_rep(SEND_USER_LIST, &cl_push);
  if (ret < 0) {
    return ret;
  }

  printf("[%d]\t: Ho ottenuto la lista di utenti: ", port);

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

/*
 * Vogliamo che la ping_sleep svegli anche su interruzioni
 */
extern sig_atomic_t stop_flag;

int ping_sleep(int wait) {
  for (int i = 0; i < wait; i++) {
    // esci nel caso di interruzioni
    if (stop_flag) {
      return ERR_PROTOCOL;
    }

    // ricevi dal server
    cmd push;
    int ret = recv_multi(&push, NO_BLOCK);

    // gestisci errori
    if (ret < 0) {
      return ret;
    }

    // controlla di non aver ricevuto comando
    if (ret != 0) {
      printf("[%d]\t : Comando %s inaspettatato durante sleep\n", port,
             type_to_str(push.type));
      return ERR_PROTOCOL;
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
  send_server(&cm);

  // attendi ack
  return get_ack();
}

int get_card(card *c, unsigned short clients[MAX_CLIENTS], int *num_clients) {
  printf("[%d]\t: Cerco di ottenere una nuova card\n", port);

  // ottieni card
  cmd c_push = {0};

  int ret = get_rep(HANDLE_CARD, &c_push);
  if (ret < 0) {
    return ret;
  }

  // leggi card
  c->id = atoi(c_push.args[0]);
  strncpy(c->desc, c_push.args[1], CARD_DESC_LEN - 1);
  c->desc[CARD_DESC_LEN - 1] = '\0';

  printf("[%d]\t: Ho ottenuto la card %d\n", port, c->id);

  // ottieni lista client
  ret = get_user_list(clients, num_clients);
  if (ret < 0) {
    return ret;
  }

  // fai l'ack
  char id_str[6];
  snprintf(id_str, 6, "%d", c->id);

  cmd ack = {.type = ACK_CARD,
             .args = {id_str, "effettuo l'ack per la card ottenuta"}};
  send_server(&ack);

  return 0;
}

int hello() {
  printf("[%d]\t: Richiedo la mia registrazione al server\n", port);

  // richiedi registrazione
  cmd cm = {.type = HELLO, .args = {"richiedo la mia registrazione"}};
  send_server(&cm);

  // attendi ack
  return get_ack();
}

int quit() {
  printf("[%d]\t: Richiedo la mia deregistrazione al server\n", port);

  // richiedi deregistrazione
  cmd cm = {.type = QUIT, .args = {"richiedo la mia deregistrazione"}};
  send_server(&cm);

  // attendi ack
  return get_ack();
}

int request_user_list(unsigned short clients[MAX_CLIENTS], int *num_clients) {
  printf("[%d]\t: Richiedo la lista dei client\n", port);

  // richiedi lista client
  cmd cm = {.type = REQUEST_USER_LIST,
            .args = {"richiedo la lista dei client"}};
  send_server(&cm);

  // ottieni lista client
  return get_user_list(clients, num_clients);
}

int card_done() {
  printf("[%d]\t: Segnalo di aver terminato di processare al server\n", port);

  // segnala di aver terminato
  cmd cm = {.type = CARD_DONE,
            .args = {"ho terminato di processare la mia card"}};
  send_server(&cm);

  // attendi ack
  return get_ack();
}
