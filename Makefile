CC=gcc
LIBSOCKET=-lnsl
CCFLAGS=-Wall -g -std=c99
SRV=server
CLT=client

all: build

build: $(SRV) $(CLT)

$(SRV): $(SRV).c lib$(SRV).h
	$(CC) $(SRV).c -o $(SRV) $(LIBSOCKET)

$(CLT):	$(CLT).c lib$(CLT).h
	$(CC) $(CLT).c -o $(CLT) $(LIBSOCKET)

clean:
	rm -f *.o *~
	rm -f $(SRV) $(CLT)

clean-logs:
	rm -f *.log

clean-all: clean clean-logs