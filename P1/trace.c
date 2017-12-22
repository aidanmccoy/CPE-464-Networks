/*
Program 1 CPE 464-01 (Lab Noon)
Aidan McCoy Winter 2017
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <pcap.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "checksum.h"


struct ethernetHeader {
	u_char destMAC[6];
	u_char srcMAC[6];
	u_char type[2];
} __attribute__((packed));

struct ARPHeader {
	u_char opCode[2];
	u_char senderMAC[6];
	u_char senderIP[4];
	u_char targetMAC[6];
	u_char targetIP[4];
} __attribute__((packed));

struct IPHeader {
	u_char ipVersion[1];
	u_char headerLen[1];
	u_char diffservBits[1];
	u_char ecnBits[1];
	u_char ttl[1];
	u_char protocol[1];
	u_char Checksum[2];
	u_char senderIP[4];
	u_char destIP[4];
	u_short totalLen[1];
} __attribute__((packed));

struct TCPHeader {
	u_short sourcePort[1];
	u_short destPort[1];
	unsigned int sequenceNumber[1];
	unsigned int ackNumber[1];
	u_char dataOffset[1];
	u_char synFlag[1];
	u_char rstFlag[1];
	u_char finFlag[1];
	u_char ackFlag[1];
	u_short windowSize[1];
	u_short checkSum[1];
} __attribute__((packed));

struct UDPHeader {
	u_short sourcePort[1];
	u_short destPort[1];
} __attribute__((packed));

struct ICMPHeader {
	u_char type[1];
} __attribute__((packed));

struct TCPPsuedoHeader {
	u_int senderIP[1];
	u_int destIP[1];
	u_char reserved[1];
	u_char protocol[1];
	u_short tcpLength[1];
} __attribute__((packed));

//Prints the port name rather than number for specific ports
void printPort(u_short portNumber) {
	int port = htons(portNumber);
	switch (port) {
		case 80:
		printf("HTTP\n");
		break;

		case 53:
		printf("DNS\n");
		break;

		case 23:
		printf("Telnet\n");
		break;

		case 110:
		printf("POP3\n");
		break;

		case 25:
		printf("SMTP\n");
		break;
	default : printf("%d\n", port);
	}
}

//Checksum Method for IP Headers
u_short ipCheckSum(struct IPHeader * ipHeader, u_short * packetData) {
	u_char ipv4[1] = {0x45};
	u_short checksumTotal;

	if (memcmp(ipHeader->ipVersion, ipv4, sizeof(u_char)) == 0) {
		checksumTotal = in_cksum((packetData + 7), 20);
		return checksumTotal;
	} else {
		checksumTotal = in_cksum((packetData + 7), 40);
		return checksumTotal;
	}
	return checksumTotal;
}

//Checksum method for TCP Headers
u_short tcpCheckSum(const u_char * packetData, struct IPHeader * ipHeader, int ipHeaderLen) {
	u_short checksumTotal = -1;
	struct TCPPsuedoHeader * pHeader = malloc(sizeof(struct TCPPsuedoHeader));
	u_char * tcpBuffer;
	int psuedoHeaderLen = sizeof(struct TCPPsuedoHeader);
	int buffLen;


	memcpy((*pHeader).senderIP, (*ipHeader).senderIP, sizeof(int));
	memcpy((*pHeader).destIP, (*ipHeader).destIP, sizeof(int));
	pHeader->reserved[0] = 0x00;
	memcpy((*pHeader).protocol, (*ipHeader).protocol, sizeof(u_char));
	pHeader->tcpLength[0] = ntohs(htons(ipHeader->totalLen[0]) - ipHeaderLen);

	buffLen = psuedoHeaderLen + ntohs(pHeader->tcpLength[0]);

	tcpBuffer = malloc(buffLen);

	memcpy(tcpBuffer, pHeader, sizeof(struct TCPPsuedoHeader));
	memcpy(&tcpBuffer[12], &packetData[ipHeaderLen + 14], ntohs(pHeader->tcpLength[0]));

	checksumTotal = in_cksum((u_short *)tcpBuffer, buffLen);

 	return checksumTotal;
}


int printEthernetHeader(struct ethernetHeader *ethHeader) {
	u_char arp[2] = { 0x08, 0x06 };
	u_char ip[2] = { 0x08, 0x00 };

	printf("\tEthernet Header\n\t\tDest MAC: "
		"%.1x:%.1x:%.1x:%.1x:%.1x:%.1x\n", ethHeader->destMAC[0], ethHeader->destMAC[1],
		ethHeader->destMAC[2], ethHeader->destMAC[3], ethHeader->destMAC[4], 
		ethHeader->destMAC[5]);

	printf("\t\tSource MAC: "
		"%.1x:%.1x:%.1x:%.1x:%.1x:%.1x", ethHeader->srcMAC[0], ethHeader->srcMAC[1],
		ethHeader->srcMAC[2], ethHeader->srcMAC[3], ethHeader->srcMAC[4], 
		ethHeader->srcMAC[5]);

	if (memcmp(ethHeader->type, arp, (sizeof(u_char) * 2)) == 0) {
		printf("\n\t\tType: ARP\n");
		return 1;
	}
	if (memcmp(ethHeader->type, ip, (sizeof(u_char) * 2)) == 0) {
		printf("\n\t\tType: IP\n");
		return 2;
	}
	return -1;
}

void printARPHeader(struct ARPHeader *arpHeader) {
	u_char request[2] = {0x00, 0x01};
	printf("\n\tARP header");

	if (memcmp(arpHeader->opCode, request, (sizeof(u_char) * 2)) == 0){
			printf("\n\t\tOpcode: Request\n");
	} else {
			printf("\n\t\tOpcode: Reply\n");
	}

	printf("\t\tSender MAC: "
		"%.1x:%.1x:%.1x:%.1x:%.1x:%.1x\n", arpHeader->senderMAC[0], arpHeader->senderMAC[1],
		arpHeader->senderMAC[2], arpHeader->senderMAC[3], arpHeader->senderMAC[4], 
		arpHeader->senderMAC[5]);	

	printf("\t\tSender IP: %s\n",inet_ntoa(*(struct in_addr *) &arpHeader->senderIP));	

	printf("\t\tTarget MAC: "
		"%.1x:%.1x:%.1x:%.1x:%.1x:%.1x\n", arpHeader->targetMAC[0], arpHeader->targetMAC[1],
		arpHeader->targetMAC[2], arpHeader->targetMAC[3], arpHeader->targetMAC[4], 
		arpHeader->targetMAC[5]);

	printf("\t\tTarget IP: %s\n\n", inet_ntoa(*(struct in_addr *) &arpHeader->targetIP));	
}

void printTCPHeader(struct TCPHeader *tcpHeader, const u_char *packetData, struct IPHeader *ipHeader, int ipHeaderLen) {
	int offset = *tcpHeader->dataOffset >> 4;
	offset = offset & 0x00f;
	offset = offset * 4;

	printf("\n\tTCP Header\n");
	printf("\t\tSource Port:  ");
	printPort(tcpHeader->sourcePort[0]); 
	printf("\t\tDest Port:  ");
	printPort(tcpHeader->destPort[0]);
	printf("\t\tSequence Number: %u\n", htonl(tcpHeader->sequenceNumber[0]));
	printf("\t\tACK Number: %u\n", htonl(tcpHeader->ackNumber[0]));
	printf("\t\tData Offset (bytes): %d\n", offset);
	
	if (*tcpHeader->synFlag & 0b00000010) {
		printf("\t\tSYN Flag: Yes\n");
	} else {
		printf("\t\tSYN Flag: No\n");
	}

	if (*tcpHeader->rstFlag & 0b00000100) {
		printf("\t\tRST Flag: Yes\n");
	} else {
		printf("\t\tRST Flag: No\n");
	}

	if (*tcpHeader->finFlag & 0b00000001) {
		printf("\t\tFIN Flag: Yes\n");
	} else {
		printf("\t\tFIN Flag: No\n");
	}

	if (*tcpHeader->ackFlag & 0b00010000) {
		printf("\t\tACK Flag: Yes\n");
	} else {
		printf("\t\tACK Flag: No\n");
	}
	printf("\t\tWindow Size: %d\n", htons(tcpHeader->windowSize[0]));

	if (tcpCheckSum(packetData, ipHeader, ipHeaderLen) == 0) {
		printf("\t\tChecksum: Correct (0x%.4x)\n", htons(tcpHeader->checkSum[0]));
	} else {
		printf("\t\tChecksum: Incorrect (0x%.4x)\n", htons(tcpHeader->checkSum[0]));
	}
}

void printUPDHeader(struct UDPHeader *udpHeader) {

	printf("\n\tUDP Header\n");
	printf("\t\tSource Port:  ");
	printPort(udpHeader->sourcePort[0]);
	printf("\t\tDest Port:  ");
	printPort(udpHeader->destPort[0]);
}

void loadTCPHeader(const u_char *packetData, struct IPHeader *ipHeader, int ipHeaderLen) {
	struct TCPHeader *tcpHeader = malloc(sizeof(struct TCPHeader));

	memcpy((*tcpHeader).sourcePort, (packetData + ipHeaderLen + 14), sizeof(u_char) * 2);
	memcpy((*tcpHeader).destPort, (packetData + ipHeaderLen + 16), sizeof(u_char) * 2);
	memcpy((*tcpHeader).sequenceNumber, (packetData + ipHeaderLen + 18), sizeof(u_char) * 4);
	memcpy((*tcpHeader).ackNumber, (packetData + ipHeaderLen + 22), sizeof(u_char) * 4);
	memcpy((*tcpHeader).dataOffset, (packetData + ipHeaderLen + 26), sizeof(u_char));
	memcpy((*tcpHeader).synFlag, (packetData + ipHeaderLen + 27), sizeof(u_char));
	memcpy((*tcpHeader).rstFlag, (packetData + ipHeaderLen + 27), sizeof(u_char));
	memcpy((*tcpHeader).finFlag, (packetData + ipHeaderLen + 27), sizeof(u_char));
	memcpy((*tcpHeader).ackFlag, (packetData + ipHeaderLen + 27), sizeof(u_char));
	memcpy((*tcpHeader).windowSize, (packetData + ipHeaderLen + 28), sizeof(u_char) * 2);
	memcpy((*tcpHeader).checkSum, (packetData + ipHeaderLen + 30), sizeof(u_char) * 2);

	printTCPHeader(tcpHeader, packetData, ipHeader, ipHeaderLen);
}

void loadUDPHeader(const u_char *packetData, int ipHeaderLen) {
	struct UDPHeader *udpHeader = malloc(sizeof(struct UDPHeader));

	memcpy((*udpHeader).sourcePort, (packetData + ipHeaderLen + 14), sizeof(u_char) * 2);
	memcpy((*udpHeader).destPort, (packetData + ipHeaderLen + 16), sizeof(u_char) * 2);

	printUPDHeader(udpHeader);
}

void loadICMPHeader(const u_char *packetData, int ipHeaderLen) {
	u_char typeRequest[1] = {0x08};
	u_char typeReply[1] = {0x00};

	struct ICMPHeader *icmpHeader = malloc(sizeof(struct ICMPHeader));

	memcpy((*icmpHeader).type, (packetData + ipHeaderLen + 14) ,sizeof(u_char));

	printf("\n\tICMP Header\n");

	if (memcmp(icmpHeader->type, typeRequest, sizeof(u_char)) == 0) {
		printf("\t\tType: Request\n");
	} else if (memcmp(icmpHeader->type, typeReply, sizeof(u_char)) == 0 ){
		printf("\t\tType: Reply\n");
	} else {
		printf("\t\tType: %d\n", icmpHeader->type[0]);
	}
}

void printIPHeader(struct IPHeader *ipHeader,const u_char *packetData) {
	u_char tcp[1] = {0x06};
	u_char icmp[1] = {0x01};
	u_char udp[1] = {0x11};
	int protocolType, headerLen, ipVersion;
	
	ipVersion = ((int)*ipHeader->ipVersion & 0x00f0) >> 4;
	headerLen = ((int)*ipHeader->headerLen & 0x000f) * 4;
	
	printf("\n\tIP Header\n");
	printf("\t\tIP Version: %d\n", ipVersion);
	printf("\t\tHeader Len (bytes): %d\n", headerLen);
	printf("\t\tTOS subfields:\n");
	printf("\t\t   Diffserv bits: %d\n", ipHeader->diffservBits[0]);
	printf("\t\t   ECN bits: %d\n", ipHeader->ecnBits[0]);
	printf("\t\tTTL: %u\n", (unsigned int)ipHeader->ttl[0]);

	if (memcmp(ipHeader->protocol, tcp, sizeof(u_char)) == 0) {
		printf("\t\tProtocol: TCP\n");
		protocolType = 1;
	} else if (memcmp(ipHeader->protocol, icmp, sizeof(u_char)) == 0) {
		printf("\t\tProtocol: ICMP\n");
		protocolType = 3;
	} else if (memcmp(ipHeader->protocol, udp, sizeof(u_char)) == 0) {
		printf("\t\tProtocol: UDP\n");
		protocolType = 2;
	} else {
		printf("\t\tProtocol: Unknown\n");
	}

	if (ipCheckSum(ipHeader, (u_short *)packetData) == 0) {
		printf("\t\tChecksum: Correct (0x%.2x%.2x)\n",ipHeader->Checksum[0], ipHeader->Checksum[1]);
	} else {
		printf("\t\tChecksum: Incorrect (0x%.2x%.2x)\n", ipHeader->Checksum[0], ipHeader->Checksum[1]);
	}

	printf("\t\tSender IP: %s\n", inet_ntoa(*(struct in_addr *) &ipHeader->senderIP));
	printf("\t\tDest IP: %s\n", inet_ntoa(*(struct in_addr *) &ipHeader->destIP));	
	
	if (protocolType == 1) {
		loadTCPHeader(packetData, ipHeader, headerLen);
	}
	if (protocolType == 2) {
		loadUDPHeader(packetData, headerLen);
	}
	if (protocolType == 3) {
		loadICMPHeader(packetData, headerLen);
	}
}

int loadEthernetHeader(const u_char *packetData) {
	int packetType = 0;																//Int to set depending on type
	struct ethernetHeader *ethHeader = malloc(sizeof(struct ethernetHeader));	//Ethernet headed struct
	
	memcpy((*ethHeader).destMAC, packetData, sizeof(u_char)*6);
	memcpy((*ethHeader).srcMAC, (packetData + 6), sizeof(u_char)*6);
	memcpy((*ethHeader).type, (packetData + 12), sizeof(u_char)*2);

	packetType = printEthernetHeader(ethHeader);

	free(ethHeader);

	return packetType;
}

void loadARPHeader(const u_char *packetData) {
	struct ARPHeader *arpHeader = malloc(sizeof(struct ARPHeader));

	memcpy((*arpHeader).opCode, (packetData + 20), sizeof(u_char)*2);
	memcpy((*arpHeader).senderMAC, (packetData +22), sizeof(u_char)*6);
	memcpy((*arpHeader).senderIP, (packetData + 28), sizeof(u_char)*4);
	memcpy((*arpHeader).targetMAC, (packetData + 32), sizeof(u_char)*6);
	memcpy((*arpHeader).targetIP, (packetData + 38), sizeof(u_char)*4);

	printARPHeader(arpHeader);
}

void loadIPHeader(const u_char *packetData) {
	struct IPHeader *ipHeader = malloc(sizeof(struct IPHeader));

	memcpy((*ipHeader).ipVersion, (packetData + 14), sizeof(u_char));
	memcpy((*ipHeader).headerLen, (packetData + 14), sizeof(u_char));
	memcpy((*ipHeader).diffservBits, (packetData + 15), sizeof(u_char));
	memcpy((*ipHeader).ecnBits, (packetData + 15), sizeof(u_char));
	memcpy((*ipHeader).ttl, (packetData + 22), sizeof(u_char));
	memcpy((*ipHeader).protocol, (packetData + 23), sizeof(u_char));
	memcpy((*ipHeader).Checksum, (packetData + 24), sizeof(u_char) * 2);
	memcpy((*ipHeader).senderIP, (packetData + 26), sizeof(u_char) * 4);
	memcpy((*ipHeader).destIP, (packetData + 30), sizeof(u_char) * 4);
	memcpy((*ipHeader).totalLen, (packetData + 16), sizeof(u_char) * 2);

	ipHeader->diffservBits[0] = (ipHeader->diffservBits[0] & 0b11111100) >> 2;
	ipHeader->ecnBits[0] = ipHeader->ecnBits[0] & 0b00000011;

	printIPHeader(ipHeader, packetData);
}

int main(int argc, char const *argv[])
{
	pcap_t *pcapPtr;					
	char errbuf[PCAP_ERRBUF_SIZE];		
	int packetReadStatus;				
	struct pcap_pkthdr *packetHeader; 	
	const u_char *packetData;			
	int packetCount = 1;				
	int packetType;

	if (argc != 2) {						
		printf("Not enough arguments\n");	
		return -1;							
	}

	if ((pcapPtr = pcap_open_offline(argv[1], errbuf)) == NULL) {	
		printf("There was an error opening the file\n");			
		return -1;													
	}

	while((packetReadStatus = pcap_next_ex(pcapPtr, &packetHeader, &packetData)) == 1) {	
 		printf("\nPacket number: %d  Packet Len: %d\n\n", packetCount, packetHeader->len);  	

 		packetType = loadEthernetHeader(packetData);
        
        if (packetType == -1) {
        	printf("\nUnknown Packet Type\n");
        }
        if (packetType == 1) {
        	loadARPHeader(packetData);
        }
        if (packetType == 2) {
        	loadIPHeader(packetData);
        }
        packetCount++;
 	}

 	pcap_close(pcapPtr);
	return 0;
}