CC = gcc
CFLAGS = -g -Wall 
#CFLAGS = -g

all:  	trace

trace: trace.c
	clear
	$(CC) $(CFLAGS) -o trace trace.c checksum.c  -lpcap 
	
clean:
	rm -f trace

run:
	./trace ArpTest.pcap >myArp.out
	./trace Http.pcap >myHttp.out
	./trace IP_bad_checksum.pcap >myIP_bad.out
	./trace PingTest.pcap >myPingTest.out
	./trace smallTCP.pcap >mysmallTCP.out
	./trace UDPfile.pcap >myUDP.out
	./trace largeMix.pcap >mylarge.out
	./trace largeMix2.pcap >mylarge2.out
	./trace Capture1.pcap >myCapture1.out
	./trace Capture2.pcap >myCapture2.out

diff:
	diff ArpTest.out myArp.out
	diff Http.out myHttp.out
	diff IP_bad_checksum.out myIP_bad.out
	diff PingTest.out myPingTest.out
	diff smallTCP.out mysmallTCP.out
	diff UDPfile.out myUDP.out
	diff largeMix.out mylarge.out
	diff largeMix2.out mylarge2.out
