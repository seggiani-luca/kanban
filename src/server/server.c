#include "cons/cons.h" // console
#include "net/net.h"   // gestione di rete server
#include <signal.h>    // segnali
#include <stdlib.h>    // exit

/*
 * Handler predisposto alla gestione di SIGINT per la chisura pulita della
 * connessione
 */
void int_handler() {
  mostra_interfaccia();
  close_net();
  exit(0);
}

int main() {
  // imposta segnale SIGINT
  signal(SIGINT, int_handler);

  // configura il socket di ascolto
  if (configure_net())
    return 1;

  // mostra la lavagna all'avvio
  mostra_interfaccia();
  mostra_shell();

  // mettiti in ascolto di richieste
  while (1) {
    if (listen_net()) {
      // mostra la lavagna su aggiornamenti
      mostra_interfaccia();
      mostra_shell();
    }
  }

  return 0;
}
