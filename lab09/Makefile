all: server client

debug: ott_server.c ott_client.c
	gcc -o server ott_server.c -lpthread -D_REENTRANT -DDEBUG
	gcc -o client ott_client.c -lpthread -D_REENTRANT -DDEBUG

server: ott_server.c
	gcc -o server ott_server.c -lpthread -D_REENTRANT

client: ott_client.c
	gcc -o client ott_client.c -lpthread -D_REENTRANT

clean:
	rm -f server client