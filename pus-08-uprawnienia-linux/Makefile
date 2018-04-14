PROJECT = 08
SYSTEM = linux
SRCDIR = src
BINDIR = bin
CC = gcc
CFLAGS = -O2 -Wall -pedantic

all: suidserver rootserver daemon fserver capserver

clean:
	@rm -rf $(BINDIR)/*

distrib: clean
	tar -cvf $(SYSTEM)-$(PROJECT).tar $(SRCDIR)/* Makefile

suidserver: suidserver.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -o $(BINDIR)/$@

suidserver.o: $(SRCDIR)/suidserver.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

rootserver: rootserver.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -o $(BINDIR)/$@

rootserver.o: $(SRCDIR)/rootserver.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

daemon: daemon.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -o $(BINDIR)/$@

daemon.o: $(SRCDIR)/daemon.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

fserver: fserver.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -o $(BINDIR)/$@

fserver.o: $(SRCDIR)/fserver.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

capserver: capserver.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -lcap -o $(BINDIR)/$@

capserver.o: $(SRCDIR)/capserver.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o
