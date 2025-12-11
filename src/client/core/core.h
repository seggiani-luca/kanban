#ifndef CLIENT_CORE_H
#define CLIENT_CORE_H

#include "../../shared/card/card.h"  // tipo card
#include "../../shared/core_const.h" // costanti core

// ==== UTILITY ====

/*
 * Sospende per il tempo specificato, restando in ascolto dei ping dal server
 */
int ping_sleep(int wait);

// ==== RICHIESTE CLIENT ====

/*
 * Richiede la creazione di una card
 */
int create_card(const card *c);

/*
 * Ottiene una nuova card da processare
 */
int get_card(card *c, int clients[MAX_CLIENTS], int *num_clients);

/*
 * Si registra al server
 */
int hello();

/*
 * Si deregistra dal server
 */
int quit();

/*
 * Richiede la lista di utenti
 */
int request_user_list(int clients[MAX_CLIENTS], int *num_clients);

/*
 * Segnala il fine della processazione di una card
 */
int card_done();

#endif
