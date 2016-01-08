
CC=arm-linux-gcc
LIBDIR=-L.
LINKS=-lswm -lswfile -lpcap
CFLAGS=-I/root/eldk/arm/usr/include/pcap
all:
	$(CC) $(CFLAGS) swm-regwrite.c -o swm-regwrite $(LIBDIR) $(LINKS)
	$(CC) $(CFLAGS) swm-test.c -o swm-test $(LIBDIR) $(LINKS)
