#include "cons.h"
#include <stdlib.h>

/*
 * Dimensione massima buffer letto da console
 */
#define CONS_BUF_SIZE 100

/*
 * Stringa che provoca la chisura della shell
 */
#define QUIT_TOK "BYE"

void recv_cons_cmd(client_id* cl, int* argc, char* argv[MAX_CONS_ARGS]) {
	// ottieni stringa
	printf("[kanban]$ ");
	static char buf[CONS_BUF_SIZE]; // statico, vive per sempre
	fgets(buf, sizeof(buf), stdin);
	buf[strcspn(buf, "\n")] = '\0';

	// tokenizza la stringa
	*argc = -1;
	char* token = strtok(buf, " ");
	while(token && *argc < MAX_CONS_ARGS + 1) {
		if(*argc == -1) {
			// hook per uscire
			if(strcmp(token, QUIT_TOK) == 0) {
				printf("Bye!\n");
				exit(0);
			}

			// il primo token è l'id del client
			*cl = atoi(token);
			(*argc)++;
		} else {
			// i successivi token sono argomenti
			argv[(*argc)++] = token;
		}

  	token = strtok(NULL, " ");
  }
}

void reply_cons_cmd(client_id cl, int argc, char* argv[]) {
	printf("[%d]-> ", cl);

	for(int i = 0; i < argc; i++) {
		printf("%s ", argv[i]);
	}

	printf("\n");
}
