all: servermain serverA serverB serverC client

servermain: servermain.cpp
	g++ -std=c++11 servermain.cpp -o servermain -lpthread

serverA: serverA.cpp
	g++ -std=c++11 serverA.cpp -o serverA

serverB: serverB.cpp
	g++ -std=c++11 serverB.cpp -o serverB

serverC: serverC.cpp
	g++ -std=c++11 serverC.cpp -o serverC

client: client.cpp
	g++ -std=c++11 client.cpp -o client

clean:
	rm -f servermain serverA serverB serverC client
