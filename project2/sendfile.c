#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <math.h>
#include <jmorecfg.h>
#include "utils.h"
#include "checksum.h"

#define BUFFERSIZE 1000000



typedef struct {
    char **str;     //the PChar of string array
    size_t num;     //the number of string
}IString;


typedef struct window{
    int windowsize;  // total window size of sliding window
    int usable; // total availwindow
    int unack;
    int nextwindow;
} sendwindow;

int Split(char *src, char *delim, IString* istr)//split buf
{
    int i;
    char *str = NULL, *p = NULL;

    (*istr).num = 1;
    str = (char*)calloc(strlen(src)+1,sizeof(char));
    if (str == NULL) return 0;
    (*istr).str = (char**)calloc(1,sizeof(char *));
    if ((*istr).str == NULL) return 0;
    strcpy(str,src);

    p = strtok(str, delim);
    (*istr).str[0] = (char*)calloc(strlen(p)+1,sizeof(char));
    if ((*istr).str[0] == NULL) return 0;
    strcpy((*istr).str[0],p);
    for(i=1; p = strtok(NULL, delim); i++)
    {
        (*istr).num++;
        (*istr).str = (char**)realloc((*istr).str,(i+1)*sizeof(char *));
        if ((*istr).str == NULL) return 0;
        (*istr).str[i] = (char*)calloc(strlen(p)+1,sizeof(char));
        if ((*istr).str[0] == NULL) return 0;
        strcpy((*istr).str[i],p);
    }
    free(str);
    str = p = NULL;

    return 1;
}


void helpMenu()
{
    printf("------------------------            Menu for COMP 556 Project 2         ------------------------|\n");
    printf("|./sendfile -h:                                                               show help menu    |\n");
    printf("|./sendfile -r <recv host>:<recv port> -f <subdir>/<filename>                                   |\n");
    printf("|        <recv host> (Required):                                                                |\n"
           "|               The IP address of the remote host in a.b.c.d format.                            |\n" );
    printf("|        <recv port> (Required):                                                                |\n"
           "|               The UDP port of the remote host.                                                |\n");
    printf("|        <subdir> (Required):                                                                   |\n"
           "|               The local subdirectory where the file is located.                               |\n");
    printf("|        <filename>(Required):                                                                  |\n"
           "|               The name of the file to be sent.                                                |\n");
    printf("|------------------------                End of the Menu                ------------------------|\n");
}

void errorMenu()
{
    printf("Wrong Input! \n Enter ./sendfile -h to see usage \n");
    abort();
}

long double getLatency(char* buffer, struct timeval* end){
    long returnSecond = (long) ntohl(*(long *)(buffer+2));
    long returnUSecond = (long) ntohl(*(long *)(buffer+6));
    return (end->tv_sec - returnSecond) * 1000000 + end->tv_usec - returnUSecond;
}

void checkFilePath(char* request_dir){
    if(access(request_dir,F_OK)!= 0){
        printf("File Can not be found! \n");
        abort();
    }

    if(access(request_dir,R_OK)!=0){
        printf("Errors when trying to open the file \n");
        abort();
    }
}

void checkPort(int port)
{
    if (port < 18000 || port > 18200)
    {
        perror("Port number should between 18000 and 18200 \n Enter ./sendfile -h to see usage \n");
        abort();
    }
}

int main(int argc, char** argv) {

    if (strcmp(argv[1], "-h") == 0)
    {
        helpMenu();
        abort();
    }

    if (argc != 5 || strcmp(argv[1], "-r") !=0 || strcmp(argv[3], "-f") !=0 ){
        errorMenu();
        abort();
    }

//    char* hostAndPort = (char*) malloc(BUFSIZ);
    int sock;
    unsigned int server_addr;
    struct sockaddr_in sin, addr;
    struct addrinfo *getaddrinfo_result, hints;
    unsigned short server_port;  /* server port number */
    char* filePath = argv[4];
    checkFilePath(filePath);

    /* convert server domain name to IP address */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; /* indicates we want IPv4 */

    if (getaddrinfo(argv[1], NULL, &hints, &getaddrinfo_result) == 0) {
        server_addr = (unsigned int) ((struct sockaddr_in *) (getaddrinfo_result->ai_addr))->sin_addr.s_addr;
        freeaddrinfo(getaddrinfo_result);
    }

    IString istr;
    /* Split the hostname:port, and then interpret the hostname and port number */
    if ( Split(argv[2], ":", &istr)){
        if (getaddrinfo(istr.str[0], NULL, &hints, &getaddrinfo_result) == 0) {
            server_addr = (unsigned int) ((struct sockaddr_in *) (getaddrinfo_result->ai_addr))->sin_addr.s_addr;
            freeaddrinfo(getaddrinfo_result);
        }
        server_port = atoi(istr.str[1]); // this is the port number
        checkPort(server_port);
        // free the struct
    }else{
        errorMenu();
        abort();
    }

    if ((sock = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror ("UDP socket creation failure");
        abort ();
    }

    /* fill in the server's address */
    memset (&sin, 0, sizeof (sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = server_addr;
    sin.sin_port = htons(server_port);

//    char* buffer = (char*) malloc(BUFSIZ);
//    readFile(filePath, buffer);

    struct window send_window;
    send_window.windowsize = DEFAULTMAXWINDOWSIZE;
    send_window.usable = DEFAULTMAXWINDOWSIZE;
    send_window.unack = 1;
    send_window.nextwindow = 1;

    int send_num;
    int recv_num;
    // get the data size first
    double total_file_length = getFileLength(filePath);
    // calculate the total number of packets
    double total_count_double = total_file_length/DATASIZE;
    int total_count = ceil(total_count_double);

    // read the data of length BUFFERSIZE each time from the file, allocate memory for msg

    char* buffer = (char*) malloc(BUFFERSIZE);

    packet send_packet = {1,0,0,0,0};
    ackpacket ack_packet;
    memcpy(send_packet.data, filePath, DATASIZE);
    send_packet.payload_checksum = crc_16((unsigned char*)send_packet.data, sizeof(send_packet.data));
    boolean ack=FALSE;
    while (ack == FALSE) {
        send_num = sendto(sock, &send_packet, sizeof(send_packet), 0, (const struct sockaddr *) &sin, sizeof(sin));
        recv_num = recvfrom(sock, buffer, sizeof(packet), 0, (const struct sockaddr *) &sin, sizeof(sin));
    }

    /* when file is less than 1 MB*/
    // send the packet which contains the filename and directory name
    sendto(sock, &send_packet, sizeof(send_packet), 0, (const struct sockaddr *) &sin, sizeof(sin));
    printf("send the data successfully");

    while (send_window.usable > 0 && send_window.nextwindow < total_file_length){
        
    }
}

