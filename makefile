

```makefile
# Makefile for newshell

CC = gcc
CFLAGS = -Wall -g
TARGET = newshell
SRC = shell.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
