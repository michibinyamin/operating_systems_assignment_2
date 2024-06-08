CC = gcc
CFLAGS = -Wall -Wextra -g
TARGET1 = mync
TARGET2 = ttt
SRC1 = mync.c
SRC2 = ttt.c

all: $(TARGET1) $(TARGET2)

$(TARGET1): $(SRC1)
	$(CC) $(CFLAGS) -o $@ $<

$(TARGET2): $(SRC2)
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -f $(TARGET1) $(TARGET2)
