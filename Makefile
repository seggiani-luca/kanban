SRC := src

# ==== SHARED SOURCES =====
SHARED_SRC := $(SRC)/shared
SHARED_SRCS := $(shell find $(SHARED_SRC) -type f -name '*.c')

# ==== SERVER SOURCES =====
SERVER_SRC := $(SRC)/server
SERVER_SRCS := $(shell find $(SERVER_SRC) -type f -name '*.c')

OUT := out

# ==== SHARED OBJECTS =====
SHARED_OUT := $(OUT)/shared
SHARED_OUTS := $(SHARED_SRCS:$(SHARED_SRC)/%.c=$(SHARED_OUT)/%.o)

# ==== SERVER OBJECTS =====
SERVER_OUT := $(OUT)/server
SERVER_OUTS := $(SERVER_SRCS:$(SERVER_SRC)/%.c=$(SERVER_OUT)/%.o)

SERVER_TARGET := $(OUT)/server_exec 

CC := gcc
CFLAGS := -Wall -Wextra -std=c11
LDFLAGS :=

run_server: $(SERVER_TARGET) 
	@echo -e "=> Running server..."
	@./$(SERVER_TARGET)

$(SERVER_TARGET): $(SERVER_OUTS) $(SHARED_OUTS)
	@echo -e "=> Linking server objects: $^"
	@$(CC) $^ $(LDFLAGS) -o $@
	@echo -e "=> Server executable built in $@"

$(SERVER_OUT)/%.o: $(SERVER_SRC)/%.c
	@echo -e "=> Compiling $<"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

$(SHARED_OUT)/%.o: $(SHARED_SRC)/%.c
	@echo -e "=> Compiling $<"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo -e "=> Cleaning directory $(OUT)"
	@rm -rf $(OUT)
