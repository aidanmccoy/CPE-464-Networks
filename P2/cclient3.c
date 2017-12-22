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

int clientSocket, handleLen;
char * clientHandle;
char * msgBuff;

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
void recieveInitFlag(){
	if ((recv(clientSocket, msgBuff, BUFFSIZE, 0)) < 0) {
		perror("Error on recv call");
		exit(-1);
	}
	if (msgBuff[2] == 3) {
		printf("Handle already in use");
		exit(-1);
	} else if (msgBuff[2] == 2) {
		printf("Flag 2 returned\n");
		return;
	} else {
		printf("Unknown response flag");
		exit(-1);
	}
}


void sendFlag1() {
	char * packet = (char *) malloc(4 + handleLen);
	struct chatHeader * cHeader = (struct chatHeader *) packet;

	cHeader->flag = 1;		
	packet[4] = handleLen;

	if((send(clientSocket, packet, (handleLen + 4), 0)) < 0) {
		perror("Error on send call");
		exit(-1);
	}
	printf("Flag1 Sent\n");
	recieveInitFlag();
}

void initClientHandle(char * handle) {
	clientHandle = malloc(strlen(handle) + 1);
	strcpy(clientHandle, handle);
}
int main(int argc, char *argv[]) {

	if (argc != 4) {
		printf("Wrong number of arguements\n");
		exit(-1);
        }
        if ((handleLen = strlen(argv[1])) > 100) {
                printf("Handle length is too long\n");
                exit(-1);
        }
	msgBuff = malloc(BUFFSIZE);
	initClientHandle(argv[1]);
	
	clientSocket = setUpClient(argv[2], argv[3]);

	sendFlag1();
			

	return 0;	
}






