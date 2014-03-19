#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <vector>
#include <string>
#include <sstream> 
#include <fstream>
#include <iostream>

#define DEFAULT_PORT 58028
#define BUFFER_SIZE 1024

typedef struct{
	std::string name;
	std::string host;
	int port;
} Content;

std::string topic;
std::vector<Content> contents;

void readContentList(std::string filename) {
	std::ifstream file(filename.c_str());

	std::string line;
	if(std::getline(file, line))
		topic = line.substr(1);

	while(std::getline(file, line)) {
		Content content;
		std::stringstream stream(line);

		stream >> content.name;
		stream >> content.host;
		stream >> content.port;

		contents.push_back(content);
	}

	file.close();

	std::cout << "Topic: " << topic << std::endl;

	std::cout << "Contents: " << contents.size() << std::endl;
	for(int i = 0; i < contents.size(); i++)
		std::cout << contents[i].name << " " << contents[i].host << " " << contents[i].port << std::endl;
} 

int main(int argc, char* argv[]) {

	readContentList("test.txt");

	int port = DEFAULT_PORT;

	if(getopt(argc, argv, "p:") != -1)
		std::stringstream(std::string(optarg)) >> port;

	std::cout << "Using port " << port << std::endl;

	int socketID = socket(AF_INET, SOCK_DGRAM, 0);

	struct sockaddr_in serveraddr, clientaddr;

	memset((void*) &serveraddr, (int) '\0', sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons((u_short) port);

	if(bind(socketID, (struct sockaddr*) &serveraddr, sizeof(serveraddr)) == -1) {
    	perror("setsockopt");
    	return 1;
	}

	for(;;) {
		char buffer[BUFFER_SIZE];

		unsigned int addrlen = sizeof(clientaddr);

		recvfrom(socketID, buffer, BUFFER_SIZE, 0, (struct sockaddr*) &clientaddr, &addrlen);

		std::string request;
		std::string bufferString(buffer);
		std::stringstream reqStream(bufferString);

		reqStream >> request;

		char hostclient[BUFFER_SIZE];
		if(inet_ntop(AF_INET, &clientaddr.sin_addr.s_addr, hostclient, sizeof(hostclient)) == NULL){
			std::cerr << "Error: Could not convert byte to address" << std::endl;
			return -3;
		}

		std::cout << "Request: " << request << " from " << hostclient << " on port " << clientaddr.sin_port << std::endl;

		std::string reply;

		if(request.compare("RQT") == 0){
			std::stringstream msg;
			msg << "AWT " << topic << " " << contents.size() << "\n";

			for(int i = 0; i < contents.size(); i++){
				msg << contents[i].name << "\n";
			}
		
			reply = msg.str();	
		} else if(request.compare("RQC") == 0){
			int contentNo;
			reqStream >> contentNo;
			contentNo--;

			if(contentNo < 0 || contentNo >= contents.size()){
				reply = "ERR\n";
			} else{
				Content content = contents[contentNo];
				std::stringstream msg;
				msg << "AWC " << content.name << " " << content.host << " " << content.port << "\n";
				reply = msg.str();		

			}
		} else{
			reply = "ERR\n";			
		}

		std::cout << "Reply: " << reply << std::endl;

		sendto(socketID, reply.c_str(), reply.size(), 0, (struct sockaddr*) &clientaddr, addrlen);
	
	}

	close(socketID);

	return 0;

}
