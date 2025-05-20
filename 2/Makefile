CC = gcc
CFLAGS = -Wall -pthread
SRC = src/main.c src/modules/accounts.c src/modules/transfer.c
OBJ = $(SRC:.c=.o)
TARGET = transferencia

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

