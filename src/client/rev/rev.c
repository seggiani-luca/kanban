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
  cmd ok = {.type = REVIEW_CARD,
            .args = {"chiedo approvazione per la mia card"}};
  cmd rep;

  // effettuiamo richieste periodicamente finché non otteniamo risposta
  while (1) {
    // effettua richiesta
    send_peer(who, &ok);

    // ricevi fino al timeout
    int time = 0;
    while (time < REVIEW_TIMEOUT) {
      // ricevi risposta
      int res = recv_multi(&rep, NO_BLOCK);

      // non c'è stata risposta, incrementa timer
      if (res == 0) {
        time++;
        continue;
      }

      // c'è stato errore
      if (res < 0) {
        return -1;
      }

      // c'è stata risposta
      if (res > 0) {
        // del tipo aspettato?
        if (rep.type != ACK_REVIEW_CARD) {
          printf("[%d]\t: Risposta inaspettata a richiesta di valutazione\n",
                 port);
          return -1;
        }

				// restituisci la risposta
        return atoi(rep.args[0]);
      }
    }
  }
}
