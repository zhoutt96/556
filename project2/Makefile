CC	 	= gcc
LD	 	= gcc
CFLAGS	 	= -Wall -g
LDFLAGS	 	= 
DEFS 	 	=

#all:	recvfile sendfile
all: sendfile

#recvfile: recvfile.cc
#	$(CC) $(DEFS) $(CFLAGS) $(LIB) -o recvfile recvfile.c utils.c

sendfile: sendfile.c
	$(CC) $(DEFS) $(CFLAGS) $(LIB) -o sendfile utils.c sendfile.c

clean:
	rm -f *.o
	rm -f *~
	rm -f core.*.*
	rm -f recvfile
	rm -f sendfile
	rm -rf *.dSYM
