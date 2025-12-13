#ifndef CORE_WATCH_H
#define CORE_WATCH_H

#include "core.h"

// ==== GESTIONE CLIENT ====

/*
 * Rappresenta lo stato del client nella ricezione delle carte
 */
typedef enum {
  IDLE,
  SENT_CARD,
  BUSY,
} client_sts;

/*
 * Rappresenta un client, identificato da:
 * - id (se è 0 il client è nullo)
 * - stato
 * - puntatore alla card che sta gestendo (se è NULL sta aspettando una card)
 * - deadline del timer 
 * - un flag che rappresenta se si sta aspettando un ping dal client
 */
typedef struct {
  client_id id;
  client_sts sts;
  card *handling;
  struct timespec deadline;
  int sent_pong;
} client;

/*
 * Helper che imposta il timer di un client
 */
void set_timer(client *cl, int wait);

/*
 * Trova il client con dato id nel vettore client registrati. Restituisce NULL
 * se non lo trova
 */
client *find_client(client_id cl);

/*
 * Deregistra un client dal vettore client registrati
 */
int unregister_client(client *cl);

#endif
