CC = gcc
CFLAGS = -Wall -pthread -I./src/modules

SRC_DIR = src
MOD_DIR = $(SRC_DIR)/modules

SRCS = $(SRC_DIR)/main.c \
       $(MOD_DIR)/threadpool.c \
       $(MOD_DIR)/sharedbuffer.c

OBJS = $(SRCS:.c=.o)

TARGET = threadpool_test

all: $(TARGET)


$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $(SRCS)


clean:
	rm -f $(TARGET) $(SRC_DIR)/*.o $(MOD_DIR)/*.o

