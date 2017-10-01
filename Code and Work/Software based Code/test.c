/****************************************************************************
 * * timestamp_packet.
 * * 
 * * This code is a quick and dirty test of getting the timestamp a packet was 
 * * recieved by the kernel using IOCTL SIOCGSTAMP
 * *
 * * This code based upon example code in the "Unix Network Sockets Programming"
 * * books.
 * *
 * ***************************************************************************/
#include <sys/types.h>
#include <sys/socket.h>
#include <asm/sockios.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#define MAX_PACKET_SIZE 2048
/****************************************************************************
 * * error
 * * Write an error message and aborts..
 * ***************************************************************************/
void error(char *msg)
{
    perror(msg);
    exit(0);
}
/****************************************************************************
 * * get_time
 * *
 * * Gets the time using both gettimeofday and IOCTL and prints delta.
 * *
 * * Returns:
 * *      1 on success.
 * ***************************************************************************/
int get_time (int sock, char * packet, uint length) {
    //Places to store the timestamps.
    struct timeval tv_tod, tv_ioctl;
    int error;
    // Get the time packet was recieved with usec resolution
    gettimeofday (&tv_tod, 0);
    // and now use the IOCTL to get the time the packet was stamped by the kernel.
    error = ioctl(sock, SIOCGSTAMP, &tv_ioctl);
    printf ("gettimeofday: %d.%d\t ioctl: %d.%d\t delta: %d.%d\n", 
        tv_tod.tv_sec, tv_tod.tv_usec,
        tv_ioctl.tv_sec, tv_ioctl.tv_usec,
        tv_tod.tv_sec - tv_ioctl.tv_sec, 
        tv_tod.tv_usec - tv_ioctl.tv_usec
    );
    return 1;
}
/****************************************************************************
* main
* Parses arguments, opens port.
* Takes: 
*      A port number to listen on.
* Returns:
*      1 on success.
***************************************************************************/
int main(int argc, char *argv[])
{
    int sock, length, fromlen, n;
    struct sockaddr_in server;
    struct sockaddr_in from;
    char buf[MAX_PACKET_SIZE];
    unsigned char pending_packets=0;
    // Parse out the prot number. Abort if no port given.
    if (argc < 2) {
        fprintf(stderr, "Useage:\n %s port\n", argv[0]);
        exit(0);
    }
    // Standard create and listen on a UDP port snippet.
    sock=socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) error("Opening socket");
    length = sizeof(server);
    bzero(&server,length);
    server.sin_family=AF_INET;
    server.sin_addr.s_addr=INADDR_ANY;
    server.sin_port=htons(atoi(argv[1]));
    if (bind(sock,(struct sockaddr *)&server,length)<0) 
        error("binding");
    fromlen = sizeof(struct sockaddr_in);
    // Loop, getting packets, parsing the header and doing stuff...
    while (1) {
        n = recvfrom(sock,buf,MAX_PACKET_SIZE,0,(struct sockaddr *)&from,(socklen_t *)&fromlen);
        if (n < 0) error("recvfrom");
        get_time (sock, buf, n);
    }
}
