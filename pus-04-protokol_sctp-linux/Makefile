CMakeLists.txtPROJECT = 04
SYSTEM = linux
SRCDIR = src
BINDIR = bin
CC = gcc
CFLAGS = -O2 -pedantic -Wall

all: client1 server1 client2 server2 client3 server3

clean:
	@rm -rf $(BINDIR)/*

distrib: clean
	tar -cvf $(SYSTEM)-$(PROJECT).tar $(SRCDIR)/* Makefile

client1: client1.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -o $(BINDIR)/$@

client1.o: $(SRCDIR)/client1.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

server1: server1.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -o $(BINDIR)/$@

server1.o: $(SRCDIR)/server1.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

# -L/usr/local/lib

client2: client2.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -o $(BINDIR)/$@ -lsctp

client2.o: $(SRCDIR)/client2.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o 

server2: server2.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -o $(BINDIR)/$@ -lsctp

server2.o: $(SRCDIR)/server2.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

client3: client3.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -o $(BINDIR)/$@ -lsctp

client3.o: $(SRCDIR)/client3.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

server3: server3.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -o $(BINDIR)/$@ -lsctp

server3.o: $(SRCDIR)/server3.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

