# Makefile for calc project with math and readline libraries

CC = gcc
CFLAGS = -g -Wall -Wextra -Werror -std=c17
LDFLAGS = -lm -lreadline
TARGET = calc

SRC = main.c my_math.c stack_printing.c stack.c globals.c my_statistics.c words.c help_functions.c registers.c util_func.c tab_complete.c tokenizer.c interpreter.c
OBJ = $(SRC:.c=.o)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJ)
