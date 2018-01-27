CC = g++
CFLAGS += -Wall -I../lib -lzmq -lm

all: LTC2983-pub

spiLTC2983.o: ../lib/spiLTC2983.cpp
	$(CC) $(CFLAGS) -c ../lib/spiLTC2983.cpp

LTC2983-pub.o: LTC2983-pub.cpp
	$(CC) $(CFLAGS) -c LTC2983-pub.cpp

LTC2983-pub: LTC2983-pub.o spiLTC2983.o
	$(CC) LTC2983-pub.o spiLTC2983.o -o LTC2983-pub -lzmq
clean:
	rm -f *.o LTC2983-pub

