#ifndef CLIENT_NET_H
#define CLIENT_NET_H

#include "../../shared/command/command.h" // tipo cmd

// ==== GESTIONE CLIENT ====

/*
 * Porta del client
 */
extern int port;

/*
 * Configura il socket del client connettendolo al server
 */
int configure_net();

/*
 * Chiude la connessione col server
 */
void close_net();

// ==== TRASMISSIONE ====

/*
 * Effettua una richiesta al server
 */
void send_cmd(const cmd *cm);

/*
 * Riceve una risposta ad una richiesta fatta al server
 */
int recv_cmd(cmd *cm, int flags);

#endif
