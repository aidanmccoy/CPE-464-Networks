#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/select.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
//#include <fcntl.h>
#include <string.h>
#include <strings.h>

#include "server.h"


//#define BACKLOG 5

/*typedef struct clientStruct {
	char  *handle;
	int socket;
} client;*/


struct chatLink *links;
int numLinks = 0;
int maxLinks = 10;

int serverSocket;   //socket descriptor for the server socket
//int port_num;	

int seq_num = 0;

char *msgBuff;              //buffer for receiving from client



int main(int argc, char *argv[])
{
	initBuffs();
	//links = (struct chatLink *) malloc(sizeof(struct chatLink) * maxLinks);
    //create packet buffer
    //msgBuff = (char *) malloc(BUFFSIZE);
    
    //	get port_num from args if passed in
    if(argc == 2) {
    	serverSocket = setUpServer(atoi(argv[1]));
    } else {
    	serverSocket = setUpServer(0);
    }

    //create the server socket
    //serverSocket = setUpServer(port_num);

    //look for a client to serve
    //tcp_listen();
    
    do {
    	selectSocket();
    } while(1);
}

/* This function sets the server socket.  It lets the system
   determine the port number if none is set via arguments.  
   The function returns the server socket number 
   and prints the port number to the screen.  */

int setUpServer(int portNum)
{
	int sk;
    struct sockaddr_in local;      
    socklen_t len = sizeof(local);  /* length of local address        */

    /* create the socket  */
    if((sk = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    	 perror("Error in socket call");
      exit(-1);
    }

    /*if(sk < 0) {
      perror("Error in socket call");
      exit(-1);
    }*/

    local.sin_family = AF_INET;         
    local.sin_addr.s_addr = INADDR_ANY; 
    local.sin_port = htons(portNum);                 

    /* bind the name (address) to a port */
    if (bind(sk, (struct sockaddr *) &local, sizeof(local)) < 0) {
		perror("Error in bind call");
		exit(-1);
    }
    
    //get the port name and print it out
    if (getsockname(sk, (struct sockaddr*)&local, &len) < 0) {
		perror("Error in getsockname call");
		exit(-1);
    }

    printf("Server is using port %d\n", ntohs(local.sin_port));
	        
	if (listen(sk, 10) < 0) {
  	 	perror("listen call");
    	exit(-1);
  	}
    return sk;
}


void initBuffs() {
	links = (struct chatLink *) calloc(sizeof(struct chatLink), maxLinks);

	msgBuff = (char *) malloc(BUFFSIZE);
}



/* This function waits for a client to ask for services.  It returns
   the socket number to service the client on.    */

/*void tcp_listen()
{
  if (listen(serverSocket, 10) < 0) {
    	perror("listen call");
    	exit(-1);
  }
}*/

int getNewClient() {
  int clientSocket = 0;
  void * tempArray;
  if ((clientSocket = accept(serverSocket, (struct sockaddr*)0, (socklen_t *)0)) < 0) {
    	perror("accept call");
    	exit(-1);
  }

   if(numLinks == maxLinks - 1) {
   		tempArray = calloc(sizeof(struct chatLink), (maxLinks * 2));
   		memcpy(tempArray, (void *)links, (numLinks * sizeof(struct chatLink)));
   		links = (struct chatLink *) tempArray;
   		maxLinks = (maxLinks * 2);
   	}
/*maxLinks * = 2;
  	struct chatLink *temp = links;
  	links = realloc(links, sizeof(struct chatLink) * maxLinks);
  	int i;
  	for(i = 0; i < maxLinks; i++) {
  		links[i] = temp[i];
  	}  */
    
  //	Add client to array, increasing size if necessary
  links[numLinks++].socket = clientSocket;
    
/*  if(numLinks == maxLinks) {
  	maxLinks * = 2;
  	struct chatLink *temp = links;
  	links = realloc(links, sizeof(struct chatLink) * maxLinks);
  	int i;
  	for(i = 0; i < maxLinks; i++) {
  		links[i] = temp[i];
  	}  */	  
  	return(clientSocket);
}

/*int max_client_socket() {
	int max = serverSocket, i;
	for(i = 0; i < numLinks; i++) {
		if(links[i].socket > max) max = links[i].socket;
	}
	return max;
}*/

void selectSocket() {
	fd_set readySks;
	int linkIter, maxSocket = serverSocket;
	
	FD_ZERO(&readySks); // reset variables

	//	check server socket first
	FD_SET(serverSocket, &readySks);
	
	for(linkIter = 0; linkIter < numLinks; linkIter++) {
		FD_SET(links[linkIter].socket, &readySks); 
	}
	
	for(linkIter = 0; linkIter < numLinks; linkIter++) {
		if(links[linkIter].socket > maxSocket) {
			maxSocket = links[linkIter].socket;
		}
	}
	maxSocket++;

	if(select(maxSocket, (fd_set *) &readySks, NULL, NULL, NULL)  < 0) {
		perror("select call");
		exit(-1);
	}
	
	//	check server socket first
	if(FD_ISSET(serverSocket, &readySks)) {
		getNewClient();
	}
	
	//	check all client sockets
	for(linkIter = 0; linkIter < numLinks; linkIter++) {
		if(FD_ISSET(links[linkIter].socket, &readySks)) {
			printf("Before the process call\n");
			processMsg(links[linkIter].socket);
		}
	}
}

void processMsg(int clientSocket) {
	int msgLen;
		
	//now get the data on the clientSocket
    if ((msgLen = recv(clientSocket, msgBuff, BUFFSIZE, 0)) < 0) {
		perror("recv call");
		exit(-1);
    }
    printf("After recv call\n");
    
    /*if(msgLen == 0) {
    	clientExit(clientSocket);
    }
    else*/
    if (msgLen != 0) {
    	printf("The flag is %d\n",msgBuff[2]);
    	switch(msgBuff[2]) {
    		case 1: //Initial Packet
    			initialPacket(clientSocket, msgBuff, msgLen);
    			break;
    		case 4:	//Broadcast Message
    			clientBroadcastReceive(clientSocket, msgBuff, msgLen);
    			break;
    		case 5: //Drirect Message
    			printf("Direct Message recieved\n");
    			clientMessageReceive(clientSocket, msgBuff, msgLen);
    			break;
			case 8:  //Client exiting
    			clientExitReceive(clientSocket, msgBuff, msgLen);
    			break;
    		case 10: //Request for client list
    			clientListReceive(clientSocket, msgBuff, msgLen);
    			break;
    		
    		default:
    			printf("Unknown flag recieved\n");
    	}  
    } else {
    	clientExit(clientSocket);
    }  
    
}

void sendPacket(int clientSocket, char *send_buf, int send_len) {
	int sent;
	
	sent = send(clientSocket, send_buf, send_len, 0);
    if(sent < 0) {
        perror("send call");
		exit(-1);
    }
    
    seq_num++;

    //printf("Amount of data sent is: %d\n", sent);
}

void clientMessageReceive(int clientSocket, char *buf, int msgLen) {
	char *clientHandle, *destHandle, *message, *orig = buf;
	int handleLength, destLength = (int) *(buf + 5);
		
	destHandle = malloc(destLength + 1); 
	
	memcpy(destHandle, buf + 6, destLength);
	destHandle[destLength] = '\0';
	
	buf += 6 + destLength;
	
	handleLength = (int) *buf;
	
	clientHandle = malloc(handleLength + 1);
	memcpy(clientHandle, buf + 1, handleLength);
	clientHandle[handleLength] = '\0';
	
	buf += 1 + handleLength;
	
	message = malloc(msgLen - 7 - handleLength - destLength);
	strcpy(message, buf);
		
	if(checkForHandle(destHandle) > -1) {
		sendMessageOk(clientSocket, handleLength, clientHandle);
		sendClientMessage(destHandle, orig, msgLen);
	} 
	else {
		sendMessageError(clientSocket, destLength, destHandle);
	}
}

void clientBroadcastReceive(int clientSocket, char *buf, int msgLen) {
	char *clientHandle, *message, *orig = buf;
	int handleLength = (int) *(buf + 5);
		
	clientHandle = malloc(handleLength + 1); 
	
	memcpy(clientHandle, buf + 6, handleLength);
	clientHandle[handleLength] = '\0';
	
	buf += 6 + handleLength;

	message = malloc(TEXT_MAX);
	strcpy(message, buf);
		
	sendBroadcastToAll(clientSocket, orig, msgLen);
}

void clientExitReceive(int clientSocket, char *buf, int msgLen) {
	char *packet, *ptr;
	
	int packetLength = 5;
	
	packet = malloc(packetLength);
	ptr = packet;
		
	*ptr = seq_num;
	ptr += 4;
	
	*ptr = SERVER_EXIT_ACK;
		
	sendPacket(clientSocket, packet, packetLength);
}

void clientListReceive(int clientSocket, char *buf, int msgLen) {
	sendList(clientSocket);
}

void sendList(int clientSocket) {
	int ndx;
	
	sendListCount(clientSocket);
	
	for(ndx = 0; ndx < numLinks; ndx++) {
		sendListHandle(clientSocket, links[ndx].handle);
	}
}

void sendListCount(int clientSocket) {
	char *packet, *ptr;
	
	int packetLength = 5 + 4;
	
	packet = malloc(packetLength);
	ptr = packet;
		
	*ptr = seq_num;
	ptr += 4;
	
	*ptr = SERVER_LIST;
	ptr += 1;
	
	*ptr = numLinks;
		
	sendPacket(clientSocket, packet, packetLength);
}

void sendListHandle(int clientSocket, char *handle) {
	char *packet, *ptr;
	int handleLength = strlen(handle);
	
	int packetLength = 1 + handleLength;
	
	packet = malloc(packetLength);
	ptr = packet;
	
	memset(ptr, handleLength, 1);
	ptr += 1;
	
	memcpy(ptr, handle, handleLength);
	
	sendPacket(clientSocket, packet, packetLength);
}

void sendClientMessage(char *dest_handle, char *packet, int packet_len) {
	int index = checkForHandle(dest_handle);
	int dest_socket = links[index].socket;
	
	sendPacket(dest_socket, packet, packet_len);
}

void sendBroadcastToAll(int sender_socket, char *packet, int packet_len) {
	int ndx;
	for(ndx = 0; ndx < numLinks; ndx++) {
		if(links[ndx].socket != sender_socket) {
			sendPacket(links[ndx].socket, packet, packet_len);
		}
	}
}

void sendMessageOk(int clientSocket, int handleLength, char *clientHandle) {
	char *packet, *ptr;
	
	int packetLength = 5 + 1 + handleLength;
	
	packet = malloc(packetLength);
	ptr = packet;
		
	*ptr = seq_num;
	ptr += 4;
	
	memset(ptr, SERVER_MESSAGE_GOOD, 1);
	ptr += 1;
	
	memset(ptr, handleLength, 1);
	ptr += 1;
	
	memcpy(ptr, clientHandle, handleLength);
	
	sendPacket(clientSocket, packet, packetLength);
}
void sendMessageError(int clientSocket, int handleLength, char *clientHandle) {
	char *packet, *ptr;
	
	int packetLength = 5 + 1 + handleLength;
	
	packet = malloc(packetLength);
	ptr = packet;
		
	//memset(ptr, seq_num, 4);
	*ptr = seq_num;
	ptr += 4;
	
	memset(ptr, SERVER_MESSAGE_ERROR, 1);
	ptr += 1;
	
	memset(ptr, handleLength, 1);
	ptr += 1;
	
	memcpy(ptr, clientHandle, handleLength);
	
	sendPacket(clientSocket, packet, packetLength);
}

void initialPacket(int clientSocket, char *msgBuff, int msgLen) {
	char *clientHandle;
	int handleLength = (int) *(msgBuff + 3), socketIndex, linkIter;
	clientHandle = (char *) malloc(handleLength + 1);
	
	memcpy(clientHandle, msgBuff + 4, handleLength);
	clientHandle[handleLength] = '\0';
	//printf("Check 1\n");
		
	/* if(checkForHandle(clientHandle) > -1) {
		sendErrorHandle(clientSocket);
	} 
	else {
		int index = findClient(clientSocket);
		//clients[index].handle = (char *) malloc(handleLength + 1);
		links[index].handle = clientHandle;
		strcpy(links[index].handle, clientHandle);
		
		sendConfirmGoodHandle(clientSocket);
		//printf("Confirmation Handle Sent\n");*/
	if (checkForHandle(clientHandle) == -1) {
		for(linkIter = 0; linkIter < numLinks; linkIter++) {
			if(links[linkIter].socket == clientSocket) {
				socketIndex = linkIter;
				break;
			}
		}
		links[linkIter].handle = clientHandle;
		printf("Check 2 socketIndex is %d\n",socketIndex);
		strcpy(links[socketIndex].handle, clientHandle);
		printf("Check 3\n");
		send3Flag(clientSocket);
		//printf("COnfirmaiton sent");

	} else {

		send2Flag(clientSocket);
	}
}

int findClient(int socket) {
	int index;
	
	for(index  = 0; index < numLinks; index++) {
		if(links[index].socket == socket) {
			return index;
		}
	}
	
	return -1;
}

void send2Flag(int clientSocket) {
	struct chatHeader * cHeader = (struct chatHeader *)malloc(sizeof(struct chatHeader));
	cHeader->length = 3;
	cHeader->flag = 3;

	if((send(clientSocket, (char *) cHeader, 3, 0)) < 0) {
		perror("Error on send call");
		exit(-1);
	}
}

void send3Flag(int clientSocket) {
	struct chatHeader * cHeader = (struct chatHeader *)malloc(sizeof(struct chatHeader));
	cHeader->length = 3;
	cHeader->flag = 2;

	if((send(clientSocket, (char *) cHeader, 3, 0)) < 0) {
		perror("Error on send call");
		exit(-1);
	}
}

int checkForHandle(char *handle) {
	int ndx;
	for(ndx = 0; ndx < numLinks; ndx++) {
		if(links[ndx].handle && strcmp(links[ndx].handle, handle) == 0) {
			return ndx;
		}
	}
	return -1;
}

/*void sendConfirmGoodHandle(int clientSocket) {
	char *packet, *ptr;
	printf("send COnfirmaiton\n");
	

	int packetLength = 3;
	
	packet = malloc(packetLength);
	ptr = packet;
		
	//memset(ptr, seq_num, 4);
	*ptr = seq_num;
	ptr += 4;
	
	memset(ptr, SERVER_INITIAL_GOOD, 1);
	printf("sendConfirmation 2\n");
	
	sendPacket(clientSocket, packet, packetLength);
}*/

/*void sendErrorHandle(int clientSocket) {
	char *packet, *ptr;
		
	int packetLength = 5;
	
	packet = malloc(packetLength);
	ptr = packet;
		
	//memset(ptr, seq_num, 4);
	*ptr = seq_num;
	ptr += 4;
	
	memset(ptr, SERVER_INITIAL_ERROR, 1);
	
	sendPacket(clientSocket, packet, packetLength);
}*/

void clientExit(int clientSocket) {
	int index = findClient(clientSocket);
	int i;
	
	for(i = index; i < numLinks + 1; i++) {
		links[i] = links[i + 1];
	}	
	
	numLinks--;	
}

