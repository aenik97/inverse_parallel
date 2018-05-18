CFLAGS = -pthread

all: a.out

a.out: main.o inverse.o
	g++ $(CFLAGS) main.o inverse.o -o a.out

main.o: main.cpp prototype.h
	g++ -c $(CFLAGS) main.cpp

inverse.o: inverse.cpp prototype.h
	g++ -c $(CFLAGS) inverse.cpp
