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


int main(int argc, char* argv[]) {
	int fd, fd1;
	struct hostent *hostptr, *hostptr2;
	struct sockaddr_in serveraddr, serveraddr1, clientaddr;
	int addrlen, addrlen2;
	char hostUDP[300];
	char portUDP[300];
	int portnrUDP;
	int portnrTCP;
	int c;
	char bufferUDP[700];
	char bufferUDP2[600];
	int PORT = 58028;
	int nrconteudo;
	char request1[300];
	int count=1;
	char* token;
	///////////////////////
	char* s = " \n";
	char* file;
	char* hostTCP;
	char* portTCP;
	char bufferTCP [1024];
	int nbytes;
	int nleft;
	char *ptr;
	int nread;
	int flag, flag1;
	int i;
	char* ptr2;
	char* rep;
	int tamanho= 0;
	int nwrite= 0;
	FILE* file1;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	portnrUDP = PORT;
	gethostname(hostUDP, sizeof(hostUDP));
	printf("HOST DA MAQUINA: %s\n", hostUDP);
	printf("PORT DEFAULT: %d\n", portnrUDP);
	 
 	while ( (c = getopt(argc, argv, "n:p:")) != -1) {
        	switch (c) {
        		case 'n': {
				strcpy(hostUDP, optarg);
				printf("HOST NOVO: %s\n", hostUDP);
				break;

			}	
        		case 'p': {
				strcpy(portUDP, optarg);
				portnrUDP = atoi(portUDP);
				printf("PORT NOVO: %d\n", portnrUDP);
				break;
			}
        		default : {
			        printf ("Foi para o default\n", c);
			}
		}
	}
	 
      	if((hostptr=gethostbyname(hostUDP))==NULL)exit(1);


	memset((void*) &serveraddr, (int) '\0', sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = ((struct in_addr *) (hostptr->h_addr_list[0]))->s_addr;
	serveraddr.sin_port = htons((u_short) portnrUDP);

	addrlen = sizeof(serveraddr);
	sendto(fd, "RQT\n", 3 + 1, 0, (struct sockaddr*) &serveraddr, addrlen);
	printf("o pedido foi feito\n");

	addrlen = sizeof(serveraddr);
	int size = recvfrom(fd, bufferUDP, sizeof(bufferUDP), 0, (struct sockaddr*) &serveraddr, &addrlen);

	bufferUDP[size -1] = '\0';
	token = strtok(bufferUDP, s);
	printf("%s ", token);
	token = strtok(NULL, s);
	printf("%s ", token);
	token = strtok(NULL, s);
	printf("%s\n", token);
	while((token = strtok(NULL, s)) !=  NULL){
		printf("%d: %s\n", count++,token);
	}

	addrlen2 = sizeof(serveraddr);
	printf("Escolha um conteúdo inserindo o número correspondente:\n");
	scanf("%d", &nrconteudo);
	sprintf(request1, "RQC %d", nrconteudo);
	strcat(request1, "\n");
	sendto(fd, request1, sizeof(request1), 0, (struct sockaddr*) &serveraddr, addrlen2);
	
	addrlen2 = sizeof(serveraddr);
	recvfrom(fd, bufferUDP2, sizeof(bufferUDP2), 0, (struct sockaddr*) &serveraddr, &addrlen2);
	
	printf("%s\n", strtok(bufferUDP2, "\n")); //este strtok foi para impedir lixos
	close(fd);
	

	///////////////////////// TCP ////////////////////////////////

	token = strtok(bufferUDP2, s);
    file = strtok(NULL, s);
	hostTCP = strtok(NULL, s);
	portTCP = strtok(NULL, s);
  
	if((fd1 = socket(AF_INET, SOCK_STREAM, 0)) == -1) exit(1);

	if((hostptr2=gethostbyname(hostTCP))==NULL)exit(1);

	portnrTCP = atoi(portTCP);

	memset((void*) &serveraddr1, (int) '\0', sizeof(serveraddr1));
	serveraddr1.sin_family = AF_INET;
	serveraddr1.sin_addr.s_addr = ((struct in_addr *) (hostptr2->h_addr_list[0]))->s_addr;
	serveraddr1.sin_port = htons((u_short) portnrTCP);
	
	connect(fd1, (struct sockaddr*) &serveraddr1, sizeof(serveraddr1));
	
	char rquest[600];
	sprintf(rquest, "REQ %s", file);
	strcat(rquest, "\n");

	write(fd1, rquest, strlen(rquest));


	nleft = 1024;
	ptr = bufferTCP;
	i = 1;
	
	nread =read(fd1, ptr, nleft);
	printf("o ptr %s\n", ptr);	
	ptr2 = malloc(sizeof(char)*nread);

	memcpy(ptr2, ptr, nread);

	rep= strtok(ptr, " ");
	while((rep = strtok(NULL, " ")) != NULL){
		i++; //o contador para os espaços
		if(strcmp(rep, "nok") == 0){
			close(fd1);
		}
		if (i == 3){
			tamanho = atoi(rep);
			break;
		}
	} 

	
	printf("%s\n", ptr2);
	file1 = fopen(file,"w");
	int nright = 100;
	while(tamanho > nwrite){
		nread =read(fd1, ptr, nright);
		//printf("%s",ptr);
		fwrite(ptr, sizeof(char), nread, file1);
		nwrite += nread;

		ptr = bufferTCP;
	}
	fclose(file1);

close(fd1);

}

