CFLAGSDEBUG = -Wall -Og -g3 -DDEBUG

all: server client

# I prerequisiti vanno sulla stessa riga!
debug: server_debug client_debug

server:
	gcc src/server/*.c src/common/*.c src/server/database/*.c -o server -lcrypto

client:
	gcc src/client/*.c src/common/*.c src/client/prompts/*.c -o client -lcrypto

server_debug:
	gcc src/server/*.c src/common/*.c src/server/database/*.c $(CFLAGSDEBUG) -o server -lcrypto

client_debug:
	gcc src/client/*.c src/common/*.c src/client/prompts/*.c $(CFLAGSDEBUG) -o client -lcrypto

.PHONY: all debug client server
