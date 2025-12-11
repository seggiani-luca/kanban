#ifndef LOG_H
#define LOG_H

/*
 * Il numero di eventi da mantenere in log
 */
#define LOG_ENTRIES 10

/*
 * Usato per registrare un evento
 */
void log_event(const char *fmt, ...);

/*
 * Usato per stampare gli ultimi LOG_ENTRIES eventi
 */
void dump_logs();

#endif
