all: bingo_server.c bingo_client.c
	gcc bingo_server.c -D_REENTRANT -o server -lpthread
	gcc bingo_client.c -D_REENTRANT -o client -lpthread