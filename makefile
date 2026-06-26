CFLAGSDEBUG = -Wall -Og -g3

all: server client

# I prerequisiti vanno sulla stessa riga!
debug: server_debug client_debug

server:
	gcc src/server/*.c src/common/*.c -o server -lcrypto

client:
	gcc src/client/*.c src/common/*.c -o client -lcrypto

server_debug:
	gcc src/server/*.c src/common/*.c $(CFLAGSDEBUG) -o server

client_debug:
	gcc src/client/*.c src/common/*.c $(CFLAGSDEBUG) -o client

# Rimosso server e client da PHONY per ripristinare il check dei timestamp
.PHONY: all debug
