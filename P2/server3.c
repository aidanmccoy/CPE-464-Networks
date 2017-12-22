#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>

#define STARTSIZE 10
#define BUFFER_SIZE 1024
#define	HEADER_SIZE 3

struct connection {
	int socket;
	char handle[100];	
};

struct chatHeader {
	uint16_t data;
	uint8_t flag; 
} __attribute__ ((packed));

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


int selectSocket(struct connection * connections, int numConnections) {
	struct timeval timeVal;
	fd_set readySks;
	int skIter = 0, numReady;

	FD_ZERO(&readySks);
	while (skIter < numConnections) {
		FD_SET(connections[skIter].socket, &readySks);
		skIter++;
	}

	timeVal.tv_sec = 0;
	timeVal.tv_usec = 0;

	if((numReady = select((skIter + 1), &readySks, NULL, NULL, &timeVal)) < 0) {
		perror("Error on select call\n");
		exit(-1);
	}
	skIter = 0;
	while (skIter < numConnections) {
		if (FD_ISSET(connections[skIter].socket, &readySks)) {
			return skIter;
		}
	}
	return 0;
}


void newClient(int serverSocket, struct connection ** connections, int * numConnections, int * connArrayLength) {
	printf(">>New Client 0");
	int newSocket = 0;
	void * tempArray;
	printf(">>New Client 1");
	if((newSocket = accept(serverSocket, (struct sockaddr * )0, (socklen_t *) 0)) < 0) {
		perror("Error in accept call");
		exit(-1);
	}	
	printf(">>New CLient 2\n");
	if (*numConnections == *connArrayLength) {
		tempArray = malloc((sizeof(struct connection) * (*numConnections * 2)));
		memcpy(tempArray, (void *)*connections, (*numConnections * sizeof(struct connection)));
		*connections = (struct connection *) tempArray;
		*connArrayLength = (*connArrayLength * 2);
	}

	(*connections)[*numConnections].socket = newSocket;
	(*numConnections)++;
}

int recieveMsg(struct connection * connections, char ** msgBuffer, int socket) {
	int messageLen;
	if((messageLen = recv(connections[socket].socket, *msgBuffer, BUFFER_SIZE, 0)) < 0) {
		perror("Error in recv call\n");
		exit(-1);
	}
	return messageLen;
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
	cHeader.flag = 3;
	cHeader.data = 0;

	if ((send(socket, (char *) &cHeader, 2, 0)) < 0){
		perror("Error on send call");
		exit(-1);
	}
}

void addNewHandle(char *handle, struct connection * connections, int numConnections, int socket) {
	int handleLen = *handle++, connectionsIter = 0, flag3Sent = 0;

	printf("Handle Len is %d\n", handleLen);

	while (connectionsIter < numConnections) {
		if ((strcmp(connections[connectionsIter].handle, handle)) == 0) {
			sendFlag3(socket);
			flag3Sent = 1;
			connectionsIter++;
		}
	}
	if (flag3Sent == 0) {
		strncpy(connections[socket].handle, handle, handleLen);
		connections[socket].handle[handleLen] = '\0';
		sendFlag2(socket);
	}
}

void processMsg(struct connection * connections, int numConnections, char **msgBuffer, int socket){
	struct chatHeader *cHeader;

	cHeader = (struct chatHeader *) *msgBuffer;
	switch (cHeader->flag) {
		case 1:
			addNewHandle((char *)cHeader + HEADER_SIZE, connections, numConnections, socket);
		break;

		case 2:
		break;

		case 3:
		break;

		case 4:
		break;

		case 5:
		break;

		case 7:
		break;

		case 8:
		break;

		case 9:
		break;

		case 10:
		break;

		case 11:
		break;

		case 12:
		break;
	}

}


int main(int argc, char const* argv[]) {
	int serverSocket, numConnections = 0, readySk, connArrayLength = STARTSIZE, messageLen;
	struct connection * connections = malloc(sizeof(struct connection) * STARTSIZE);
	char * msgBuffer = (char *)malloc(BUFFER_SIZE);

	if (argc == 2) {
		serverSocket = setUpServer(atoi(argv[1]));
	} else {
		serverSocket = setUpServer(0);
	}

	while(1) {
		printf(">While Loop 1\n");
		while ((readySk = selectSocket(connections, numConnections)) < 0);
		printf(">While Loop 2\n");
		if (readySk == 0) {
			printf(">While Loop 3\n");
	
			newClient(serverSocket, &connections, &numConnections, &connArrayLength);
			printf(">While Loop 4\n");
		} else {
			messageLen = recieveMsg(connections, &msgBuffer, readySk);
			printf("MessageLen1 is %d\n", messageLen);
			processMsg(connections, numConnections, &msgBuffer, readySk);
		}
		printf(">While loop is running\n");
	}
	return 1;
}
