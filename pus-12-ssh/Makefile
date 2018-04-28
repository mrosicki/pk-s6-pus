PROJECT = 12
SYSTEM = linux
SRCDIR = src
BINDIR = bin
CC = gcc
CFLAGS = -O2 -pedantic -Wall

all: authlist exec exec_rsa sftp sftp_get

clean:
	@rm -rf $(BINDIR)/*

distrib: clean
	tar -cvf $(SYSTEM)-$(PROJECT).tar $(SRCDIR)/* Makefile

authlist: authlist.o libcommon.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o  $(BINDIR)/libcommon.o -lssh2 -o $(BINDIR)/$@

authlist.o: $(SRCDIR)/authlist.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

libcommon.o: $(SRCDIR)/libcommon.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

exec: exec.o libcommon.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o $(BINDIR)/libcommon.o -lssh2 -o $(BINDIR)/$@

exec.o: $(SRCDIR)/exec.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

exec_rsa: exec_rsa.o libcommon.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o $(BINDIR)/libcommon.o -lssh2 -o $(BINDIR)/$@

exec_rsa.o: $(SRCDIR)/exec_rsa.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

sftp: sftp.o libcommon.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o $(BINDIR)/libcommon.o -lssh2 -o $(BINDIR)/$@

sftp.o: $(SRCDIR)/sftp.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

sftp_get: sftp_get.o libcommon.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o $(BINDIR)/libcommon.o -lssh2 -o $(BINDIR)/$@

sftp_get.o: $(SRCDIR)/sftp_get.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o