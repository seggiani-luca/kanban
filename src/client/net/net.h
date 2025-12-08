#ifndef CLIENT_NET_H
#define CLIENT_NET_H

#include "../../shared/net_const.h"

/*
 * Configura il socket del client connettendolo al server
 */
int configure_net(int port);

/*
 * Chiude la connessione col server
 */
void close_net();

/*
 * Effettua una richiesta al server
 */
void send_net_req(const char* req);

/*
 * Numero di argomenti ricevuti dal server
 */
extern int r_argc;

/*
 * Vettore di argomenti ricevuti dal server
 */
extern const char* r_argv[MAX_NET_ARGS];

/*
 * Riceve una risposta ad una richiesta fatta al server 
 */
void recv_net_res();

#endif
