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

void checkCmdLineArgs();
void initBuff(char *, char *);
void tcp_receive();
void tcp_select();
void sendPacket();
void initialPacket();
void initialReceive();
void message();
void sendMessagePacket();
void sendBroadcastPacket();
void sendListPacket();
void serverMessageError(int, char *, int);
void receiveMessagePacket(int, char *, int);
void receiveBroadcastPacket(int, char *, int);
void receiveList(int, char *, int);
void receiveHandle(int);
void broadcast();
void sendExitPacket();
void clientExit();
void terminate();

int setUpTCP(char *host_name, char *port);

struct chatHeader {
	uint16_t length;
	uint8_t flag;
};





