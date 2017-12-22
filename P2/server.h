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

#define CLIENT_INITIAL 1
#define SERVER_INITIAL_GOOD 2
#define SERVER_INITIAL_ERROR 3
#define CLIENT_BROADCAST 4
#define CLIENT_MESSAGE 5
#define SERVER_MESSAGE_GOOD 6
#define SERVER_MESSAGE_ERROR 7
#define CLIENT_EXIT 8
#define SERVER_EXIT_ACK 9
#define CLIENT_LIST 10
#define SERVER_LIST 11

#define TEXT_MAX 1000
#define BUFFSIZE 1024

void send2Flag(int);
void send3Flag(int);
void initBuffs();
void sendPacket();
//int max_client_socket();
void processMsg(int);
void selectSocket();
int getNewClient();
void initialPacket(int, char *, int);
void clientMessageReceive(int, char *, int);
void clientBroadcastReceive(int, char *, int);
void clientExitReceive(int, char *, int);
void clientListReceive(int, char *, int);
void sendConfirmGoodHandle(int);
void sendErrorHandle(int);
void sendClientMessage(char *, char *, int);
void sendBroadcastToAll(int, char *, int);
void sendList(int);
void sendListCount(int);
void sendListHandle(int, char *);
void sendMessageOk(int, int, char *);
void sendMessageError(int, int ,char *);
int checkForHandle(char *);
int findClient(int);
void clientExit(int);

struct chatHeader {
	uint16_t length;
	uint8_t flag;
} __attribute__((packed));

struct chatLink {
	int socket;
	char *handle;
};

// for the server side
int setUpServer(int);
void tcp_listen();