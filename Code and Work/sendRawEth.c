/*
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 */

#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/ether.h>

#define SOCK_PATH "/home/mayur/echo_socket"

#define MY_DEST_MAC0	0x00
#define MY_DEST_MAC1	0x21
#define MY_DEST_MAC2	0x5C
#define MY_DEST_MAC3	0xBA
#define MY_DEST_MAC4	0xD0
#define MY_DEST_MAC5	0x4C

#define DEFAULT_SENDIF	"eth0"
#define DEFAULT_RECVIF	"wlan0"
#define BUF_SIZ		1024
#define MAX_PACKET_SIZE 2048

int sendSocketFd, recvSocketFd, fromlen, n;
struct ifreq if_idx;
struct ifreq sendIfMac;
struct ifreq recvIfMac;
struct sockaddr_in server;
struct sockaddr_in from;
int tx_len = 0;
char sendbuf[BUF_SIZ], recvBuf[BUF_SIZ];
struct ether_header *eh = (struct ether_header *) sendbuf;
struct iphdr *iph = (struct iphdr *) (sendbuf + sizeof(struct ether_header));
struct sockaddr_ll socket_address;
char sendIfName[IFNAMSIZ], recvIfName[IFNAMSIZ];
uint64_t startTime, endTime;

void error(char *msg)
{
    perror(msg);
    exit(0);
}

uint64_t getTimeStamp() {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec*(uint64_t)1000000+tv.tv_usec;
}

uint64_t getSocketTimeStamp(int sock, char * packet, uint length) {
    //Places to store the timestamps.
    struct timeval tv_ioctl;
    int error;
    // Use the IOCTL to get the time the packet was stamped by the kernel.
    error = ioctl(sock, SIOCGSTAMP, &tv_ioctl);
    return tv_ioctl.tv_sec*(uint64_t)1000000+tv_ioctl.tv_usec;
}

void *sendThread(void *arg){
    while(1){
        if (sendto(sendSocketFd, sendbuf, tx_len, 0, (struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll)) < 0)
            printf("Send failed\n");
        startTime = getTimeStamp();
        printf("Send Success\n");
        usleep(1000000);
    }
}

void *recieveThread(void *arg){
    printf("Waiting on Recieving Packet\n");
    while(1){
        n = recvfrom(recvSocketFd, recvBuf, tx_len, 0, (struct sockaddr *)&from, (socklen_t *)&fromlen);
        if (n < 0) error("recvfrom");
        printf("Recieved Packet\n");
        getSocketTimeStamp(sendSocketFd, recvBuf, n);
    }
}

int main(int argc, char *argv[])
{
    pthread_t senderPThread, recievePThread;
	
	/* Get sender interface name */
	if (argc > 1)
		strcpy(sendIfName, argv[1]);
	else
		strcpy(sendIfName, DEFAULT_SENDIF);

    printf("Sender IF name:%s\n", sendIfName);

	/* Get sender interface name */
	if (argc > 1)
		strcpy(recvIfName, argv[2]);
	else
		strcpy(recvIfName, DEFAULT_RECVIF);

    printf("Reciever IF name:%s\n", recvIfName);

	/* Open RAW socket to send on */
	if ((sendSocketFd = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW)) == -1) {
	    perror("send socket");
	}
    printf("Sender Socket Opened\n");

	/* Open RAW socket to recv on */
	if ((recvSocketFd = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW)) == -1) {
	    perror("recv socket");
	}
    printf("Reciever Socket Opened\n");

	/* Get the index of the interface to send on */
	memset(&if_idx, 0, sizeof(struct ifreq));
	strncpy(if_idx.ifr_name, sendIfName, IFNAMSIZ-1);
	if (ioctl(sendSocketFd, SIOCGIFINDEX, &if_idx) < 0)
	    perror("SIOCGIFINDEX");

	/* Get the MAC address of the interface to send on */
	memset(&sendIfMac, 0, sizeof(struct ifreq));
	strncpy(sendIfMac.ifr_name, sendIfName, IFNAMSIZ-1);
	if (ioctl(sendSocketFd, SIOCGIFHWADDR, &sendIfMac) < 0)
	    perror("SIOCGIFHWADDR");

	/* Get the MAC address of the interface to recv on */
	memset(&recvIfMac, 0, sizeof(struct ifreq));
	strncpy(recvIfMac.ifr_name, recvIfName, IFNAMSIZ-1);
	if (ioctl(recvSocketFd, SIOCGIFHWADDR, &recvIfMac) < 0)
	    perror("SIOCGIFHWADDR");

    printf("Successfully received Sender MAC Address : %02x:%02x:%02x:%02x:%02x:%02x\n",
        (unsigned char) sendIfMac.ifr_hwaddr.sa_data[0],
        (unsigned char) sendIfMac.ifr_hwaddr.sa_data[1],
        (unsigned char) sendIfMac.ifr_hwaddr.sa_data[2],
        (unsigned char) sendIfMac.ifr_hwaddr.sa_data[3],
        (unsigned char) sendIfMac.ifr_hwaddr.sa_data[4],
        (unsigned char) sendIfMac.ifr_hwaddr.sa_data[5]);

    printf("Successfully received Reciever MAC Address : %02x:%02x:%02x:%02x:%02x:%02x\n",
        (unsigned char) recvIfMac.ifr_hwaddr.sa_data[0],
        (unsigned char) recvIfMac.ifr_hwaddr.sa_data[1],
        (unsigned char) recvIfMac.ifr_hwaddr.sa_data[2],
        (unsigned char) recvIfMac.ifr_hwaddr.sa_data[3],
        (unsigned char) recvIfMac.ifr_hwaddr.sa_data[4],
        (unsigned char) recvIfMac.ifr_hwaddr.sa_data[5]);

	/* Construct the Ethernet header */
	memset(sendbuf, 0, BUF_SIZ);
	/* Ethernet header */
	eh->ether_shost[0] = ((uint8_t *)&sendIfMac.ifr_hwaddr.sa_data)[0];
	eh->ether_shost[1] = ((uint8_t *)&sendIfMac.ifr_hwaddr.sa_data)[1];
	eh->ether_shost[2] = ((uint8_t *)&sendIfMac.ifr_hwaddr.sa_data)[2];
	eh->ether_shost[3] = ((uint8_t *)&sendIfMac.ifr_hwaddr.sa_data)[3];
	eh->ether_shost[4] = ((uint8_t *)&sendIfMac.ifr_hwaddr.sa_data)[4];
	eh->ether_shost[5] = ((uint8_t *)&sendIfMac.ifr_hwaddr.sa_data)[5];
	eh->ether_dhost[0] = ((uint8_t *)&recvIfMac.ifr_hwaddr.sa_data)[0];
	eh->ether_dhost[1] = ((uint8_t *)&recvIfMac.ifr_hwaddr.sa_data)[1];
	eh->ether_dhost[2] = ((uint8_t *)&recvIfMac.ifr_hwaddr.sa_data)[2];
	eh->ether_dhost[3] = ((uint8_t *)&recvIfMac.ifr_hwaddr.sa_data)[3];
	eh->ether_dhost[4] = ((uint8_t *)&recvIfMac.ifr_hwaddr.sa_data)[4];
	eh->ether_dhost[5] = ((uint8_t *)&recvIfMac.ifr_hwaddr.sa_data)[5];

	/* Ethertype field */
	eh->ether_type = htons(ETH_P_IP);
	tx_len += sizeof(struct ether_header);

    /* IP header */
	sendbuf[tx_len++] = 0x45;//IPv4, header length of 5
	sendbuf[tx_len++] = 0x00;//DSCP and ECN
	sendbuf[tx_len++] = 0x00;//
	sendbuf[tx_len++] = 0x14;// 20 bytes for size
    sendbuf[tx_len++] = 0x00;// ID
    sendbuf[tx_len++] = 0x00;// Flag and Fragment Offset
    sendbuf[tx_len++] = 0xF0;// TTL and Protocol
    sendbuf[tx_len++] = 0x00;// Header CHecksum
    sendbuf[tx_len++] = 0x00;// Src IP
    sendbuf[tx_len++] = 0x00;// Src IP
    sendbuf[tx_len++] = 0x00;// Dst IP
    sendbuf[tx_len++] = 0x00;// Dst IP

	/* Packet data */
    sendbuf[tx_len++] = 0xde;
    sendbuf[tx_len++] = 0xad;
    sendbuf[tx_len++] = 0xbe;
    sendbuf[tx_len++] = 0xef;
    sendbuf[tx_len++] = 0xde;
    sendbuf[tx_len++] = 0xad;
    sendbuf[tx_len++] = 0xbe;
    sendbuf[tx_len++] = 0xef;

	/* Index of the network device */
	socket_address.sll_ifindex = if_idx.ifr_ifindex;
	/* Address length*/
	socket_address.sll_halen = ETH_ALEN;
	/* Destination MAC */
	socket_address.sll_addr[0] = MY_DEST_MAC0;
	socket_address.sll_addr[1] = MY_DEST_MAC1;
	socket_address.sll_addr[2] = MY_DEST_MAC2;
	socket_address.sll_addr[3] = MY_DEST_MAC3;
	socket_address.sll_addr[4] = MY_DEST_MAC4;
	socket_address.sll_addr[5] = MY_DEST_MAC5;

	/* Send packet */
    pthread_create(&senderPThread,NULL,sendThread,NULL);
    pthread_create(&recievePThread,NULL,recieveThread,NULL);
    pthread_join(senderPThread, NULL);
    pthread_join(recievePThread, NULL);
	return 0;
}
