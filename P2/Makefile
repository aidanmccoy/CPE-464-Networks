# Makefile for CPE464 

CC= gcc
CFLAGS= -g

# The  -lsocket -lnsl are sometimes needed for the sockets.
# The -L/usr/ucblib -lucb gives location for the Berkeley library needed for
# the bcopy, bzero, and bcmp.  The -R/usr/ucblib tells where to load
# the runtime library.

LIBS =

all:  myClient myServer

myClient: cclient.c  testing.o
	clear
	$(CC) $(CFLAGS) -o Client -Wall cclient.c testing.o $(LIBS)

myServer: server.c  testing.o
	echo "COMPILING SERVER"
	$(CC) $(CFLAGS) -o Server -Wall server.c testing.o  $(LIBS)

#networks.o: networks.c networks.h
#	$(CC) $(CFLAGS) -c networks.c
	
testing.o: testing.c testing.h
	   $(CC) $(CFLAGS) -c testing.c
clean:
	rm -f myServer myClient *.o

rs: clean
	make
	hostname -I
	./Server 55555

rc: clean
	make
	./Client handle1 129.65.158.162 55555
