#include <stdio.h>      
#include <sys/socket.h> 
#include <arpa/inet.h>  
#include <stdlib.h>     
#include <string.h>     
#include <unistd.h>   

#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>  

#define true 1
#define false 0

int main(int argc, char *argv[])
{
	int bcSock;                        
	struct sockaddr_in broadcastAddr, udpClient; 
	char *broadcastIP;                
	unsigned short broadcastPort;

	char *sendString;                 
	int broadcastPermission;          
	unsigned int sendStringLen; 

	int status;
	socklen_t addrlen = sizeof(broadcastAddr);
	char buffer[255];
	char ip[17];
	u_short clientPort;

	int i;
	struct ifreq ifr;

	/* I want to get an IPv4 IP address */
	ifr.ifr_addr.sa_family = AF_INET;

	/* I want IP address attached to "eth0" */
	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);
	
	if (argc < 3)                     
	{
		fprintf(stderr,"Usage:  %s <Port> <Send String>\n", argv[0]);
		exit(1);
	}

	//broadcastIP = argv[1];            /* First arg:  broadcast IP address */ 
	broadcastPort = atoi(argv[1]);    /* Second arg:  broadcast port */
	sendString = argv[2];             /* Third arg:  string to broadcast */

	//Creates socket with IPv4 communication throug daragrams using UDP
	bcSock = socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP);
	ioctl(bcSock, SIOCGIFADDR, &ifr);

	if(bcSock == -1) {
		fprintf(stderr,"Can't create Socket");
		return 1;
	}
	
	//Set broadcast permission for given socket
	broadcastPermission = 1;
	status = setsockopt(bcSock, SOL_SOCKET, SO_BROADCAST, (void *) &broadcastPermission, sizeof(broadcastPermission));
	if(status == -1) {
		fprintf(stderr,"Can't set Brodcast Option");
		return 1;
	}
	
	//Set memory area in zeroes for destiny
	memset(&broadcastAddr, 0, sizeof(broadcastAddr));   
	broadcastAddr.sin_family = AF_INET;           

     	//Copies network address to destiny using IPv4
	inet_pton(AF_INET,broadcastIP,&broadcastAddr.sin_addr.s_addr); 
	broadcastAddr.sin_port = htons(broadcastPort);         

	//Send one message to broadcast
	sendStringLen = strlen(sendString);
	status = sendto(bcSock,sendString,sendStringLen,0,(struct sockaddr*)&broadcastAddr, sizeof(broadcastAddr));    
	printf("Send %i bytes to brodcast addr\n",status);
	//sleep(3);
	
	while(true) {
		//Receive message from clients
		bzero(buffer,255);
		status = recvfrom(bcSock, buffer, 255, 0, (struct sockaddr*)&broadcastAddr, &addrlen );
		inet_ntop(AF_INET,&(udpClient.sin_addr),ip,INET_ADDRSTRLEN);
		clientPort = ntohs(udpClient.sin_port);
		
		printf("Recibimos de [%s:%i] > %s\n",ip,clientPort,buffer);
	}
}
