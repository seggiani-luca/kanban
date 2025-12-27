# ==== SOURCES ====
SRC := src
SHARED_SRC := $(SRC)/shared
SERVER_SRC := $(SRC)/server
CLIENT_SRC := $(SRC)/client
SHARED_SRCS := $(shell find $(SHARED_SRC) -type f -name '*.c')
SERVER_SRCS := $(shell find $(SERVER_SRC) -type f -name '*.c')
CLIENT_SRCS := $(shell find $(CLIENT_SRC) -type f -name '*.c')

# ==== OBJECTS ====
OUT := out
SHARED_OUT := $(OUT)/shared
SERVER_OUT := $(OUT)/server
CLIENT_OUT := $(OUT)/client
SHARED_OUTS := $(SHARED_SRCS:$(SHARED_SRC)/%.c=$(SHARED_OUT)/%.o)
SERVER_OUTS := $(SERVER_SRCS:$(SERVER_SRC)/%.c=$(SERVER_OUT)/%.o)
CLIENT_OUTS := $(CLIENT_SRCS:$(CLIENT_SRC)/%.c=$(CLIENT_OUT)/%.o)

# ==== TARGETS ====
SERVER_TARGET := lavagna
CLIENT_TARGET := client

# ==== DATA ====
CARD_DATA := dat/cards.txt

# ==== FLAGS ====
CC := gcc
CFLAGS := -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=199309L -g
LDFLAGS :=

run_lavagna: $(SERVER_TARGET)
	@printf "=> Eseguo lavagna...\n"
	@tabs -19
	@cat $(CARD_DATA) - | ./$(SERVER_TARGET)

run_clients: $(CLIENT_TARGET)
	@($(MAKE) --no-print-directory run_client ARGS=5679 & \
		$(MAKE) --no-print-directory run_client ARGS=5680 & \
		$(MAKE) --no-print-directory run_client ARGS=5681 & \
		$(MAKE) --no-print-directory run_client ARGS=5682 & \
		wait)

run_client: $(CLIENT_TARGET)
	@printf "=> Eseguo client con argomento $(ARGS)...\n"
	@./$(CLIENT_TARGET) $(ARGS)

$(SERVER_TARGET): $(SERVER_OUTS) $(SHARED_OUTS)
	@printf "=> Collego oggetti server: $^\n"
	@$(CC) $^ $(LDFLAGS) -o $@ 
	@printf "=> Eseguibile server compilato in $@\n"

$(CLIENT_TARGET): $(CLIENT_OUTS) $(SHARED_OUTS)
	@printf "=> Collego oggetti client: $^\n"
	@$(CC) $^ $(LDFLAGS) -o $@ 
	@printf "=> Eseguibile client compilato in $@\n"

$(SERVER_OUT)/%.o: $(SERVER_SRC)/%.c
	@printf "=> Compilo sorgente server $<\n"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

$(CLIENT_OUT)/%.o: $(CLIENT_SRC)/%.c
	@printf "=> Compilo sorgente client $<\n"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

$(SHARED_OUT)/%.o: $(SHARED_SRC)/%.c
	@printf "=> Compilo sorgente condivisa $<\n"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

format:
	@printf "=> Formatto il sorgente\n"
	@find . \( -name "*.c" -o -name "*.h" \) -type f -exec clang-format -i {} +

clean:
	@printf "=> Ripulisco directory $(OUT)\n"
	@rm -rf $(OUT)
	@printf "=> Ripulisco $(SERVER_TARGET)\n"
	@rm -f $(SERVER_TARGET)
	@printf "=> Ripulisco $(CLIENT_TARGET)\n"
	@rm -f $(CLIENT_TARGET)
