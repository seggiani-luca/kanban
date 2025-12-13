#include "cons/cons.h" // gestione console
#include "net/net.h"   // gestione di rete server
#include <signal.h>    // segnali

/*
 * Flag di arresto
 */
volatile sig_atomic_t stop_flag = 0;

/*
 * Handler predisposto alla gestione di SIGINT per la chisura pulita della
 * connessione
 */
void int_handler(int sig __attribute__((unused))) { stop_flag = 1; }

/*
 * Helper che mostra l'interfaccia della lavagna
 */
void mostra() {
  mostra_stats();
  mostra_shell();
}

int main() {
  // imposta gestore segnale SIGINT
  signal(SIGINT, int_handler);

  // configura il socket di ascolto
  if (configure_net()) {
    return 1;
  }

  // mostra la lavagna all'avvio
  mostra();

  // mettiti in ascolto di richieste
  while (!stop_flag) {
    if (listen_net()) {
      // mostra la lavagna su aggiornamenti
      mostra();
    }
  }

  close_net();
  return 0;
}
