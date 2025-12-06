#include "card.h"

const char* col_names[] = {
	"TO_DO",
	"DOING",
	"DONE"
};

col_id atoc(const char* str) {
	for(int i = 0; i < NUM_COLS; i++) {
		if(strcmp(col_names[i], str) == 0) {
			return i;
		}
	}

	// sarebbe errore 
	return TO_DO; // (0)
}

const char* ctoa(col_id id) {
	return col_names[id];
}

void print_card(struct card* c) {
	// passa il timestamp a stringa con strftime
	char tm_buf[64];
	strftime(tm_buf, sizeof(tm_buf), "%Y-%m-%d %H:%M:%S", &c->timestamp);
	
	printf("{id: %d, desc: %s, user: %d, timestamp: %s}", 
			c->id, c->desc, c->user, tm_buf);
}
