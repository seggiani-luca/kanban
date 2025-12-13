#ifndef NET_CONST_H
#define NET_CONST_H

#include <time.h> // timeval

// ==== COSTANTI SERVER ====

/*
 * Dimensione backlog socket di ascolto
 */
#define BACKLOG 10

/*
 * L'indirizzo del server
 */
#define SERVER_ADDR "127.0.0.1"

/*
 * La porta del server
 */
#define SERVER_PORT 5678

// ==== COSTANTI CLIENT ====

/*
 * L'indirizzo dei client
 */
#define CLIENT_ADDR "127.0.0.1"

/*
 * La porta minima (inclusa) dei client
 */
#define CLIENT_MIN_PORT (SERVER_PORT + 1)

/*
 * La porta massima (esclusa) dei client
 */
#define CLIENT_MAX_PORT (CLIENT_MIN_PORT + MAX_CLIENTS)

#endif
