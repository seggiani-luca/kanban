#ifndef NET_CONST_H
#define NET_CONST_H

// ==== COSTANTI RETE ====

/*
 * Dimensione massima buffer letto da socket
 */
#define NET_BUF_SIZE 500

/*
 * Numero massimo di argomenti leggibili da socket
 */
#define MAX_NET_ARGS 20

/*
 * Dimensione backlog socket di ascolto
 */
#define BACKLOG 10

/*
 * L'indirizzo del server
 */
#define SERVER_ADDR "127.0.0.1"

/*
 * L'indirizzo dei client
 */
#define CLIENT_ADDR "127.0.0.1"

/*
 * La porta del server
 */
#define SERVER_PORT 5678

/*
 * La porta minima (inclusa) dei client
 */
#define CLIENT_MIN_PORT (SERVER_PORT + 1)

/*
 * La porta massima (esclusa) dei client
 */
#define CLIENT_MAX_PORT (CLIENT_MIN_PORT + MAX_CLIENTS)

#endif
