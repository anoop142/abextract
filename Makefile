CC=gcc
CFLAGS=-Wall -O2 
LIBS=-lz
LOCATION=/usr/local/bin


all: abextract



abextract: abextract.c
	$(CC) $(CFLAGS) $(LIBS)  abextract.c -o abextract


install: all
	strip abextract
	cp abextract $(LOCATION)

clean:
	rm -rf abextract
