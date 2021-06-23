CC=gcc
CFLAGS=-Wall -O2 
LIBS=-lz
LOCATION=/usr/local/bin


all: abextract



abextract: abextract.c
	$(CC) -o abextract abextract.c $(CFLAGS) $(LIBS)


install: all
	install -s abextract $(LOCATION)

uninstall:
	rm $(LOCATION)/abextract

clean:
	rm -rf abextract
