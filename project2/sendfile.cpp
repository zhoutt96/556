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


typedef struct HostAndPort{
    char *host;     //the PChar of string array
    unsigned int port;     //the number of string
}HostAndPort;


typedef struct ACKCount{
    int count;
    int ackNum;
}ACKCount;

void MoveForward(struct window* send_window, Queue* queue){
    send_window->usable -= 1;
    send_window->to_be_send += 1;
    Enqueue(queue);
}

int SplitPort(char *str, char *delim, HostAndPort *server){
//    printf("%s \n", str);
    char s[strlen(str)];
    for (int i=0; i< strlen(str); i++){
        s[i] = str[i];
    }
    int start = 0;
    char *p;
    p = strtok(s, delim);
    while(start<=1) {
        if (start == 0){
            server->host = p;
        }else{
            server->port = atoi(p);
//            memcpy(&server->port, &p, 5);
        }
        p = strtok(NULL, delim);
        start ++;
    }
    return 1;
}


void helpMenu()
{
    printf("------------------------            Menu for COMP 556 Project 2         ------------------------|\n");
    printf("|./sendfile -h:                                                               show help menu    |\n");
    printf("|./sendfile -r <recv_host>:<recv_port> -f <subdir>/<filename>                                   |\n");
    printf("|        <<recv_host> (Required):                                                                |\n"
           "|               The IP address of the remote host in a.b.c.d format.                            |\n" );
    printf("|        <recv_port> (Required):                                                                |\n"
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

//debug，判断窗口中的数据包是否收到了ACK
bool is_ack[QUEUE_SIZE];//debug
//debug，test RTT
struct timeval ack_time;
struct timeval send_time[QUEUE_SIZE];
struct timeval receive_time[QUEUE_SIZE];

//struct timeval send_time;
int main(int argc, char** argv) {

    struct timeval send_start_time, send_end_time;
    //debug
    bool is_first = false;
    long delay;
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

    HostAndPort server;

    /* Split the hostname:port, and then interpret the hostname and port number */
    if (SplitPort(argv[2], ":", &server)){
        if (getaddrinfo(server.host, NULL, &hints, &getaddrinfo_result) == 0) {
            server_addr = (unsigned int) ((struct sockaddr_in *) (getaddrinfo_result->ai_addr))->sin_addr.s_addr;
            freeaddrinfo(getaddrinfo_result);
        }
        server_port = server.port; // this is the port number
//        printf("%u \n", server_port);
        checkPort(server_port);
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
    timeout.tv_sec = 0;//second
    timeout.tv_usec = 1;//m_second
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
    __uint32_t total_count;
    if( total_file_length % DATASIZE ==0)
    {
        total_count = total_file_length/DATASIZE;
    } else {
        total_count = total_file_length / DATASIZE + 1;
    }
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

    packet *send_packet = (packet*) malloc(sizeof(packet));
    ackpacket *recv_packet = (ackpacket*) malloc(sizeof(ackpacket));
    ackpacket *send_fin_packet = (ackpacket*) malloc(sizeof(ackpacket));

    InitQueue(queue); // init the queue, and allocate memory to the data

    /*
     * Send the filename and path first, the sender will not transfer data until receiving the ack from the receiver
     */
//    send_packet = (packet*) malloc(sizeof(packet));
    int recv_filename_ack=NOTRECEIVED;

    memset(send_packet->data,0,DATASIZE);
    memcpy(send_packet->data, filePath, fileLength);
    fillFilePacket(send_packet,fileLength);
    send_num = sendto(sock, send_packet, sizeof(*send_packet), 0, (const struct sockaddr *) &sin, sizeof(sin));

    while(send_num<0){
//        printf("[SEND FAIL] Resend the filepath and filename \n");
        send_num = sendto(sock, send_packet, sizeof(*send_packet), 0, (const struct sockaddr *) &sin, sizeof(sin));
        printf("[send filepath/filename] \n");
    }

//    printf("[SEND SUCCESS] Send the path of file /filename successfully\n");

    gettimeofday(&last_send_tstamp, NULL);

    while (recv_filename_ack == NOTRECEIVED){
        recv_num = recvfrom(sock, recv_packet, sizeof(ackpacket), 0, (struct sockaddr *)&sin, &sinlen);
        if (recv_num>0 && recv_packet->last_inorder_ack == recv_packet->ack_num){
            /*Receive the ack package from receiver succesfully,
             * begin to send the content */
            recv_filename_ack = RECEIVED;
//            printf("[RECEIVE ACK SUCCESS] Receive the ack number of filepath/filename successfully \n");
        }else{
            /*
             * if time out, resend the packet
             */
            gettimeofday(&cur_timestamp, NULL);
            latency = getLatency(&last_send_tstamp, &cur_timestamp);
            if(latency >= TIMEEXCEEDLIMIT){
                send_num = sendto(sock, send_packet, sizeof(*send_packet), 0, (const struct sockaddr *) &sin, sizeof(sin));
                gettimeofday(&last_send_tstamp, NULL);
                printf("[TIME EXCEED LIMIT] Resend the filepath and filename \n");
            }
        }
    }

//    printf("---------------- [BEGIN SENDING DATA] ---------------- \n");
    __uint32_t last_inorder_ack;
    __uint16_t new_checksum;

    while (true){
        while (send_window.usable > 0 && send_window.to_be_send <= total_count) {
            /*
             * Keep sending data, until the usable windows is equal to zero
             */

            offset = (send_window.to_be_send - 1) * DATASIZE;
            __uint32_t left_filesize = total_file_length - (send_window.to_be_send - 1) * DATASIZE;
            send_packet = Rear(queue);
            //seq_num data is not ACK
            is_ack[queue->rear % QUEUE_SIZE] = false;
            fillPacket(fp, send_packet, &send_window, left_filesize);
            send_num = sendto(sock, send_packet, sizeof(*send_packet), 0, (const struct sockaddr *) &sin, sizeof(sin));
//            printf("[send data] %u (%u) Seq(%u)\n", offset, send_packet->payload_size, send_packet->seq_num);
            printf("[send data] %u (%u) \n", offset, send_packet->payload_size);
            while (send_num <= 0) {
                send_num = sendto(sock, send_packet, sizeof(*send_packet), 0, (const struct sockaddr *) &sin,
                                  sizeof(sin));
                printf("[send data] %u (%u)\n", offset, send_packet->payload_size);
//                printf("[SEND DATA] %u (%u)\n", offset, send_packet->payload_size);
            }
            gettimeofday(&last_send_tstamp, NULL);
            MoveForward(&send_window, queue);
        }
        /*
         * if there is no available data, then begin to wait for the ack and free the data
         */
        recv_num = recvfrom(sock, recv_packet, sizeof(ackpacket), 0, (struct sockaddr *) &sin, &sinlen);
        /*
         * if the ack is correct, and it is equal to the smallest ack in queue, accept it, otherwise, ignore it
         */
        if (recv_num > 0) {
            last_inorder_ack = recv_packet->last_inorder_ack;
            new_checksum = recv_packet->ack_checksum;
            recv_packet->ack_checksum = 0;
            if (cksum((u_short*) recv_packet, sizeof(ackpacket)/2) == new_checksum){
                if (recv_packet->isFin == FIN){
                    gettimeofday(&send_end_time, NULL);
                    latency = getLatency(&send_start_time, &send_end_time)/1000;
                    printf("[Finish File Transmission], Using %ld s\t %ld ms\n", latency/1000, latency%1000);
                    break;
                }

                if (last_inorder_ack >= Front(queue)->seq_num && last_inorder_ack < (Front(queue)->seq_num + queue->size)) {
                    initAck.ackNum = last_inorder_ack;
                    initAck.count = 1;
                    while (queue->size > 0 && last_inorder_ack >= Front(queue)->seq_num) {
                        Dequeue(queue);
                        send_window.usable++;
                    }

                    /*
                     * first case, ack_num <= inorder_ack, all ack are inorder, no need to do further check
                     * second case, ack > inorder_ack, receive the out-of-order ack
                     */
                    if (last_inorder_ack < recv_packet->ack_num){
//                    if (recv_packet->ack_num >= Front(queue)->seq_num && recv_packet->ack_num < (Front(queue)->seq_num + queue->size)) {
                        if (recv_packet->ack_num >= Front(queue)->seq_num) {
                            for (int i = queue->front; i < (queue->front + queue->size); i++) {
                                if (queue->data[i % QUEUE_SIZE]->seq_num == recv_packet->ack_num) {
                                    is_ack[i % QUEUE_SIZE] = true;
                                    break;
                                }
                            }

                            // pop all the continuous ack
                            while (queue->size>0 && is_ack[queue->front]){
                                Dequeue(queue);
                                send_window.usable ++;
                            }
                        }
                    }
                } else {
//                if (recv_packet->ack_num >= Front(queue)->seq_num && recv_packet->ack_num < (Front(queue)->seq_num + queue->size)) {
                    if (recv_packet->ack_num >= Front(queue)->seq_num) {
//                        printf("[Recv ACK Disorder] %u \n", recv_packet->ack_num);
                        for (int i = queue->front; i < (queue->front + queue->size); i++) {
                            if (queue->data[i % QUEUE_SIZE]->seq_num == recv_packet->ack_num) {
                                is_ack[i % QUEUE_SIZE] = true;
                                break;
                            }
                        }
                    }

                    if (last_inorder_ack == initAck.ackNum) {
                        initAck.count += 1;
                        if (initAck.count == RESENDLIMIT) {
                            /*
                             * if we receive the same ack for three times, then we need to resend this packet
                             */
                            send_packet = Front(queue);
                            offset = (send_packet->seq_num-1) * DATASIZE;
                            send_num = sendto(sock, send_packet, sizeof(*send_packet), 0,
                                              (const struct sockaddr *) &sin, sizeof(sin));

                            printf("[resend data, 3 same ack] %u (%u) \n", offset, send_packet->payload_size);
                            while (send_num <= 0) {
                                send_num = sendto(sock, send_packet, sizeof(*send_packet), 0,
                                                  (const struct sockaddr *) &sin, sizeof(sin));
//                                printf("[MORE THAN 3 ACK RESEND] (%u)\t finish!!! \n", send_packet->seq_num);
                                printf("[resend data, 3 same ack] %u (%u) \n", offset, send_packet->payload_size);
                            }

                            initAck.count = 0;
                            initAck.ackNum = -1;
                            gettimeofday(&send_time[queue->front], NULL);
                        }
                    }else {
                        initAck.ackNum = last_inorder_ack;
                        initAck.count = 1;
                    }
//                }
                }
            }else{
                printf("[recv corrupted ack] %u", recv_packet->ack_num);
            }
        }else{
            gettimeofday(&cur_timestamp, NULL);
            latency = getLatency(&last_send_tstamp, &cur_timestamp);
            int resend_no;
            if ((latency >= RESENDTIMELIMIT) && (queue->size > 0)) {
                /*
                * time exceed, retransmit the packet
                 */
                for (resend_no = queue->front; resend_no < (queue->front + queue->size); resend_no++){
                    if (!is_ack[resend_no % QUEUE_SIZE]) {
                        send_packet = queue->data[resend_no % QUEUE_SIZE];
                        send_num = sendto(sock, send_packet, sizeof(*send_packet), 0, (const struct sockaddr *) &sin,
                                          sizeof(sin));
                        offset = (send_packet->seq_num-1) * DATASIZE;
                        printf("[resend data, timeout] %u (%u) \n", offset, send_packet->payload_size);
                        while (send_num <= 0) {
                            send_num = sendto(sock, send_packet, sizeof(*send_packet), 0, (const struct sockaddr *) &sin,
                                              sizeof(sin));
                            printf("[resend data, timeout] %u (%u) \n", offset, send_packet->payload_size);
                        }
                        gettimeofday(&last_send_tstamp, NULL);
                    }
                }

                gettimeofday(&last_send_tstamp, NULL);
            }
        }
    }

    send_fin_packet->ack_num = FINACK;
    send_fin_packet->last_inorder_ack = FINACK;
    send_fin_packet->isFin = FINACK;
    fillackPacket(send_fin_packet);
    send_num = sendto(sock, send_fin_packet, sizeof(*send_fin_packet), 0, (const struct sockaddr *) &sin, sizeof(sin));
    gettimeofday(&last_send_tstamp, NULL);

    while (true){
        recv_num = recvfrom(sock, recv_packet, sizeof(ackpacket), 0, (struct sockaddr *)&sin, &sinlen);
        if (recv_num > 0 && recv_packet->isFin == FIN){
            send_num = sendto(sock, send_fin_packet, sizeof(*send_fin_packet), 0, (const struct sockaddr *) &sin, sizeof(sin));
            gettimeofday(&last_send_tstamp, NULL);
        }else{
            gettimeofday(&cur_timestamp, NULL);
            latency = getLatency(&last_send_tstamp, &cur_timestamp);
            if (latency > FIN_TIMEOUT){
                break;
            }
        }
    }

    printf("Exit Successfully\n");
    free(queue);
    exit(0);
}