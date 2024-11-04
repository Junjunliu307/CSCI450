all: servermain serverA serverB serverC

servermain: servermain.cpp
	g++ -std=c++11 servermain.cpp -o servermain

serverA: serverA.cpp
	g++ -std=c++11 serverA.cpp -o serverA

serverB: serverB.cpp
	g++ -std=c++11 serverB.cpp -o serverB

serverC: serverC.cpp
	g++ -std=c++11 serverC.cpp -o serverC

clean:
	rm -f servermain serverA serverB serverC
