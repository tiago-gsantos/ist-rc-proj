CC = gcc
CFLAGS = -Wall -g
TARGET = GS
SRCS = GS.c parser_server.c commands.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
	rm -rf ./GAMES/* ./SCORES/*

.PHONY: all clean
