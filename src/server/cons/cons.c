#include "cons.h"
#include "../core/core.h"	// logica del server
#include "../core/core.h"	// logica del server
#include "../log/log.h"		// logs
#include <stdlib.h>				// system
#include <stdio.h>				// printf
#include <string.h>				// utilità string

/*
 * Helper per stampare intestazioni formattate
 */
void print_sep(const char* s, int w){
	int len = strlen(s) + 2;
	int pad = w - len;
	printf("%s ", s);
	for (int j = 0; j < pad; j++) putchar('-');
	putchar(' ');
}

void mostra_interfaccia() {
	system("clear");
	
	printf("Kanban server v0.0 - Luca Seggiani\n");
	
	// intestazione lavagna	
	for (int i = 0; i < NUM_COLS; i++) {
    const char* s = ctoa(i);
		print_sep(s, 32);
	}
	printf("\n");

	// lavagna
	show_lavagna();
	
	// client
	print_sep("CLIENTS", 96);
	printf("\n");
	show_clients();

	// log
	print_sep("LOG", 96);
	printf("\n");
	dump_logs();
}

void mostra_shell() {
	// shell
	print_sep("SHELL", 96);
	printf("\n$ ");
	fflush(stdout);
}


