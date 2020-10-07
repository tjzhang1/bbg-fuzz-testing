CC = gcc
CFLAGS = -Wall -g -c -I$(IDIR)
IDIR = ./inc
SDIR = ./src
ODIR = ./obj

SERV_DEP := server.o tcp.o aht20.o http.o circularqueue.o
SERV_OBJECTS := $(patsubst %, $(ODIR)/%, $(SERV_DEP))  #:= ./obj/server.o ./obj/tcp.o ./obj/aht20.o ./obj/http.o ./obj/circularqueue.o
CLI_DEP := client.o tcp.o http.o
CLI_OBJECTS := $(patsubst %, $(ODIR)/%, $(CLI_DEP))

all: server client

server: $(SERV_OBJECTS)
	$(CC) -g -lpthread $(SERV_OBJECTS) -o server.sh
client: $(CLI_OBJECTS)
	$(CC) -g $(CLI_OBJECTS) -o client.sh 

#$(ODIR)/server.o: $(SDIR)/server.c
#	$(CC) $(CFLAGS) -o $@ $<
#$(ODIR)/tcp.o: $(SDIR)/tcp.c
#	$(CC) $(CFLAGS) -o $(ODIR)/tcp.o $(SDIR)/tcp.c
#$(ODIR)/aht20.o: $(SDIR)/aht20.c
#	$(CC) $(CFLAGS) -o $(ODIR)/aht20.o $(SDIR)/aht20.c
#$(ODIR)/http.o: $(SDIR)/http.c
#	$(CC) $(CFLAGS) -o $(ODIR)/http.o $(SDIR)/http.c
#$(ODIR)/circularqueue.o: $(SDIR)/circularqueue.c
#	$(CC) $(CFLAGS) -o $(ODIR)/circularqueue.o $(SDIR)/circularqueue.c
#$(ODIR)/client.o: $(SDIR)/client.c
#	$(CC) $(CFLAGS) -o $(ODIR)/client.o $(SDIR)/client.c

$(ODIR)/%.o: $(SDIR)/%.c
	$(CC) $(CFLAGS) $< -o $@

start:
	./server.sh 4000 &
test:
	./client.sh localhost 4000
stop:
	pkill server.sh
restart: stop start

clean:
	rm -f *.o
	rm -f $(ODIR)/*.o
