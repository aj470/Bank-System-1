CCOMPILER = gcc
CFLAGS = -Wall -Werror
CTHREAD = -lpthread

all: server client

server: server.c
	$(CCOMPILER) $(CFLAGS) $(CTHREAD) -o server server.c

client: client.c
	$(CCOMPILER) $(CFLAGS) $(CTHREAD) -o client client.c

clean:
	rm -f server
	rm -f client