#include "rev.h"
#include "../net/net.h" // gestione di rete client
#include "stdlib.h"     // utilità
#include <stdio.h>      // printf

void handle_rev(unsigned short who) {
  printf("[%d]\t: Ricevuta richiesta valutazione da peer %d, rispondo\n", port,
         who);

  // rispondi
  cmd ok = {.type = ACK_REVIEW_CARD, .args = {"1", "approvo la tua card"}};
  send_peer(who, &ok);
}

int req_review(unsigned short who) {
  // effettua richiesta
  cmd ok = {.type = REVIEW_CARD,
            .args = {"chiedo approvazione per la mia card"}};
  send_peer(who, &ok);

  // ricevi risposta
  cmd rep;
  if (recv_multi(&rep, BLOCK) < 0)
    return -1;

  if (rep.type != ACK_REVIEW_CARD) {
    printf("[%d]\t: Risposta inaspettata a richiesta di valutazione\n", port);
    return -1;
  }

  return atoi(rep.args[0]);
}
