#include "watch.h"
#include "../../shared/core_const.h" // costanti core
#include "../log/log.h"              // logs
#include "../core/core_watch.h" // interfaccia specifica per modulo watchdog
#include "../net/net.h"              // gestione di rete server
#include <stdio.h>                   // printf

int tick_timer(client_id cl_id) {
  // ottieni client
  client *cl = find_client(cl_id);
  if (cl == NULL) {
    return 0; // non ancora registrato
  }

  // ottieni il tempo corrente
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);

  if (cl->sts == BUSY && now.tv_sec > cl->deadline.tv_sec) {
    // timer scattato
    if (cl->sent_pong) {
      log_event("[kanban]\t: Timeout controllo scattato su %d\n", cl_id);

      // timeout, per gentilezza avverti
      cmd err = {.type = ERR, .args = {"fallito a rispondere al PONG"}};
      send_client(cl_id, &err);

      // deregistra
      unregister_client(cl);

      return -1;
    } else {
      log_event("[kanban]\t: Controllo scattato su %d\n", cl_id);

      // fine attesa, invia pong
      cmd pong = {.type = PING_USER, .args = {"è sempre in linea?"}};
      send_client(cl_id, &pong);

      // aggiorna watchdog
      set_timer(cl, WATCHDOG_TIMEOUT);
      cl->sent_pong = 1;

      return 1;
    }
  }

  return 0;
}

void handle_pong(client_id cl_id) {
  // ottieni client
  client *cl = find_client(cl_id);
  if (cl == NULL) {
    printf("[kanban]\t: Ricevuto PONG da client %d non registrato", cl->id);
		return;
  }
  
	// controlla se si aspettava PONG
	if (!cl->sent_pong) {
    printf("[kanban]\t: Ricevuto PONG da client %d a cui non si è mai inviato PING", cl->id);
		return;
  }

  // ok, è in linea
  cl->sent_pong = 0;

  // se è sempre busy, attendi di nuovo
  if (cl->sts == BUSY) {
    set_timer(cl, WATCHDOG_WAIT);
  }
}
