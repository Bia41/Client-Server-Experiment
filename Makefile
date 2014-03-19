all: clean compile

clean:
	rm -f LS
	rm -f SS
	rm -f UserClient/user

compile:
	gcc UserClient/userFinal.c -o UserClient/user
	g++ StorageServer.cpp -o SS
	g++ ListServer.cpp -o LS