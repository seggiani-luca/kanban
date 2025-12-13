#ifndef CLIENT_NET_H
#define CLIENT_NET_H

#include "../../shared/cmd/cmd.h" // tipo cmd

#define ERR_PROTOCOL -1
#define ERR_TCP_SOCKET -2
#define ERR_UDP_SOCKET -3

#define BLOCK 1
#define NO_BLOCK 0

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
 * Invia un comando al server
 */
void send_server(const cmd *cm);

/*
 * Invia un comando ad un peer
 */
void send_peer(unsigned short who, const cmd *cm);

/*
 * Gestisce comandi PEER e WATCH finché non si ha un comando CORE, e lo
 * restituisce
 */
int recv_multi(cmd *cm, int block);

#endif
