#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "simulator.h"

#define SERVER_PORT 6000

Environment    environment;  // The environment that contains all the robots

// Handle client requests coming in through the server socket.  This code should run
// indefinitiely.  It should repeatedly grab an incoming messages and process them. 
// The requests that may be handled are STOP, REGISTER, CHECK_COLLISION and STATUS_UPDATE.   
// Upon receiving a STOP request, the server should get ready to shut down but must
// first wait until all robot clients have been informed of the shutdown.   Then it 
// should exit gracefully.  

int canItMove(unsigned char buffer[30])
{
	int d;
	if (buffer[7] == 1)
		d = (buffer[8]*256 + buffer[9]) * -1;
	else
		d = (buffer[8]*256 + buffer[9]);
  	
  	d = (d*PI)/180;
	int x = (buffer[3]*256 + buffer[4]);
	x = x+ROBOT_SPEED*cos(d);
  	
  	int y = (buffer[5]*256 + buffer[6]);
  	y = y+ROBOT_SPEED*sin(d);	
    	
  	if(x >= (ENV_SIZE-ROBOT_RADIUS) || y >= (ENV_SIZE-ROBOT_RADIUS) || (x <= ROBOT_RADIUS) || (y <= ROBOT_RADIUS))
  	{
  		buffer[0] = NOT_OK_BOUNDARY;
  		return(buffer[0]);
  	}
  	for(int i = 0; i < buffer[2]; i++)
  	{
  		float val = 2;
  		float a = powf(environment.robots[i].x-x,val);
  		float b = powf(environment.robots[i].y-y,val);
  		if((sqrt(a + b)) <= (2 * ROBOT_RADIUS))
  		{
  			buffer[0] = NOT_OK_COLLIDE;
  			return(buffer[0]);
  		}		
  	}
  	if (environment.shutDown == 1)
  	{
  		buffer[0] = LOST_CONTACT;
  	}
  	else
  	{
  		buffer[0] = OK;
  	}
  	return(buffer[0]);
}
void *handleIncomingRequests(void *e) {
	char   		  	    	online = 1;
  	int			        	serverSocket;
	struct sockaddr_in		serverAddr, clientAddr;
	int			        	status, addrSize, bytesReceived;
	fd_set 					readfds, writefds;
	unsigned char 			buffer[30];
	environment.numRobots  = 0;
  	
	serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  	
  	// Initialize the server
	if (serverSocket < 0) {
	 	exit(-1);
	}
	
	// Setup the server address
	memset(&serverAddr, 0, sizeof(serverAddr)); // zeros the struct
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons((unsigned short) SERVER_PORT);
	 
	// Bind the server socket
	status = bind(serverSocket,(struct sockaddr *)&serverAddr, sizeof(serverAddr));
	if (status < 0) {
	 	printf("*** SERVER ERROR: Could not bind socket.\n");
	 	exit(-1);
	}  
  	
  	// Wait for clients now
	while (online) 
	{
	    FD_ZERO(&readfds);
	    FD_SET(serverSocket, &readfds);
	    FD_ZERO(&writefds);
	    FD_SET(serverSocket, &writefds);
	    status = select(FD_SETSIZE, &readfds, &writefds, NULL, NULL);
	    if (status == 0) { // Timeout occurred, no client ready
	    }
	    else if (status < 0) 
	    {
	        printf("*** SERVER ERROR: Could not select socket.\n");
	        exit(-1);
	    }
	    else 
	    {
	        addrSize = sizeof(clientAddr);
	        bytesReceived = recvfrom(serverSocket, buffer, sizeof(buffer), 0, (struct sockaddr *) &clientAddr, &addrSize);
	        if (bytesReceived > 0) {
	           buffer[bytesReceived] = '\0';
	        }
	        if (buffer[0] == REGISTER)
	        {
	            if(environment.numRobots <= MAX_ROBOTS-1)
	            {
	            	int x;
	            	int y;
	            	int d; 
					srand(time(NULL));
					int n = environment.numRobots;
					buffer[0] = OK;
					int t = 1;
                        
					// ID of robot (robot #)
					buffer[2] = n;
                        
					while (t > 0)
					{
			    		t = 0;
			    		//X position 
			    		x = (int)(rand()/(double)RAND_MAX*(ENV_SIZE-30));
			    		x = x + ROBOT_RADIUS;
			   	    	//Y position
			    		y = (int)(rand()/(double)RAND_MAX*(ENV_SIZE-30));
			    		y = y + ROBOT_RADIUS;
			
			    		for (int l = 0; l < n; l++)
			    		{
			        		if(environment.robots[l].x == environment.robots[n].x && environment.robots[l].y == environment.robots[n].y)
			     	    		t++; 
                		}
            		}
					printf("X value: %d\n",x);
					buffer[3] = x/256;
					buffer[4] = x%256;
					float x1 = (float)x;
					environment.robots[n].x = x1;
                        
					printf("Y value: %d\n",y);
					buffer[5] = y/256; 
					buffer[6] = y%256;
					float y1 = (float)y;
					environment.robots[n].y = y1;

					//Direction
                	d = (int)(rand()/(double)RAND_MAX*360);
                	d = d - 180; 
                	environment.robots[n].direction = d;
                	if(d < 0)
                	{
                		buffer[7] = 1;
                		d = d * -1;
                	}
                	else
                		buffer[7] = 2;
                	buffer[8] = d/256; 
                	buffer[9] = d%256; 
                	environment.numRobots++;
	            }
	            else
	            	buffer[0] = NOT_OK;

	            sendto(serverSocket, buffer, sizeof(buffer), 0, (struct sockaddr *) &clientAddr, addrSize);
			}
			if (buffer[0] == STOP)
			{
				printf("Sending shut down to client.\n");
				buffer[0] = LOST_CONTACT;
				environment.numRobots--;
				environment.shutDown = 1;
				sendto(serverSocket, buffer, sizeof(buffer), 0, (struct sockaddr *) &clientAddr, addrSize);

				if (environment.numRobots == -1)
					break;
			}
			else if (buffer[0] == CHECK_COLLISION)
			{	
				buffer[0] = canItMove(buffer);
				if (environment.shutDown == 1)
				{
					environment.numRobots--;
	         	}
	         	sendto(serverSocket, buffer, sizeof(buffer), 0, (struct sockaddr *) &clientAddr, addrSize);

				if (environment.numRobots == -1)
					break;
			}
			if (buffer[0] == STATUS_UPDATE)
			{
				int x = buffer[3]*256+buffer[4];
                float x1 = (float)x;
				environment.robots[buffer[2]].x = x1;
			
				int y = buffer[5]*256+buffer[6];
                float y1 = (float)y;
    			environment.robots[buffer[2]].y = y1;
    			
    			int d;
    			if (buffer[7] == 1)
    				d = (buffer[8]*256 + buffer[9]) * -1;
    			else 
    				d = (buffer[8]*256 + buffer[9]);
				environment.robots[buffer[2]].direction = d;
			}
	    }
  	}
  	// ... WRITE ANY CLEANUP CODE HERE ... //
  	environment.shutDown = 1;
	printf("SERVER: Shutting down...\n");
  	close(serverSocket);  
}

int main() {

	pthread_t server;
	pthread_t drawRobots;
	
	// So far, the environment is NOT shut down
	environment.shutDown = 0;
  
	// Set up the random seed
	srand(time(NULL));

	pthread_create(&server, NULL, handleIncomingRequests, &environment);
	pthread_create(&drawRobots, NULL, redraw, &environment);

	// Spawn an infinite loop to handle incoming requests and update the display
	while(environment.shutDown == 0)
	{
		pthread_join(server, NULL);
		pthread_join(drawRobots, NULL);
	}

	// Wait for the update and draw threads to complete
	//pthread_exit(void *status);
}





