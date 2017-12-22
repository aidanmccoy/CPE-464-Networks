#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netdb.h>

#define BUFFSIZE 1024

struct chatHeader {
	uint16_t data;
	uint8_t flag;
} __attribute__((packed));

int setUpClient(char * hostName, char * port) {
	int sk;
	struct sockaddr_in clientSocket;
	struct hostent *host;

	if ((sk = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Error on socket call");
		exit(-1);
	}

	clientSocket.sin_family = AF_INET;

	if ((host = gethostbyname(hostName)) == NULL) {
		printf("Error getting hose name at %s\n", hostName);
		exit(-1);
	}

	memcpy((char *) &clientSocket.sin_addr, (char *) host->h_addr, host->h_length);

	clientSocket.sin_port = htons(atoi(port));

	if (connect(sk, (struct sockaddr *) &clientSocket, sizeof(struct sockaddr_in)) < 0) {
		perror("Error on connect call");
		exit(-1);
	}

	return sk;
}

void send1Flag(char * handle, int handleLen, int socket) {
	char * msgBuff = malloc(BUFFSIZE);
	uint16_t data = 0;
	struct chatHeader * cHeader = (struct chatHeader *)msgBuff;
	
	cHeader->data = data;
	cHeader->flag = 1;
	
	*(msgBuff + 3) = handleLen;
	strncpy((msgBuff + 4), handle, handleLen);

	if ((send(socket, msgBuff, (handleLen + 4), 0)) < 0) {
		perror("Error in send call");
		exit(-1);
	} 

}

int checkHandle(int socket) {
	char * msgBuff = malloc(BUFFSIZE);
	struct chatHeader * cHeader;
	if ((recv(socket, msgBuff, BUFFSIZE, 0)) < 0) {
		perror("Error on recv call");
		exit(-1);
	}	
	cHeader = (struct chatHeader *)msgBuff;
	if(cHeader->flag == 3) {
		printf("Initial packet error, the handle already exists\n");
		exit(1);
	}
	if(cHeader->flag != 2) {
		printf("Unknown error occurred on first packet");
		exit(1);
	}
	return 1;
}

int main(int argc, char *argv[]) {
	int handleLen, socket, validHandle;
//	char * msgBuff = malloc(BUFFSIZE);

	if (argc != 4) {
		printf("Wrong number of arguements\n");
		exit(-1);
	}

	if ((handleLen = strlen(argv[1])) > 100) {
		printf("Handle length is too long\n");
		exit(-1);
	}

	socket = setUpClient(argv[2], argv[3]);
	printf(">Socket Set up\n");
	send1Flag(argv[1], handleLen, socket);
	printf(">Flag 1 sent\n");
	validHandle = checkHandle(socket);
	printf("Valid handle is %d\n", validHandle);
	printf("Setup with socket %d", socket);

	return 1;
}
