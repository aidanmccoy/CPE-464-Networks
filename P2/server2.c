#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/types.h>


struct connection {
	int socket;
	char handle[100];
};

struct chatHeader {
	uint16_t data;
	uint8_t flag;
} __attribute__((packed));
