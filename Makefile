CC= gcc
Deps= malloc.c
Output= main

all:
	$(CC) $(Deps) -o $(Output)