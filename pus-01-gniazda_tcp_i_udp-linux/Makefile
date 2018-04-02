PROJECT = 01
SYSTEM = linux
SRCDIR = src
BINDIR = bin
CC = gcc
CFLAGS = -O2 -pedantic -Wall -lpthread

all: client1 server1 client2 server2 client3 server3 server4 server5

clean:
	@rm -rf $(BINDIR)/*.o \
	$(BINDIR)/client1 $(BINDIR)/server1 \
	$(BINDIR)/client2 $(BINDIR)/server2 \
	$(BINDIR)/client3 $(BINDIR)/server3 \
	$(BINDIR)/server4 $(BINDIR)/server5

distrib: clean
	tar -cvf $(SYSTEM)-$(PROJECT).tar $(SRCDIR)/* $(BINDIR)/img Makefile

client1: client1.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -o $(BINDIR)/$@

client1.o: $(SRCDIR)/client1.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

client2: client2.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -o $(BINDIR)/$@

client2.o: $(SRCDIR)/client2.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

client3: client3.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -o $(BINDIR)/$@

client3.o: $(SRCDIR)/client3.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

server1: server1.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -o $(BINDIR)/$@

server1.o: $(SRCDIR)/server1.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

server2: server2.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -o $(BINDIR)/$@

server2.o: $(SRCDIR)/server2.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

server3: server3.o libpalindrome.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o $(BINDIR)/libpalindrome.o -o $(BINDIR)/$@

server3.o: $(SRCDIR)/server3.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

server4: server4.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -o $(BINDIR)/$@

server4.o: $(SRCDIR)/server4.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

server5: server5.o
	$(CC) $(CFLAGS) -pthread $(BINDIR)/$@.o -o $(BINDIR)/$@

server5.o: $(SRCDIR)/server5.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

libpalindrome.o: $(SRCDIR)/libpalindrome.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o
