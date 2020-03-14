#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
//#include <ctime>
#include <sys/time.h>
#include <cmath>
#include "utils.h"
#include "queue.h"

#define BUFFERSIZE 1000000
#define TIMEEXCEEDLIMIT 2000
#define RECEIVED 1
#define NOTRECEIVED 0

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

long getLatency(struct timeval* start, struct timeval* end){
    return (end->tv_sec - start->tv_sec) * 1000000 + end->tv_usec - start->tv_usec;
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




void fillPacket(FILE *fp, packet* sendbuffer, struct window* send_window, __uint32_t left_filesize){
    __uint16_t new_checksum;

    if (left_filesize <= DATASIZE){
        memset(sendbuffer->data, 0, DATASIZE);
        sendbuffer->payload_size = left_filesize;
        sendbuffer->isEnd = 1;
        readFile(fp, sendbuffer->data, left_filesize);
    }else{
        sendbuffer->payload_size = DATASIZE;
        sendbuffer->isEnd = 0;
        readFile(fp, sendbuffer->data, DATASIZE);
    }

    sendbuffer->checksum = 0;
    sendbuffer->seq_num = send_window->to_be_send;
    new_checksum = cksum((u_short*) sendbuffer, sizeof(packet)/2);
    sendbuffer->checksum = new_checksum;
}

void fillFilePacket(packet* sendbuffer, __uint16_t filename_path_len){
    __uint16_t new_checksum;
    sendbuffer->seq_num = 0;
    sendbuffer->isEnd = 2;
    sendbuffer->payload_size = filename_path_len;
    sendbuffer->checksum = 0;
    new_checksum = cksum((u_short*) sendbuffer, sizeof(packet)/2);
    sendbuffer->checksum = new_checksum;
}

int main(int argc, char** argv) {
    struct timeval send_start_time, send_end_time;
    gettimeofday(&send_start_time, NULL);


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
    struct sockaddr_in sin;
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

    struct timeval timeout;
    timeout.tv_sec = 1;//sencond
    timeout.tv_usec = 0;//m_second
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1) {
        perror("setsockopt failed:");
    }

    /* fill in the server's address */
    memset (&sin, 0, sizeof (sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = server_addr;
    sin.sin_port = htons(server_port);

    socklen_t sinlen = sizeof(sin);

    struct window send_window;
    send_window.usable = DEFAULTMAXWINDOWSIZE;
    send_window.to_be_send = 1;
    send_window.invalid_count = 0;

    int send_num;
    int recv_num;
    // get the data size first
    __uint32_t total_file_length = getFileLength(filePath);
    // calculate the total number of packets
    printf("the total_file_length is %u \n", total_file_length);
    __uint32_t total_count = (int)total_file_length/DATASIZE+1;
    printf("the datasize is %d \n", DATASIZE);
    printf("the total count is %u \n", total_count);

    struct ACKCount initAck;
    initAck.count = -1;
    initAck.ackNum = -1;
    FILE* fp = openFile(filePath);
    struct timeval last_send_tstamp, cur_timestamp;
    long c = 0;
    __uint32_t offset = 0;
    long latency;

    /*
     * send the packet which contains the filename and directory name, in the format of filepath/filename
     */

    packet* send_packet;
    ackpacket *recv_packet = (ackpacket*) malloc(sizeof(ackpacket));
    InitQueue(queue); // init the queue, and allocate memory to the data

    /*
     * Send the filename and path first, the sender will not transfer data until receiving the ack from the receiver
     */
    send_packet = (packet*) malloc(sizeof(packet));
    int recv_filename_ack=NOTRECEIVED;

    memset(send_packet->data,0,DATASIZE);
    memcpy(send_packet->data, filePath, fileLength);
    fillFilePacket(send_packet,fileLength);
    send_num = sendto(sock, send_packet, sizeof(*send_packet), 0, (const struct sockaddr *) &sin, sizeof(sin));

    while(send_num<0){
        printf("[SEND FAIL] Resend the filepath and filename \n");
        send_num = sendto(sock, send_packet, sizeof(*send_packet), 0, (const struct sockaddr *) &sin, sizeof(sin));
    }

    printf("[SEND SUCCESS] Send the filepathf/filename successfully\n");

    gettimeofday(&last_send_tstamp, NULL);

    while (recv_filename_ack == NOTRECEIVED){
        recv_num = recvfrom(sock, recv_packet, sizeof(ackpacket), 0, (struct sockaddr *)&sin, &sinlen);
//        printf("recv_num is %d \n", recv_num);
        if (recv_num>0 && recv_packet->ack_checksum == recv_packet->ack_num){
            /*Receive the ack package from receiver succesfully,
             * begin to send the content */
            recv_filename_ack = RECEIVED;
            printf("[RECEVE ACK SUCCESS] Receive the ack number of filepath/filename successfully \n");
        }else{
            /*
             * if time out, resend the packet
             */
            latency = getLatency(&last_send_tstamp, &cur_timestamp);
            gettimeofday(&cur_timestamp, NULL);
//            printf("The latency is %ld \n", latency);
            if(latency >= TIMEEXCEEDLIMIT){
                send_num = sendto(sock, send_packet, sizeof(*send_packet), 0, (const struct sockaddr *) &sin, sizeof(sin));
                gettimeofday(&last_send_tstamp, NULL);
                printf("[TIME EXCEED LIMIT] Resend the filepath and filename \n");
            }
        }
    }

    printf("-------- [BEGIN TRANSFERRING] -------- \n");
    __uint32_t ack_checksum;
    while (send_window.to_be_send <= total_count || queue->size>0){
        while(send_window.usable > 0 && send_window.to_be_send<=total_count){
            /*
             * Keep sending data, until the usable windows is equal to zero
             */

            offset = (send_window.to_be_send - 1)*DATASIZE;
            __uint32_t left_filesize = total_file_length-(send_window.to_be_send-1)*DATASIZE;
            send_packet = Rear(queue);
            fillPacket(fp, send_packet, &send_window, left_filesize);
            send_num = sendto(sock,send_packet,sizeof(*send_packet), 0, (const struct sockaddr *) &sin, sizeof(sin));
            printf("[SEND DATA] %u (%u) Seq(%u)\n", offset, send_packet->payload_size, send_packet->seq_num);
            while(send_num<=0){
                send_num = sendto(sock,send_packet,sizeof(*send_packet), 0, (const struct sockaddr *) &sin, sizeof(sin));
                printf("[SEND DATA] %u (%u)\n", offset, send_packet->payload_size);
            }

            MoveForward(&send_window, queue);
            gettimeofday(&last_send_tstamp, NULL);
        }


        /*
         * if there is no available data, then begin to wait for the ack and free the data
         */
        recv_num = recvfrom(sock, recv_packet, sizeof(ackpacket), 0, (struct sockaddr *)&sin, &sinlen);
        ack_checksum = recv_packet->ack_checksum;

        /*
         * if the ack is correct, and it is equal to the smallest ack in queue, accept it, otherwise, ignore it
         */

        if (recv_num > 0){
            if (recv_packet->ack_checksum == recv_packet->ack_num ){

                /*
                 * store the ack number, and take a look
                 */

                if (ack_checksum >= Front(queue)->seq_num){
                    printf("[Recv ACK] %u \n", ack_checksum);
                    initAck.ackNum = ack_checksum;
                    initAck.count = 1;
                    while (queue->size>0 && ack_checksum >= Front(queue)->seq_num){
                        Dequeue(queue);
                        send_window.usable ++;
                    }
                }else{
                    printf("[Ignore ACK] %u \n", ack_checksum);
                    if (ack_checksum == initAck.ackNum){
                        initAck.count += 1;
                        if (initAck.count == RESENDLIMIT){
                            /*
                             * if we receive the same ack for three times, then we need to resend this packet
                             */
                            send_packet = Front(queue);
                            while(send_num<=0){
                                send_num = sendto(sock, send_packet, sizeof(*send_packet), 0, (const struct sockaddr *) &sin, sizeof(sin));
                            }
                            offset = (Front(queue)->seq_num-1)*DATASIZE;

                            printf("[MORE THAN 3 ACK RESEND] %u (%u)\n", offset, send_packet->seq_num);
                            initAck.count = 0;
                            gettimeofday(&last_send_tstamp, NULL);
                        }
                    }else{
                        initAck.ackNum = ack_checksum;
                        initAck.count = 1;
                    }
                }
            }else{
                printf("[receive corrupt ack] %u \n ", recv_packet->ack_num);
            }
        }

        gettimeofday(&cur_timestamp, NULL);
        latency = getLatency(&last_send_tstamp, &cur_timestamp);
        if (latency >= TIMEEXCEEDLIMIT){
            /*
             * time exceed, retransmit the packet
             */
            send_packet = Front(queue);
            send_num = sendto(sock,send_packet,sizeof(*send_packet), 0, (const struct sockaddr *) &sin, sizeof(sin));
            gettimeofday(&last_send_tstamp, NULL);
        }

        printf("cur queue front seq is %u \n", Front(queue)->seq_num);
        printf("the usable is %d \n", send_window.usable);
        printf(" ------------------------------------------ \n");
    }

    gettimeofday(&send_end_time, NULL);
    double total_travel_time = getLatency(&send_start_time, &send_end_time)/10000000;
    printf("Sending file successfully, using %f", total_travel_time);
}
