CC = gcc
CFLAGS = -Wall -Wextra -g
TARGET = ttt
SRC = ttt.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -f $(TARGET)
