CC	 	= g++
LD	 	= g++
CFLAGS	 	= -Wall -g

LDFLAGS	 	=
DEFS 	 	=

#all: sendfile recvfile


#sendfile: sendfile.cpp
#	$(CC) $(DEFS) $(CFLAGS) $(LIB) -o sendfile.cpp utils.cpp queue.cpp
#
#recvfile: recvfile.cpp
#	$(CC) $(DEFS) $(CFLAGS) $(LIB) -o recvfile.cpp utils.cpp

all:
	g++ sendfile.cpp utils.cpp queue.cpp -o sendfile
	g++ recvfile.cpp utils.cpp -o recvfile
	#g++ compare.cpp -o compare

clean:
	rm -f *.o
	rm -f *~
	rm -f core.*.*
	rm -f recvfile
	rm -f sendfile
	rm -rf *.dSYM
	rm -f compare
