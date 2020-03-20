#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "utils.h"
#include <sys/stat.h>
#include <dirent.h>
//#include <jmorecfg.h>
//#include "checksum.h"
#include "linkList.h"
#include <queue>
using namespace std;

/**************************************************/
/* a few simple linked list functions             */
/**************************************************/


void helpMenu()
{
    printf("--------------             Menu for COMP 556 Project 2             --------------|\n");
    printf("|./recvfile -h:                                                    show help menu|\n");
    printf("|./recvfile -p <recv port>                                                       |\n");
    printf("|    <recv port> (Required):                                                     |\n"
           "|         The UDP port to listen on. recvfile’s SOCK DGRAM socket must           |\n"
           "|         be assigned this port number using bind().                             |\n"
           "|         On CLEAR, the valid values to use (i.e. not blocked                    |\n"
           "|         by the firewall) are 18000-18200 inclusive.                            |\n");
    printf("--------------                 End of the Menu                     --------------|\n");
}

//u_short cksum(u_short *buf, int count)
//{
//    register u_long sum = 0;
//    while (count--)
//    {
//        sum += *buf++;
//        if (sum & 0xFFFF0000)
//        {
//            /* carry occurred, so wrap around */
//            sum &= 0xFFFF;
//            sum++;
//        }
//    }
//    return ~(sum & 0xFFFF);
//}


/*****************************************/
/* main program                          */
/*****************************************/

/* simple server, takes one parameter, the server port number */
int main(int argc, char **argv) {

    if (strcmp(argv[1], "-h") == 0)
    {
        helpMenu();
        abort();
    }

    int sock;
    unsigned short server_port = atoi(argv[2]);
    struct sockaddr_in sin, addr; // the server address and the client address
    socklen_t addrlen = sizeof(addr);

    // create socket file descripter
    if ((sock=socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        perror("Can not create UDP socket");
        abort();
    }

    memset (&sin, 0, sizeof (sin));
    memset (&addr, 0, sizeof (addr));

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons (server_port);

    /* bind server socket to the address */
    if (bind(sock, (struct sockaddr *) &sin, sizeof (sin)) < 0)
    {
        perror("binding socket to address");
        abort();
    }


    int num;
    int receive_correct_file = 0;
    char* buffer = (char*) malloc(sizeof(packet));
    packet *recv_packet = (packet*) buffer;
    ackpacket *ack_packet = (ackpacket*) malloc(sizeof(ackpacket));


    while (receive_correct_file==0)
    {
        recvfrom(sock, buffer, sizeof(packet), 0, (struct sockaddr *)&addr, &addrlen);
        u_short checksum = recv_packet->checksum;
        recv_packet->checksum = 0;
        //debug，长度
        u_short newCheckSum = cksum((u_short*)buffer, sizeof(*recv_packet)/2);
//        u_short newCheckSum = cksum((u_short*)buffer, sizeof((DATASIZE+10)/2));

        if (checksum == newCheckSum)  //modify
        {
            // send a ack back to the client
            printf("receive the filename successfully \n");
            receive_correct_file = 1;
            ack_packet->ack_num = recv_packet->seq_num;
            ack_packet->last_inorder_ack = recv_packet->seq_num;
            sendto(sock, ack_packet, sizeof(*ack_packet), 0, (const struct sockaddr *) &addr, sizeof(addr));
        }else{
            printf("recv corrupt packet\n");
        }
    }

    char path[256];
    int len = strlen(recv_packet->data);
    int i;
    for(i = len-1;i>=0;i-- ){
        if(recv_packet->data[i] == '/')
            break;
    }
    memcpy(path, recv_packet->data, i+1);
    path[i+1]='\0';
    if(NULL == opendir(path)) {
        int ret = mkdir(path, 0775);
        if(ret<0){
            printf("Could not create directory \'%s\' \n", path);
            exit(0);
        }
    }
    strcat(recv_packet->data,".recv");


    FILE *fp = NULL;
//    int filesize;

    fp = fopen(recv_packet->data, "wb+");
    if(fp!=NULL){
        printf("Successful create file: %s\n",recv_packet->data);
    }else{
        printf("Cannot create file: %s\n",recv_packet->data);
        exit(0);
    }
    unsigned int ack = 0;
    //debug，注意接收窗口
    int window_size = DEFAULTMAXWINDOWSIZE;
    printf("window size is %d \n", window_size);
    short endFile = 0;
    LinkList list;
    unsigned long offset = 0;
    unsigned long length = 0;
    while(endFile == 0 || !list.isEmpty()){
        num = recvfrom(sock, buffer, sizeof(packet), 0, (struct sockaddr *)&addr, &addrlen);
        if(num>0){
            recv_packet = (packet*)buffer;
            unsigned int checksum = recv_packet->checksum;
            recv_packet->checksum = 0;
            //debug
            if (checksum == cksum((unsigned short*)recv_packet, sizeof(*recv_packet)/2)){
                offset = (recv_packet->seq_num-1)*sizeof(recv_packet->data);
                length = recv_packet->payload_size;
                //debug，ack是当前接收到数据的最小包号，这部分已发送了ACK
                if(recv_packet->seq_num<= ack || recv_packet->seq_num>(ack+window_size)){
                    printf("[recv data] %lu (%lu) %u %s \n",offset, length, recv_packet->seq_num,"IGNORED");
//                    ack_packet->ack_num = recv_packet->seq_num;
                    ack_packet->ack_num = IGNORE_CODE;
                    ack_packet->last_inorder_ack = ack;
                }else{
                    if(endFile==0){
                        endFile = recv_packet->isEnd;
                    }
                    //debug,按顺序接收
                    if(recv_packet->seq_num == ack+1 && list.isEmpty()){
                        printf("[recv data] %lu (%lu) %u %s \n",offset, length, recv_packet->seq_num, "ACCEPTED(in-order)");
                        fwrite(recv_packet->data,1,length,fp);
//                        printf("write seq %u, length is \n", recv_packet->seq_num, recv_packet->payload_size);
                        ack++;
                        ack_packet->ack_num = ack;
                        ack_packet->last_inorder_ack = ack;
                    }else {
                        printf("[recv data] %lu (%lu) %u %s \n", offset, length, recv_packet->seq_num, "ACCEPTED(out-of-order)");
                        list.add(recv_packet->seq_num,recv_packet->data,length);
                        ListNode* content = list.pop(ack);
                        while(content!=NULL){
                            printf("write seq %u \n", content->seq);
                            fwrite(content->data,1,content->payload_size,fp);
                            ack = content->seq;
                            ListNode* temp = content;
                            content = content->next;
                            delete temp;
                        }
                        ack_packet->ack_num = recv_packet->seq_num;
                        ack_packet->last_inorder_ack = ack;
                    }
                }
            }else{
                printf("[recv corrupt packet], seq %u, checksum %d  \n", recv_packet->seq_num, checksum);
//                ack_packet->ack_num = ERROR_NUM;
                ack_packet->ack_num = IGNORE_CODE;
                ack_packet->last_inorder_ack = ack;
//                printf("size if %lu", sizeof(*recv_packet)/2);
//                printf("queue rear is %d \n", );
//                printf("data size i")
//                printf("[recv corrupt packet]\n");
            }

            //ack_packet->ack_num = ack;
            //ack_packet->last_inorder_ack = ack;
            printf("[send ack] %u %u \n", ack_packet->ack_num, ack);
            fillackPacket(ack_packet);
            sendto(sock, ack_packet, sizeof(*ack_packet), 0, (const struct sockaddr *) &addr, sizeof(addr));
        }
    }
    fclose(fp);
    printf("[completed]\n");
    exit(0);

}
