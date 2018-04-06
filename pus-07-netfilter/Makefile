PROJECT = 07
SYSTEM = linux
SRCDIR = src
BINDIR = bin
CC = gcc
CFLAGS = -O2 -Wall -pedantic

all: queue queuemod iptc iptcmod

clean:
	@rm -rf $(BINDIR)/*

distrib: clean
	tar -cvf $(SYSTEM)-$(PROJECT).tar $(SRCDIR)/* Makefile

queue: queue.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -lnetfilter_queue -o $(BINDIR)/$@

queue.o: $(SRCDIR)/queue.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

queuemod: queuemod.o libqueue.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o $(BINDIR)/libqueue.o -lnetfilter_queue -o $(BINDIR)/$@

queuemod.o: $(SRCDIR)/queuemod.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

libqueue.o: $(SRCDIR)/libqueue.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

iptc: iptc.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -lip4tc -o $(BINDIR)/$@

iptc.o: $(SRCDIR)/iptc.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

iptcmod: iptcmod.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -lip4tc -o $(BINDIR)/$@

iptcmod.o: $(SRCDIR)/iptcmod.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o
