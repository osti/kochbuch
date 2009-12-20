/* http://gnosis.cx/publish/programming/sockets2.html */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <time.h>

#define BUFFSIZE 255

int main(int argc, const char *argv[])
{
    int sock;
    struct sockaddr_in waffelserver;
    struct sockaddr_in waffelclient;
    char buffer[BUFFSIZE];
    unsigned int querylen, clientlen;
    int received = 0;
    struct timeval to;

    char* QUERY;
    char* pQuery[] = { "qWaffelCount", "qSpendenSumme", "qLastWaffel", "qSong" };

    if (argc != 2)
    {
	fprintf(stderr, "usage: %s <server_ip>\n", argv[0]);
	exit(-1);
    }
    srand48(time(0));

    while (1)
    {
	int ASK, WAIT;

	ASK = drand48() * 4;
	QUERY = pQuery[ASK];
	WAIT = 30 + drand48() * 30;
	WAIT = 1 + drand48() * 2;

	/* create the UDP socket */
	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
	{
	    fprintf(stderr, "Failed to create socket\n");
	    exit(-2);
	}

	/* set timeout to 1 s */
	to.tv_sec  = 1;
	to.tv_usec = 0;
	setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &to, sizeof(to));
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof(to));

	/* construct the server sockaddr_in structure */
	memset(&waffelserver, 0, sizeof(waffelserver));    /* Clear struct */
	waffelserver.sin_family = AF_INET;                 /* Internet/IP */
	waffelserver.sin_addr.s_addr = inet_addr(argv[1]); /* IP address */
	waffelserver.sin_port = htons(1042);               /* server port */

	/* query the server */
	querylen = strlen(QUERY);
	if (sendto(sock, QUERY, querylen, 0, (struct sockaddr *) &waffelserver, sizeof(waffelserver)) != querylen)
	    exit(-3);

	/* Receive the word back from the server */
	clientlen = sizeof(waffelclient);
	received = recvfrom(sock, buffer, BUFFSIZE, 0, (struct sockaddr*) &waffelclient, &clientlen);

	/* Check that client and server are using same socket */
	if (waffelserver.sin_addr.s_addr != waffelclient.sin_addr.s_addr)
	{
	    printf("...\n");
	    exit(-4);
	}

	buffer[received] = '\0';        /* Assure null terminated string */
	fprintf(stdout, buffer);
	fprintf(stdout, "\n");
	close(sock);

	exit(0);
	sleep(WAIT);
    }
    return 0;
}

