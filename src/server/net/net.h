#ifndef SERVER_NET_H
#define SERVER_NET_H

#include "../core/core.h"

/*
 * Numero massimo di argomenti leggibili da socket
 */
#define MAX_NET_ARGS 10

/*
 * Ottiene il numero di client e una lista di argomenti corrispondenti ad un
 * comando da console
 */
void recv_net_cmd(client_id* cl, int* argc, char* argv[MAX_NET_ARGS]);

/*
 * Risponde ad una richiesta di un client, stampando la risposta su console
 */
void reply_net_cmd(client_id cl, int argc, const char* argv[]);

#endif
