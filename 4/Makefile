CC = gcc
CFLAGS = -Wall -Wextra -pthread -g -I./src/modules -I./src/server
LDFLAGS = -pthread

SRC_DIR = src
MOD_DIR = $(SRC_DIR)/modules
SRV_DIR = $(SRC_DIR)/server

OBJ_DIR = obj
BIN_DIR = bin

# Objetos dos módulos comuns
MOD_OBJS = \
    $(OBJ_DIR)/threadpool.o \
    $(OBJ_DIR)/sharedbuffer.o

# Objetos do servidor
SERVER_OBJS = \
    $(OBJ_DIR)/server.o \
    $(OBJ_DIR)/log.o \
    $(OBJ_DIR)/sockets.o \
    $(OBJ_DIR)/ioaux.o \
    $(OBJ_DIR)/erroraux.o \
    $(MOD_OBJS)

# Objetos do teste do thread pool
TEST_OBJS = \
    $(OBJ_DIR)/main.o \
    $(MOD_OBJS)

SERVER_BIN = $(BIN_DIR)/server
TEST_BIN = $(BIN_DIR)/threadpool_test

all: $(SERVER_BIN) $(TEST_BIN)

$(SERVER_BIN): $(SERVER_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(TEST_BIN): $(TEST_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Regra geral para compilar objetos
$(OBJ_DIR)/%.o: $(SRC_DIR)/modules/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/server/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)