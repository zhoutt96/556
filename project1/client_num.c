#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>

/* simple client, takes two parameters, the server domain name,
   and the server port number */

long double getLatency(char* buffer, struct timeval* end){
    long returnSecond = (long) ntohl(*(long *)(buffer+2));
    long returnUSecond = (long) ntohl(*(long *)(buffer+6));
    return (end->tv_sec - returnSecond) * 1000000 + end->tv_usec - returnUSecond;
}

int main(int argc, char** argv) {

    /* our client socket */
    int sock;
    long double totalLatency = 0;
    long double averageLatency;
    /* variables for identifying the server */
    unsigned int server_addr;
    struct sockaddr_in sin;
    struct addrinfo *getaddrinfo_result, hints;

    /* convert server domain name to IP address */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; /* indicates we want IPv4 */

    if (getaddrinfo(argv[1], NULL, &hints, &getaddrinfo_result) == 0) {
        server_addr = (unsigned int) ((struct sockaddr_in *) (getaddrinfo_result->ai_addr))->sin_addr.s_addr;
        freeaddrinfo(getaddrinfo_result);
    }

    /* server port number */
    unsigned short server_port = atoi (argv[2]);

    char *buffer, *sendbuffer;
    int count = atoi(argv[4]);
    int dataSize = atoi(argv[3]);
    int size = dataSize+10;
    long returnTimeStamp;

    buffer = (char *) malloc(size);
    if (!buffer)
    {
        perror("failed to allocated buffer");
        abort();
    }

    sendbuffer = (char *) malloc(size);
    if (!sendbuffer)
    {
        perror("failed to allocated sendbuffer");
        abort();
    }


    /* create a socket */
    if ((sock = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        perror ("opening TCP socket");
        abort ();
    }

    /* fill in the server's address */
    memset (&sin, 0, sizeof (sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = server_addr;
    sin.sin_port = htons(server_port);

    /* connect to the server */
    if (connect(sock, (struct sockaddr *) &sin, sizeof (sin)) < 0)
    {
        perror("connect to server failed");
        abort();
    }

//    returnTimeStamp = (char *) malloc(4);

    *(short *) (sendbuffer) = (short) htons(dataSize);

    struct timeval start, end;

    int newCount = 0;
    while (newCount < count)
    {
        gettimeofday(&start, NULL);

        *(long *) (sendbuffer + 2) = (long) htonl(start.tv_sec);
        *(long *) (sendbuffer + 6) = (long) htonl(start.tv_usec);

        send(sock, sendbuffer, size, 0);

        recv(sock, buffer, size, 0);

        gettimeofday(&end, NULL);
        totalLatency += getLatency(buffer, &end);
        newCount ++;
    }
    
    averageLatency = totalLatency/(count*1000);
    printf("The average latency is: %.3Lf millisecond \n", averageLatency);
    return 0;
}

