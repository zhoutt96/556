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
#include "utils.h"
#include "checksum.h"
#include "queue.h"

#define BUFFERSIZE 1000000
#define TIMEOUT 1000

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

typedef struct IString{
    char **str;     //the PChar of string array
    size_t num;     //the number of string
}IString;

typedef struct ACKCount{
    int count;
    int ackNum;
}ACKCount;

void MoveForward(struct window* send_window, Queue* queue){
    send_window->usable -= 1;
    send_window->to_be_send += 1;
    Enqueue(queue);
}

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

void fillPacket(packet* sendbuffer,struct window* send_window, __uint16_t left_filesize){
    __uint16_t new_checksum;

    if (left_filesize <= DATASIZE){
        sendbuffer->payload_size = left_filesize;
        sendbuffer->isEnd = 1;
    }else{
        sendbuffer->payload_size = DATASIZE;
        sendbuffer->isEnd = 0;
    }

    sendbuffer->checksum = 0;
    sendbuffer->seq_num = send_window->to_be_send;
    new_checksum = cksum((u_short*) sendbuffer, (DATASIZE+10)/2);
    sendbuffer->checksum = new_checksum;
}

void fillFilePacket(packet* sendbuffer,struct window* send_window, __uint16_t filename_path){
    __uint16_t new_checksum;

    sendbuffer->seq_num = 0;
    sendbuffer->isEnd = 0;
    sendbuffer->payload_size = filename_path;
    sendbuffer->checksum = 0;
    new_checksum = cksum((u_short*) sendbuffer, (DATASIZE+10)/2);
    sendbuffer->checksum = new_checksum;
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
    __uint16_t fileLength = strlen(filePath);
    checkFilePath(filePath);

    Queue *queue = (Queue*) malloc(sizeof(Queue));

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

    struct window send_window;
    send_window.usable = DEFAULTMAXWINDOWSIZE;
    send_window.to_be_send = 0;
    send_window.to_be_ack = 0;
    socklen_t addrlen = sizeof(addr);

    int send_num;
    int recv_num;
    // get the data size first
    double total_file_length = getFileLength(filePath);
    // calculate the total number of packets
    int total_count = (int)total_file_length/DATASIZE+1;
//    int total_count =(int) total_count_double + 1;

    struct ACKCount initAck;
    initAck.count = 0;
    initAck.ackNum = 0;
    FILE* fp = openFile(filePath);

    char* buffer = (unsigned char*) malloc(DATASIZE);
    /*
     * send the packet which contains the filename and directory name
     */

    packet* send_packet;
    ackpacket *recv_packet = (ackpacket*) malloc(sizeof(ackpacket));
    InitQueue(queue); // init the queue, and allocate memory to the data

    send_packet = Rear(queue);
    memcpy(send_packet->data, filePath, DATASIZE);
    fillFilePacket(send_packet, &send_window, fileLength);
    send_num = sendto(sock, send_packet, sizeof(*send_packet), 0, (const struct sockaddr *) &sin, sizeof(sin));
    MoveForward(&send_window, queue);
    printf("the size of the file path is %hu \n", fileLength);
    printf("the filepath checksum is %hu \n", send_packet->checksum);
    printf(" ----------------------------------------- ");

    while (send_window.to_be_send>0 || send_window.to_be_ack>0){
        while(send_window.usable > 0){
            /*
             * Keep sending data, until the usable windows is equal to zero
             */

            __uint16_t left_filesize = total_file_length-send_window.to_be_send;
            send_packet = Rear(queue);
            readFile(fp, send_packet->data, DATASIZE);
            fillPacket(send_packet, &send_window, left_filesize);
            send_num = sendto(sock,send_packet,sizeof(*send_packet), 0, (const struct sockaddr *) &sin, sizeof(sin));
            MoveForward(&send_window, queue);

            printf("the data size is %hu \n", send_packet->payload_size);
            printf("the checksum is %hu \n", send_packet->checksum);
            printf("current to_be_ack is %d\n", send_window.to_be_ack);
            printf("current to_be_send is %d \n", send_window.to_be_send);
            printf(" ----------------------------------------- ");
        }

        if ( send_window.usable == 0){
            /*
             * if there is no available data, then begin to wait for the ack and free the data
             */
            recvfrom(sock, recv_packet, sizeof(packet), 0, (struct sockaddr *)&addr, &addrlen);
            __uint16_t ack_checksum = recv_packet->ack_checksum;
            recv_packet->ack_checksum = 0;
            __uint16_t cur_checksum = cksum((u_short*) recv_packet, sizeof(*recv_packet)/2);

            if (cur_checksum == ack_checksum){
                /*
                 * store the ack number, and take a look
                 */
                if (ack_checksum == initAck.ackNum){
                    initAck.count += 1;
                    if (initAck.count == RESENDLIMIT){
                        /*
                         * if we receive the same ack for three times, then we need to resend this packet
                         */
                        send_packet = Front(queue);
                        send_num = sendto(sock, send_packet, sizeof(*send_packet), 0, (const struct sockaddr *) &sin, sizeof(sin));
                        initAck.count = 0;
                    }
                }else{
                    initAck.ackNum = ack_checksum;
                    initAck.count = 1;
                    if (send_window.to_be_ack == ack_checksum){
                        send_window.to_be_ack ++;
                        send_window.usable ++;
                        Dequeue(queue);
                    }
                }
            }
        }
    }
}

