#include "log.h"
#include <stdarg.h>	// va_list
#include <stdio.h>	// printf

/*
 * Dimensione di un evento
 */
#define LOG_SIZE 256

/*
 * Array di eventi
 */
char logbuf[LOG_ENTRIES][LOG_SIZE];

/*
 * Indice dell'evento corrente
 */
int logidx = 0;

void log_event(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(logbuf[logidx], LOG_SIZE, fmt, ap);
	va_end(ap);

	logidx = (logidx + 1) % LOG_ENTRIES;
}

void dump_logs() {
	for (int i = 0; i < LOG_ENTRIES; i++) {
		int idx = (logidx + i) % LOG_ENTRIES;
		if (logbuf[idx][0]) {
			printf("%s", logbuf[idx]);
		} else {
			printf("\n");
		}
	}
}
