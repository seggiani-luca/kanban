SRC := src

# ==== SHARED SOURCES =====
SHARED_SRC := $(SRC)/shared
SHARED_SRCS := $(shell find $(SHARED_SRC) -type f -name '*.c')

# ==== SERVER SOURCES =====
SERVER_SRC := $(SRC)/server
SERVER_SRCS := $(shell find $(SERVER_SRC) -type f -name '*.c')

# ==== CLIENT SOURCES =====
CLIENT_SRC := $(SRC)/client
CLIENT_SRCS := $(shell find $(CLIENT_SRC) -type f -name '*.c')

OUT := out

# ==== SHARED OBJECTS =====
SHARED_OUT := $(OUT)/shared
SHARED_OUTS := $(SHARED_SRCS:$(SHARED_SRC)/%.c=$(SHARED_OUT)/%.o)

# ==== SERVER OBJECTS =====
SERVER_OUT := $(OUT)/server
SERVER_OUTS := $(SERVER_SRCS:$(SERVER_SRC)/%.c=$(SERVER_OUT)/%.o)

# ==== CLIENT OBJECTS =====
CLIENT_OUT := $(OUT)/client
CLIENT_OUTS := $(CLIENT_SRCS:$(CLIENT_SRC)/%.c=$(CLIENT_OUT)/%.o)

SERVER_TARGET := $(OUT)/server_exec 
CLIENT_TARGET := $(OUT)/client_exec 

CC := gcc
CFLAGS := -Wall -Wextra -std=c11
LDFLAGS :=

run_server: server
	@echo -e "=> Eseguo server...\n"
	@./$(SERVER_TARGET)

run_clients: client
	@$(MAKE) --no-print-directory run_client ARGS=5679 &
	@$(MAKE) --no-print-directory run_client ARGS=5680 &
	@$(MAKE) --no-print-directory run_client ARGS=5681 &
	@$(MAKE) --no-print-directory run_client ARGS=5682 &
	wait

run_client: client 
	@echo -e "=> Eseguo client con argomento $(ARGS)...\n"
	@./$(CLIENT_TARGET) $(ARGS)

server: $(SERVER_TARGET)

client: $(CLIENT_TARGET)

$(SERVER_TARGET): $(SERVER_OUTS) $(SHARED_OUTS)
	@echo -e "=> Inizio linking oggetti server: $^"
	@$(CC) $^ $(LDFLAGS) -o $@ 
	@echo -e "=> Eseguibile server compilato in $@"

$(CLIENT_TARGET): $(CLIENT_OUTS) $(SHARED_OUTS)
	@echo -e "=> Inizio linking oggetti client: $^"
	@$(CC) $^ $(LDFLAGS) -o $@ 
	@echo -e "=> Eseguibile client compilato in $@"

$(SERVER_OUT)/%.o: $(SERVER_SRC)/%.c
	@echo -e "=> Compilo sorgente server $<"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

$(CLIENT_OUT)/%.o: $(CLIENT_SRC)/%.c
	@echo -e "=> Compilo sorgente client $<"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

$(SHARED_OUT)/%.o: $(SHARED_SRC)/%.c
	@echo -e "=> Compilo sorgente condivisa $<"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo -e "=> Ripulisco directory $(OUT)"
	@rm -rf $(OUT)
