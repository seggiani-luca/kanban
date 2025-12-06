#ifndef SERVER_CONS_H
#define SERVER_CONS_H

#include "../core/core.h"

/*
 * Numero massimo di argomenti leggibili da console
 */
#define MAX_CONS_ARGS 10

/*
 * Ottiene il numero di client e una lista di argomenti corrispondenti ad un
 * comando da console
 */
void recv_cons_cmd(client_id* cl, int* argc, char* argv[MAX_CONS_ARGS]);

/*
 * Risponde ad una richiesta di un client, stampando la risposta su console
 */
void reply_cons_cmd(client_id cl, int argc, const char* argv[]);

#endif
