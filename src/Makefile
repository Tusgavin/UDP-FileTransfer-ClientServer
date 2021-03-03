CLIENT_DIR:=./client
SERVER_DIR:=./server

all:
	g++ -Wall -c utils.cpp 
	g++ -Wall -pthread server.cpp utils.o -o ${SERVER_DIR}/servidor
	g++ -Wall -pthread client.cpp -o ${CLIENT_DIR}/cliente