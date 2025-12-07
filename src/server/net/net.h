#ifndef SERVER_NET_H
#define SERVER_NET_H

#include "../core/core.h"

/*
 * Numero massimo di argomenti leggibili da socket
 */
#define MAX_NET_ARGS 10

/*
 * Dimensione backlog socket di ascolto
 */
#define BACKLOG 10

/*
 * Configura il modulo di rete, inizializzando il socket di ascolto
 */
int configure_net();

/*
 * Si mette in ascolto per nuove connessioni sul socket di ascolto e richieste
 * dai client connessi
 */
void listen_net();

/*
 * Risponde ad una richiesta di un client, stampando la risposta su console
 */
void reply_net_cmd(client_id cl, int argc, const char* argv[]);

#endif
