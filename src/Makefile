all:
	g++ -Wall -c utils.cpp 
	g++ -Wall -pthread server.cpp utils.o -o servidor
	g++ -Wall -pthread client.cpp -o cliente

