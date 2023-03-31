CC = gcc
objects = client server

all: $(objects)


client: client.c
	$(CC) -o client client.c -g


server: server.c
	$(CC) server.c -o server -lpthread -g
