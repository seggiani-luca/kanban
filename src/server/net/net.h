#ifndef SERVER_NET_H
#define SERVER_NET_H

#include "../../shared/cmd/cmd.h" // tipo cmd
#include "../core/core.h"         // logica server

// ==== GESTIONE SERVER ====

/*
 * Configura il modulo di rete, inizializzando il socket di ascolto
 */
int configure_net();

/*
 * Si mette in ascolto per nuove connessioni sul socket di ascolto e richieste
 * dai client connessi. Restituisce 1 se c'è stato un aggiornamento di stato che
 * richiede di ridesgnare l'interfaccia
 */
int listen_net();

/*
 * Chiude il socket di ascolto e le connessioni attive coi client
 */
void close_net();

// ==== TRASMISSIONE ====

/*
 * Invia un comando ad un client
 */
void send_client(client_id cl_id, const cmd *cm);

#endif
