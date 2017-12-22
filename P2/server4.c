#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/select.h>
#include <unistd.h>

#define START_SIZE 10
#define MSG_BUF_SIZE 1024
#define HEADERSIZE 3

struct connection {
	int socket;
	char *handle;
};

struct chatHeader {
	uint16_t data;
	uint8_t flag;
} __attribute__((packed));

int serverSocket = 0;
int numConnections = 0;
struct connection * connections;
char * msgBuff;
int connectionsArraySize = START_SIZE;

int setUpServer(int socketNumber) {
        int sk;
        struct sockaddr_in serverSocket;
        socklen_t len = sizeof(serverSocket);

        if ((sk = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                perror("Error on socket call\n");
                exit(-1);
        }

        serverSocket.sin_family = AF_INET;
        serverSocket.sin_addr.s_addr = INADDR_ANY;
        serverSocket.sin_port = htons(socketNumber);

        if (bind(sk, (struct sockaddr *) &serverSocket, sizeof(serverSocket)) < 0) {
                perror("Error on bind call\n");
                exit(-1);
        }

        if (getsockname(sk, (struct sockaddr *) &serverSocket, &len) < 0) {
                perror("Error on getsockname call\n");
                exit(-1);
        }

        printf(">>Server is using port %d\n", ntohs(serverSocket.sin_port));

        if (listen(sk, 5) < 0) {
                perror("Error on listen call\n");
                exit(-1);
        }

        printf(">>setUpServerComplete\n");

        return sk;
}

void initBuffers() {
	connections = malloc((sizeof(struct connection) * START_SIZE));
	msgBuff = (char *)malloc(MSG_BUF_SIZE);	
}

void checkForNewClient(fd_set * readySks) {
	int newClientSocket;
	void * tempArray;
	printf("Check for new Clients1");
	if(FD_ISSET(serverSocket, readySks)) {
		printf("Check for new clients 2");
		if ((newClientSocket = accept(serverSocket, (struct sockaddr *)0, (socklen_t *)0)) < 0) {
			perror("Error in accept call");
			exit(-1);
		}
		if (numConnections == connectionsArraySize - 2) {
			tempArray = malloc((sizeof(struct connection) * (connectionsArraySize * 2)));
			memcpy(tempArray, (void *) &connections, (numConnections * sizeof(struct connection)));
			connections = (struct connection *) tempArray;
			connectionsArraySize = (connectionsArraySize * 2);
		}	
	}
	connections[numConnections++].socket = newClientSocket;
	printf("COnnection added"); 	
}

void sendFlag3(int socket) {
	static struct chatHeader cHeader;
        cHeader.flag = 3;
        cHeader.data = 0;

        if ((send(socket, (char *) &cHeader, 3, 0)) < 0){
                perror("Error on send call");
                exit(-1);
        }
}

void sendFlag2(int socket) {
   static struct chatHeader cHeader;
        cHeader.flag = 2;
        cHeader.data = 0;

        if ((send(socket, (char *) &cHeader, 3, 0)) < 0){
                perror("Error on send call");
                exit(-1);
        }
}

void checkHandle(char *msgBuff, int packetLen, int clientSocket) {
	int handleLen = (int) *(msgBuff + HEADERSIZE);
	char *handle = malloc(handleLen + 1);
	int connectionIter = 0, invalidHandle = 0;

	memcpy(handle, (msgBuff + 4), handleLen);
	handle[handleLen] = '\0';

	while (connectionIter < numConnections) {
		if (!strcmp(connections[connectionIter].handle, handle)) {
			invalidHandle = 1;
		}
	connectionIter++;
	}
	if (invalidHandle) {
		sendFlag3(clientSocket);
		return;
	} 
	connectionIter = 0;
	while (connectionIter < numConnections) {
		if(connections[connectionIter].socket == clientSocket) {
			connections[connectionIter].handle = handle;
			strcpy(connections[connectionIter].handle, handle);
			sendFlag2(clientSocket);
		}
	connectionIter++;
	}
}

void getPacket(int clientSocket) {
	int packetLen, connectionIter = 0, ndx;
	struct chatHeader * cHeader;
	
	printf("Get Packet 1");
	if ((packetLen = recv(clientSocket, msgBuff, MSG_BUF_SIZE, 0)) < 0) {
		perror("Error in recv call");
		exit(-1);
	}
	if (packetLen > 0) {
		cHeader = (struct chatHeader *)msgBuff;
		switch(cHeader->flag) {
			case 1: //Initial Packeti
				printf("case 1 recieved");
				checkHandle(msgBuff, packetLen, clientSocket);
			break;
			
			case 4: //%B
			break;
	
			case 5: //%M
			break;
 
		}
	} else {
		while (connectionIter < numConnections) {
			if(connections[connectionIter].socket == clientSocket) {
				break;
			}
		connectionIter++;
		}
		for(ndx = connectionIter; ndx < numConnections + 1; ndx++) {
			connections[ndx] = connections[ndx + 1];
		}
		numConnections--;	
	}
}

int maxSocket() {

	int max = serverSocket, i;
	for(i = 0; i < numConnections; i++) {
		if(connections[i].socket > max) max = connections[i].socket;
	}
	return max;
}

void selectSocket() {
	printf("Select 1\n");
        fd_set readySks;
        int skIter = 0;
	printf("Select 2\n");
        FD_ZERO(&readySks);

	FD_SET(serverSocket, &readySks);
	printf("Select 3\n");
       /* while (skIter < numConnections) {
                FD_SET(connections[skIter].socket, &readySks);
                skIter++;
        }*/
	for (skIter = 0; skIter < numConnections; skIter++) {
		FD_SET(connections[skIter].socket, &readySks);
	}
	printf("Select 4\n");

        if(select(maxSocket() + 1, (fd_set *)&readySks, NULL, NULL, NULL) < 0) {
                perror("Error on select call\n");
                exit(-1);
        }
	printf("Select 5\n");

	checkForNewClient(&readySks);
	
	printf("Select 6\n");
        skIter = 0;
        while (skIter < numConnections) {
		printf("select 7");
                if (FD_ISSET(connections[skIter].socket, &readySks)) {
			printf("Select 8");	
			getPacket(connections[skIter].socket);
                }
	skIter++;
        }
}

int main(int argc, char const* argv[]) {

	initBuffers();	

        if (argc == 2) {
                serverSocket = setUpServer(atoi(argv[1]));
        } else {
                serverSocket = setUpServer(0);
	}
	printf(">Main 1\n");
	while(1){
		selectSocket();
		printf("Main 2\n");
	}	
	return 0;
}






