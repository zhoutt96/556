CC = g++
COPTS = -g -Wall -std=c++11
LKOPTS = 

OBJS =\
	Event.o\
	Link.o\
	Node.o\
	RoutingProtocolImpl.o\
	Simulator.o\
	PINGPONG.o\
	LS.o\
	DV.o\

HEADRES =\
	global.h\
	Event.h\
	Link.h\
	Node.h\
	RoutingProtocol.h\
	Simulator.h\
	common.h\

%.o: %.cc
	$(CC) $(COPTS) -c $< -o $@

all:	Simulator

Simulator: $(OBJS)
	$(CC) $(LKOPTS) -o Simulator $(OBJS)

$(OBJS): global.h
Event.o: Event.h Link.h Node.h Simulator.h
Link.o: Event.h Link.h Node.h Simulator.h
Node.o: Event.h Link.h Node.h Simulator.h
Simulator.o: Event.h Link.h Node.h RoutingProtocol.h Simulator.h
common.o: utils.h
PINGPONG.o: RoutingProtocolImpl.h utils.h
LS.o: RoutingProtocolImpl.h utils.h
DV.o: RoutingProtocolImpl.h utils.h
RoutingProtocolImpl.o: RoutingProtocolImpl.h utils.h


clean:
	rm -f *.o Simulator core*

