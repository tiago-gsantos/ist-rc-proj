CC = gcc
CFLAGS = -Wall
TARGET = player
SRCS = player.c


all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)


clean:
	rm -f $(TARGET)


.PHONY: all clean
