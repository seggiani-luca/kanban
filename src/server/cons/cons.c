#include "cons.h"
#include "../core/core.h" // logica server
#include "../log/log.h"   // logs
#include <stdio.h>        // printf
#include <stdlib.h>       // system (per clear)
#include <string.h>       // utilità stringa

#define INTERF_WIDTH 96

/*
 * Helper per stampare intestazioni formattate
 */
void print_sep(const char *s, int width) {
  // stampa stringa
  printf("%s ", s);

  // caratteri di padding (trattini)
  int len = strlen(s) + 2;
  int pad = width - len;
  for (int j = 0; j < pad; j++) {
    putchar('-');
  }

  // chiudi con spazio
  putchar(' ');
}

void mostra_stats() {
  // presentazioni
  system("clear");
  printf("Kanban server v0.0 - Luca Seggiani\n");

  // intestazione lavagna
  for (int i = 0; i < NUM_COLS; i++) {
    const char *s = col_to_str(i);
    print_sep(s, INTERF_WIDTH / 3);
  }
  printf("\n");

  // lavagna
  show_lavagna();

  // client
  print_sep("CLIENTS", INTERF_WIDTH);
  printf("\n");
  show_clients();

  // log
  print_sep("LOG", INTERF_WIDTH);
  printf("\n");
  dump_logs();
}

void mostra_shell() {
  // shell
  print_sep("SHELL", INTERF_WIDTH);
  printf("\n$ ");
  fflush(stdout);
}
