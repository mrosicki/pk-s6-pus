PROJECT = 11
SYSTEM = linux
SRCDIR = src
BINDIR = bin
CC = gcc
CFLAGS = -O2 -pedantic -Wall

all: devices icmp_sniff icmp_filter arp_sniff arp_spoof pf_packet

clean:
	@rm -rf $(BINDIR)/*

distrib: clean
	tar -cvf $(SYSTEM)-$(PROJECT).tar $(SRCDIR)/* Makefile

devices: devices.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -lpcap -o $(BINDIR)/$@

devices.o: $(SRCDIR)/devices.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

icmp_sniff: icmp_sniff.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -lpcap -o $(BINDIR)/$@

icmp_sniff.o: $(SRCDIR)/icmp_sniff.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

icmp_filter: icmp_filter.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -lpcap -o $(BINDIR)/$@

icmp_filter.o: $(SRCDIR)/icmp_filter.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

arp_sniff: arp_sniff.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -lpcap -o $(BINDIR)/$@

arp_sniff.o: $(SRCDIR)/arp_sniff.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

arp_spoof: arp_spoof.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -lpcap -o $(BINDIR)/$@

arp_spoof.o: $(SRCDIR)/arp_spoof.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

pf_packet: pf_packet.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -o $(BINDIR)/$@

pf_packet.o: $(SRCDIR)/pf_packet.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o