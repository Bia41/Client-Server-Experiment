#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

#define DEFAULT_PORT 59000
#define BUFFER_SIZE 1024

int main(int argc, char* argv[]) {

	int port = DEFAULT_PORT;

	if(getopt(argc, argv, "p:") != -1)
		std::stringstream(std::string(optarg)) >> port;

	std::cout << "Using port " << port << std::endl;

	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in serveraddr, clientaddr;

	memset((void*) &serveraddr, (int) '\0', sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons((u_short)port);

	bind(serverSocket, (struct sockaddr*) &serveraddr, sizeof(serveraddr));

	listen(serverSocket, 5);

	while(1){
		unsigned int addrlen = sizeof(clientaddr);

		int connection = accept(serverSocket, (struct sockaddr*) &clientaddr, &addrlen);
   		pid_t pid = fork();
 
   		if (pid == -1)
   			std::cerr << "error: can't fork" << std::endl;
   		
   		else if (pid == 0){
   			char buffer[BUFFER_SIZE];
   			read(connection, buffer, BUFFER_SIZE);

			std::string request;
			std::string bufferString(buffer);
			std::stringstream reqStream(bufferString);

			reqStream >> request;   			

			if(request.compare("REQ") != 0){
				std::cerr << "ERROR: invalid request" << std::endl;
				close(connection);
				_exit(1);
			}

			std::string fileName;

			reqStream >> fileName;

			std::ifstream file(fileName.c_str());

			if(!file.is_open()){
				std::cerr << "ERROR: file does not exist" << std::endl;
				std::string reply = "REP nok\n";
				write(connection, reply.c_str(), reply.size());
				close(connection);
      			_exit(1);
			}
			
			file.seekg(0, file.end);
			int size = file.tellg();
			
			std::cout << "Client requested file " << fileName << " with size " << size << std::endl;

			std::stringstream msg;
			msg << "REP ok " << size << " ";
			
			std::string reply = msg.str();

			write(connection, reply.c_str(), reply.size());

			file.seekg(0, file.beg);
			char fileBuffer[size];
			file.read(fileBuffer, size);
			file.close();

			write(connection, fileBuffer, size);

			std::cout << "File Sent" << std::endl;

      		close(connection);
      		_exit(0);
		}
	}

	close(serverSocket);
	return 0;

}