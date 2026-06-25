all: server client

CFLAGSDEBUG = -Wall -Og -g3

debug:
	server_debug client_debug

server:
	@gcc src/server/main.c -o server

client:
	@gcc src/client/main.c -o client

server_debug:
	@gcc $(CFLAGSDEBUG) src/server/main.c -o server

client_debug:
	@gcc $(CFLAGSDEBUG) src/client/main.c -o client

.PHONY: all debug server client server_debug client_debug
