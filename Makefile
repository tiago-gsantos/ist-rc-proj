CC = gcc
CFLAGS = -Wall -g
TARGET = player
SRCS = player.c parser_player.c commands.c
OBJS = $(SRCS:.c=.o) *.txt

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: all clean
