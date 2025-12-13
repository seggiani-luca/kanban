#include "log.h"
#include <stdarg.h> // va_list
#include <stdio.h>  // printf

/*
 * Dimensione di un evento
 */
#define LOG_SIZE 256

/*
 * Buffer circolare di eventi
 */
char logbuf[LOG_ENTRIES][LOG_SIZE];

/*
 * Indice dell'evento corrente
 */
int logidx = 0;

void log_event(const char *fmt, ...) {
  // stampa gli argomenti forniti sul prossimo log
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(logbuf[logidx], LOG_SIZE, fmt, ap);
  va_end(ap);

  // avanza al prossimo evento
  logidx = (logidx + 1) % LOG_ENTRIES;
}

void dump_logs() {
  // stampa tutti gli eventi nel buffer a partire da quello corrente
  for (int i = 0; i < LOG_ENTRIES; i++) {
    int idx = (logidx + i) % LOG_ENTRIES;

    if (logbuf[idx][0]) {
      printf("%s", logbuf[idx]);
    } else {
      // se non c'è un evento, stampa spazio bianco per mantenere il layout
      printf("\n");
    }
  }
}
