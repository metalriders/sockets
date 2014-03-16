#include <stdio.h>      
#include <sys/socket.h> 
#include <arpa/inet.h>  
#include <stdlib.h>     
#include <string.h>     
#include <unistd.h>

#define true 1
#define false 0

int main(int argc, char* argv[]) {
	
	int udpSocket;
	struct sockaddr_in udpServer, udpClient;
	struct sockaddr_in broadcastAddr;

	socklen_t addrlen = sizeof(udpClient);
	char buffer[255];
	char ip[17];
	u_short clientPort;

	int status;
	char sendString[255]; 
	unsigned int sendStringLen; 
	
	char *client_name;

	if (argc < 2)                     
	{
		fprintf(stderr,"Usage:  %s <Name>\n", argv[0]);
		exit(1);
	}
	client_name = argv[1];

	//Creates socket with IPv4 communication throug daragrams using UDP
	udpSocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(udpSocket == -1) {
		fprintf(stderr,"Can't create UDP Socket");
		return 1;
	}
	
	//Set protocol for server
	udpServer.sin_family = AF_INET;
	//Copies network address to server using IPv4
	inet_pton(AF_INET,"0.0.0.0",&udpServer.sin_addr.s_addr);
	udpServer.sin_port = htons(5000);

	//Assigning a name to a socket
	status = bind(udpSocket, (struct sockaddr*)&udpServer,sizeof(udpServer));
	if(status != 0) { 
	  fprintf(stderr,"Can't bind");
	}
	
	//Receive messages from server
	while(true) {
		bzero(buffer,255);
		status = recvfrom(udpSocket, buffer, 255, 0, (struct sockaddr*)&udpClient, &addrlen );
		inet_ntop(AF_INET,&(udpClient.sin_addr),ip,INET_ADDRSTRLEN);
		clientPort = ntohs(udpClient.sin_port);
		
		printf("Recibimos: [%s:%i] %s\n",ip,clientPort,buffer);
		
		//Send response
		strcpy(sendString,"Que tal, Soy ");
		strcat(sendString, client_name);

		sendStringLen = strlen(sendString);
		status = sendto(udpSocket, sendString, sendStringLen, 0, (struct sockaddr*)&udpClient, sizeof(broadcastAddr));   
		printf("Send %i bytes to Sender addr\n",status);
		sleep(3);  

   	    fflush(stdout);
	}
	
	close(udpSocket);
	
	return 0;
	
}
