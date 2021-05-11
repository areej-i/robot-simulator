#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 6000

#include "simulator.h"

// This is the main function that simulates the "life" of the robot
// The code will exit whenever the robot fails to communicate with the server
int main() {
  
  int clientSocket, addrSize, bytesReceived;
  struct sockaddr_in serverAddr;
  unsigned char buffer[30]; // stores sent and received data
  int x;
  int y;
  int d;
  int c = 1;
  int dir;
  
  // Set up the random seed
  srand(time(NULL));
  
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
  
  // Register with the server
  buffer[0] = REGISTER;

  // Send register command to server.  Get back response data
  // and store it.   If denied registration, then quit.
  sendto(clientSocket, buffer, sizeof(buffer), 0, (struct sockaddr *) &serverAddr, addrSize);
  
  bytesReceived = recvfrom(clientSocket, buffer, sizeof(buffer), 0, (struct sockaddr *) &serverAddr, &addrSize);
  buffer[bytesReceived] = 0; // put a 0 at the end so we can display the string
  
  // Go into an infinite loop exhibiting the robot behavior
  if (buffer[0] == OK)
  {
      while (1) 
      {
        // Check if can move forward
        buffer[0] = CHECK_COLLISION;
        sendto(clientSocket, buffer, sizeof(buffer), 0, (struct sockaddr *) &serverAddr, addrSize);

        // Get response from server
        bytesReceived = recvfrom(clientSocket, buffer, sizeof(buffer), 0, (struct sockaddr *) &serverAddr, &addrSize);
        buffer[bytesReceived] = 0; // put a 0 at the end so we can display the string
        
        x = buffer[3]*256 + buffer[4];
        y = buffer[5]*256 + buffer[6];
        if (buffer[7] == 1)
        	d = (buffer[8]*256 + buffer[9]) * -1;
        else
        	d = (buffer[8]*256 + buffer[9]);
    	
		if (buffer[0] == LOST_CONTACT)
		{
			printf("CLIENT: Shutting down...\n");
        	//sendto(clientSocket, buffer, sizeof(buffer), 0, (struct sockaddr *) &serverAddr, addrSize);
			break;
		}
        // If ok, move forward
        if (buffer[0] == OK)
        {
           c = 1;
           d = (d*PI)/180;
           
    	   x = x+ROBOT_SPEED*cos(d); 
    	   
    	   buffer[3] = x/256;
           buffer[4] = x%256;
    	   
    	   y = y+ROBOT_SPEED*sin(d);
    	   buffer[5] = y/256;
           buffer[6] = y%256;
        }
		// Otherwise, we could not move forward, so make a turn.
		else if (buffer[0] == NOT_OK_BOUNDARY || buffer[0] == NOT_OK_COLLIDE)
		{
			if(c == 1)
			{
				if (dir == 0)
					dir = 1;
				else
				{
   	      			srand(time(NULL));
    	      		dir = rand() % 2;
    	       }
			}
			if (dir == 0) //Move left
			{
				if(d >= -165 && d <= 180)
    	    	{
    	    	    d = d-ROBOT_TURN_ANGLE;
    	    		if (d < 0)
    	    		{
    	    			d=d*-1;
    	    			buffer[7] = 1;
    	    		}
    	    		else 
    	    		{
		 				buffer[7] = 2;
		 			}
		 			buffer[8] = d/256; 
                	buffer[9] = d%256; 
		 			c = 0;
	        	}
	        	else
	        		c = 1;
			}
			else if (dir == 1) //Move right
    		{
				if (d >= -180 && d <= 165)
	      		{
		 			d = d+ROBOT_TURN_ANGLE;
		 			if(d < 0)
		 			{
		 				d=d*-1;
                      	buffer[7] = 1;
                    }
                	else
                	{
                      	buffer[7] = 2;
                    }
                	buffer[8] = d/256; 
                	buffer[9] = d%256; 
	        	}
	        	else
	        		dir = 0;
    	    	c = 0;
    		}
		}
		buffer[0] = STATUS_UPDATE;
    	sendto(clientSocket, buffer, sizeof(buffer), 0, (struct sockaddr *) &serverAddr, addrSize);
    
		usleep(100000);
	}
	close(clientSocket);
  }
  else
	{
		printf("CLIENT: Error, cannot add more robots.\n"); 
		close(clientSocket);
		exit(-1);
	}
}

