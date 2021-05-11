#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "simulator.h"


int main() 
{
     // ... ADD SOME VARIABLES HERE ... //
     int 			clientSocket, addrSize, bytesReceived;
     struct sockaddr_in 	serverAddr;
     char 			buffer[30]; // stores sent and received data
  
     // Create socket
     clientSocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
     if (clientSocket < 0) {
     printf("*** CLIENT ERROR: Could open socket.\n");
     exit(-1);
     }
  
     // Setup address
     memset(&serverAddr, 0, sizeof(serverAddr));
     serverAddr.sin_family = AF_INET;
     serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
     serverAddr.sin_port = htons((unsigned short) SERVER_PORT);
  

     addrSize = sizeof(serverAddr);

     buffer[0] = STOP;
     printf("CLIENT: Sending \"%d\" to server.\n", STOP);
     sendto(clientSocket, buffer, sizeof(buffer), 0, (struct sockaddr *) &serverAddr, addrSize);

     
     close(clientSocket); // Don't forget to close the socket !
}
