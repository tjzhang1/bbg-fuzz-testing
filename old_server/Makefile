all:
	gcc -Wall -o server.sh server.c tcp.c aht20.c http.c
	gcc -Wall -o client.sh client.c tcp.c http.c
server: 
	gcc -Wall -o server.sh server.c tcp.c aht20.c http.c
client:
	gcc -Wall -o client.sh client.c tcp.c http.c
start:
	./server.sh 4000 &
test:
	./client.sh localhost 4000
stop:
	killall -v server.sh
