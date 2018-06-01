PROJECT = 09
SYSTEM = linux
SRCDIR = src
BINDIR = bin
CC = gcc
CFLAGS = -O2 -pedantic -Wall

all: cipher_ecb cipher_cbc cipher_bio digest mac client server \
     client_mod server_mod rsa client_rsa server_rsa

clean:
	@rm -rf $(BINDIR)/*

distrib: clean
	tar -cvf $(SYSTEM)-$(PROJECT).tar $(SRCDIR)/* Makefile

cipher_ecb: cipher_ecb.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -lcrypto -o $(BINDIR)/$@

cipher_ecb.o: $(SRCDIR)/cipher_ecb.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

cipher_cbc: cipher_cbc.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -lcrypto -o $(BINDIR)/$@

cipher_cbc.o: $(SRCDIR)/cipher_cbc.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

cipher_bio: cipher_bio.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -lcrypto -o $(BINDIR)/$@

cipher_bio.o: $(SRCDIR)/cipher_bio.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

digest: digest.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -lcrypto -o $(BINDIR)/$@

digest.o: $(SRCDIR)/digest.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

mac: mac.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -lssl -o $(BINDIR)/$@

mac.o: $(SRCDIR)/mac.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

client: client.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -o $(BINDIR)/$@

client.o: $(SRCDIR)/client.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

server: server.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -o $(BINDIR)/$@

server.o: $(SRCDIR)/server.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

client_mod: client_mod.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -lcrypto -lssl -o $(BINDIR)/$@

client_mod.o: $(SRCDIR)/client_mod.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

server_mod: server_mod.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -lcrypto -lssl -o $(BINDIR)/$@

server_mod.o: $(SRCDIR)/server_mod.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

rsa: rsa.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -lcrypto -o $(BINDIR)/$@

rsa.o: $(SRCDIR)/rsa.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

client_rsa: client_rsa.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -lcrypto -o $(BINDIR)/$@

client_rsa.o: $(SRCDIR)/client_rsa.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

server_rsa: server_rsa.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -lcrypto -o $(BINDIR)/$@

server_rsa.o: $(SRCDIR)/server_rsa.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o
