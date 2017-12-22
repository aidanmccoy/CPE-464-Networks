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

#include "cclient.h"
//#include "networks.h"
#include "testing.h"


char *clientHandle;
char *serverName;
int sockets[2];
int sdin;
int socketNum;
int seq_num = 0;
int serverSocket;
char *msgBuff;              //buffer for receiving from client
int buffer_size = 1024;  //packet size variable

int main(int argc, char * argv[])
{
    initBuff(argv[1], argv[2]);
    //create packet buffer
    //msgBuff = (char *) malloc(buffer_size);
    checkCmdLineArgs(argc);
   /* * check command line arguments
    if(argc != 4) {
        printf("Invalid cmd line arguements");
		exit(-1);
    }*/
	
	/*/*	store client handle 
	clientHandle = malloc(strlen(argv[1]) + 1);
	strcpy(clientHandle, argv[1]);
	
	*	store client handle
	serverName = malloc(strlen(argv[2]) + 1);
	strcpy(serverName, argv[2]);*/
	
    /* set up the socket for TCP transmission  */
    socketNum = setUpTCP(argv[2], argv[3]);

	//	Set up sockets to select() on: stdin and server_socket
    /*sockets[0] = 2;
    sockets[1] = socketNum;*/
    sdin = 2;
    serverSocket = socketNum;
        
    initialPacket();
    
    printf("$: ");
    fflush(stdout);
    
    while(1) {
    	tcp_select();
    }
        
    close(socketNum);
    return 0;
}
void checkCmdLineArgs(int argc) {
	if(argc != 4) {
		printf("Invalid CMD line arguements\n");
	}
}

void initBuff(char * handle, char * hostname) {
	msgBuff = (char *)malloc(BUFFSIZE);

	clientHandle = malloc(strlen(handle) + 1);
	strcpy(clientHandle, handle);

	serverName = malloc(strlen(hostname) + 1);
	strcpy(serverName, hostname);
}

int setUpTCP(char *hostname, char *port)
{
    int sk;
    struct sockaddr_in client;       
    struct hostent *host;              

    if ((sk = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	    perror("socket call");
	    exit(-1);
	}

    client.sin_family = AF_INET;

    if ((host = gethostbyname(hostname)) == NULL) {
	  printf("Error getting hostname: %s\n", hostname);
	  exit(-1);
	}
    
    memcpy((char*)&client.sin_addr, (char*)host->h_addr, host->h_length);
    client.sin_port= htons(atoi(port));

    if(connect(sk, (struct sockaddr*)&client, sizeof(struct sockaddr_in)) < 0) {
		perror("connect call");
		exit(-1);
    }

    return sk;
}

void tcp_select() {
	fd_set fdvar;
	
	FD_ZERO(&fdvar); // reset variables

	FD_SET(sdin, &fdvar);
	FD_SET(serverSocket, &fdvar);
		
	if(select(socketNum + 1, (fd_set *) &fdvar, NULL, NULL, NULL)  < 0) {
		perror("select call");
		exit(-1);
	}
	
	if(FD_ISSET(sdin, &fdvar)) {
		char* input_buf = (char *) malloc(buffer_size);
    	int input_len = 0;
    	    	
    	while((input_buf[input_len] = getchar()) != '\n')
    		input_len++;
    	input_buf[input_len] = '\0';
    	
    	if(input_buf[0] != '%') {
    		printf("Invalid command\n");
    	}
    	else if(input_buf[1] == 'M' || input_buf[1] == 'm') {
    		message(input_buf);
    	}
    	else if(input_buf[1] == 'B' || input_buf[1] == 'b') {
    		broadcast(input_buf);
    		
    		printf("$: ");
    		fflush(stdout);
    	}
    	else if(input_buf[1] == 'L' || input_buf[1] == 'l') {
    		sendListPacket();
    		
    		printf("$: ");
    		fflush(stdout);
    	}
    	else if(input_buf[1] == 'E' || input_buf[1] == 'e') {
    		sendExitPacket();
    	}
    	else {
    		printf("Invalid command\n");
    	} 
	}
	
	if(FD_ISSET(sockets[1], &fdvar)) {
		tcp_receive();
	}	
}

void tcp_receive() {
	int message_len;
	
	//now get the data on the server_socket
    if ((message_len = recv(socketNum, msgBuff, buffer_size, 0)) < 0) {
		perror("recv call");
		exit(-1);
    }
    
    if(message_len == 0) {
    	printf("Server terminated\n");
    	exit(-1);
    }
    
    switch(msgBuff[4]) {
    	case 2:
    		printf("$: ");
    		fflush(stdout);
    		break;
    	case 3: 
    		serverMessageError(socketNum, msgBuff, message_len);
    		
    		printf("$: ");
    		fflush(stdout);
    		break;
    	case 9: 
    		clientExit();
    		break;
    	case 10:
    		receiveList(socketNum, msgBuff, message_len);
    		break;
    	case 5:
    		receiveMessagePacket(socketNum, msgBuff, message_len);
    		
    		printf("$: ");
    		fflush(stdout);
    		break;
    	case 4:
    		receiveBroadcastPacket(socketNum, msgBuff, message_len);
    		
    		printf("$: ");
    		fflush(stdout);
    		break;
    	default:
    		printf("some other flag\n");
    }  
}

void initialPacket() {
	char *packet, *ptr;

	int handleLen = strlen(clientHandle);
	int headerData = htons(handleLen);

	int packetLen = 3 + 1 + handleLen;
	//int returnMsgLen;

	packet =  malloc(packetLen);
	ptr = packet;

	*ptr = headerData;
	ptr+= 2;

	memset(ptr,1 ,1);
	ptr+= 1;

	memset(ptr, handleLen, 1);
	ptr+= 1;

	memcpy(ptr, clientHandle, handleLen);

	if((send(socketNum, packet, packetLen, 0)) < 0) {
		perror("Error in the send call");
		exit(-1);
	}


	/*int senderHandleLen = strlen(clientHandle);
	int seqNum = htons(seq_num);
	
	int packetLength = 5 + 1 + senderHandleLen;
	
	packet = malloc(packetLength);
	ptr = packet;
		
	//memset(ptr, seq_num, 4);
	*ptr = seqNum;
	ptr += 4;
	
	memset(ptr, 1, 1);
	ptr += 1;
	
	memset(ptr, senderHandleLen, 1);
	ptr += 1;
	
	memcpy(ptr, clientHandle, senderHandleLen);
	ptr += senderHandleLen;
	
	sendPacket(packet, packetLength);*/

	if ((recv(socketNum, msgBuff, buffer_size, 0)) < 0) {
		perror("recv call");
		exit(-1);
    } 
    if (msgBuff[2] == 2) {
    	printf("Confirmation packet recieved\n");
    } else if(msgBuff[2] == 3) {
    	printf("Handle is already being used\n");
    	close(socketNum);
    	exit(-1);
    } else {
    	printf("Unknown response flag");
    }
	//initialReceive();
}

/*void initialReceive() {
	int message_len;
	
	//now get the data on the server_socket
    if ((message_len = recv(socketNum, msgBuff, buffer_size, 0)) < 0) {
		perror("recv call");
		exit(-1);
    }
    
    switch(msgBuff[2]) {
    	case 2:
    		printf("Confirmation packet recieved\n");
    		break;
    	case 3: 
    		printf("Handle already in use: %s\n", clientHandle);
    		clientExit();
    		break;
    	default:
    		printf("some other flag\n");
    }  
}*/

void message(char *input) {
	char 

	/*char * command , *handle, *text, *orig;
	int handleLength = 0;
	
	orig = malloc(strlen(input) + 1);
	
	strcpy(orig, input);
	
	command = strtok(input, " ");
	printf("Command is %s\n",command);
	handle = strtok(NULL, " ");
	
	if(handle == NULL) {
		printf("Error, no handle given\n");
	}
	else {
		if(strtok(NULL, " ") == NULL) {
			text = "";
		}
		else {
			handleLength = strlen(handle);
			text = orig + 4 + handleLength;
		}
	
		if(strlen(text) > 1000) {
			printf("Error, message to long, message length is: %d\n", (int)strlen(text));
			
			printf("$: ");
			fflush(stdout);
		} 
		else {
			sendMessagePacket(handle, text);
		}
	
		
	} */
}

void broadcast(char *input) {
	char *command, *text, *orig;
	
	orig = malloc(strlen(input) + 1);
	
	strcpy(orig, input);
	
	command = strtok(input, " ");
	printf("Command is %s\n", command);
	
	if(strtok(NULL, " ") == NULL) {
		text = "";
	}
	else {
		text = orig + 3;
	}
	
	if(strlen(text) > 1000) {
		printf("Error, message to long, message length is: %d\n", (int)strlen(text));
	}
	else {
		sendBroadcastPacket(text);
	}
}

void sendMessagePacket(char *destinationHandle, char *text) {
	char *packet, *ptr;

	int numHandles
	
	/*int destHandleLen = strlen(destinationHandle);
	int senderHandleLen = strlen(clientHandle);
	int textLen = strlen(text) + 1;
	int seqNum = htons(senderHandleLen);
	
	int packetLength = 3 + 1 + destHandleLen + 1 + senderHandleLen + textLen;
	
	packet = malloc(packetLength);
	ptr = packet;
		
	//memset(ptr, seq_num, 4);
	*ptr = seqNum;
	ptr += 2;
	
	memset(ptr, 5, 1);
	ptr += 1;
	
	memset(ptr, destHandleLen, 1);
	ptr += 1;
	
	memcpy(ptr, destinationHandle, destHandleLen);
	ptr += destHandleLen;
	
	memset(ptr, senderHandleLen, 1);
	ptr += 1;
	
	memcpy(ptr, clientHandle, senderHandleLen);
	ptr += senderHandleLen;
	
	memcpy(ptr, text, textLen);
	
	printf("Sending packet flag 5\n");
	sendPacket(packet, packetLength);*/
}

void serverMessageError(int socket, char *msgBuff, int message_len) {
	char *handle;
	int handleLength = (int) *(msgBuff + 5);
	
	handle = malloc(handleLength + 1);
	memcpy(handle, msgBuff + 6, handleLength);
	handle[handleLength] = '\0';
	
	
	printf("Client with handle %s does not exist\n", handle);
}

void receiveMessagePacket(int socket, char *msgBuff, int message_len) {
	char *clientHandle, *destHandle, *message;
	int handleLength, destLength = (int) *(msgBuff + 5);
		
	destHandle = malloc(destLength + 1); 
	
	memcpy(destHandle, msgBuff + 6, destLength);
	destHandle[destLength] = '\0';
	
	msgBuff += 6 + destLength;
	
	handleLength = (int) *msgBuff;
	
	clientHandle = malloc(handleLength + 1);
	memcpy(clientHandle, msgBuff + 1, handleLength);
	clientHandle[handleLength] = '\0';
	
	msgBuff += 1 + handleLength;
	
	message = malloc(message_len - 7 - handleLength - destLength);
	strcpy(message, msgBuff);
		
	printf("\n%s: %s\n", clientHandle, message);
}

void receiveBroadcastPacket(int socket, char *msgBuff, int message_len) {
	char *clientHandle, *message;
	int handleLength = (int) *(msgBuff + 5);
		
	clientHandle = malloc(handleLength + 1); 
	
	memcpy(clientHandle, msgBuff + 6, handleLength);
	clientHandle[handleLength] = '\0';
	
	msgBuff += 6 + handleLength;
	
	message = malloc(message_len - 7 - handleLength);
	strcpy(message, msgBuff);
		
	printf("\n%s: %s\n", clientHandle, message);
}

void receiveList(int socket, char *msgBuff, int message_len) {
	int handleCount;
	msgBuff += 5;
	
	memcpy(&handleCount, msgBuff, 4);
	
	//printf("There are %d handles\n", handleCount);
	printf("\n");
	int count;
	for(count = 0; count < handleCount; count++) {
		receiveHandle(count);
	}
	
	printf("$: ");
	fflush(stdout);
}

void receiveHandle(int count) {
	int message_len, handleLen;
	char *handle;
	
	//now get the data on the server_socket
    if ((message_len = recv(socketNum, msgBuff, buffer_size, 0)) < 0) {
		perror("recv call");
		exit(-1);
    }
    
    handleLen = (int) *msgBuff;
    handle = malloc(handleLen + 1);
    memcpy(handle, msgBuff + 1, handleLen);
    handle[handleLen] = '\0';
    printf("%s\n", handle);
}

void sendBroadcastPacket(char *text) {
	char *packet, *ptr;
	
	int senderHandleLen = strlen(clientHandle);
	int textLen = strlen(text) + 1;
	int seqNum = htons(seq_num);
	
	int packetLength = 5 + 1 + senderHandleLen + textLen;
	
	packet = malloc(packetLength);
	ptr = packet;
		
	//memset(ptr, seq_num, 4);
	*ptr = seqNum;
	ptr += 4;
	
	memset(ptr, 4, 1);
	ptr += 1;
	
	memset(ptr, senderHandleLen, 1);
	ptr += 1;
	
	memcpy(ptr, clientHandle, senderHandleLen);
	ptr += senderHandleLen;
	
	strcpy(ptr, text);
	
	sendPacket(packet, packetLength);
}

void sendListPacket() {
	char *packet, *ptr;
	
	int seqNum = htons(seq_num);
	
	int packetLength = 5;
	
	packet = malloc(packetLength);
	ptr = packet;
		
	//memset(ptr, seq_num, 4);
	*ptr = seqNum;
	ptr += 4;
	
	memset(ptr, 10, 1);
	ptr += 1;

	sendPacket(packet, packetLength);
}

void sendExitPacket() {
	char *packet, *ptr;
	
	int seqNum = htons(seq_num);
	
	int packetLength = 5;
	
	packet = malloc(packetLength);
	ptr = packet;
		
	*ptr = seqNum;
	ptr += 4;
	
	memset(ptr, 8, 1);
	ptr += 1;

	sendPacket(packet, packetLength);
}

void sendPacket(char *send_buf, int send_len) {
	int sent;
	seq_num++;	
	
	sent = send(socketNum, send_buf, send_len, 0);
    if(sent < 0) {
        perror("send call");
		exit(-1);
    }

    //printf("Amount of data sent is: %d\n", sent);
}

void clientExit() {
	close(socketNum);
    exit(-1);
}

