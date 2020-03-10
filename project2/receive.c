
// Created by zhoutt96 on 3/10/20.


//#include <errno.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <sys/types.h>
//#include <unistd.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>
//#include <fcntl.h>
//#include "utils.h"
//#include <sys/stat.h>
//#include <jmorecfg.h>
//
///**************************************************/
///* a few simple linked list functions             */
///**************************************************/
//
//
//void helpMenu()
//{
//    printf("--------------             Menu for COMP 556 Project 2             --------------|\n");
//    printf("|./recvfile -h:                                                    show help menu|\n");
//    printf("|./recvfile -p <recv port>                                                       |\n");
//    printf("|    <recv port> (Required):                                                     |\n"
//           "|         The UDP port to listen on. recvfile’s SOCK DGRAM socket must           |\n"
//           "|         be assigned this port number using bind().                             |\n"
//           "|         On CLEAR, the valid values to use (i.e. not blocked                    |\n"
//           "|         by the firewall) are 18000-18200 inclusive.                            |\n");
//    printf("--------------                 End of the Menu                     --------------|\n");
//}
//
//
///*****************************************/
///* main program                          */
///*****************************************/
//
///* simple server, takes one parameter, the server port number */
//int main(int argc, char **argv) {
//
//    if (strcmp(argv[1], "-h") == 0)
//    {
//        helpMenu();
//        abort();
//    }
//
//    int sock;
//    unsigned short server_port = atoi(argv[2]);
//    struct sockaddr_in sin, addr; // the server address and the client address
//    socklen_t addrlen = sizeof(addr);
//
//
//    // create socket file descripter
//    if ((sock=socket(AF_INET, SOCK_DGRAM, 0)) < 0){
//        perror("Can not create UDP socket");
//        abort();
//    }
//
//    memset (&sin, 0, sizeof (sin));
//    memset (&addr, 0, sizeof (addr));
//
//    sin.sin_family = AF_INET;
//    sin.sin_addr.s_addr = INADDR_ANY;
//    sin.sin_port = htons (server_port);
//
//    /* bind server socket to the address */
//    if (bind(sock, (struct sockaddr *) &sin, sizeof (sin)) < 0)
//    {
//        perror("binding socket to address");
//        abort();
//    }
//
//    int num;
////    boolean receive_correct_file = FALSE;
//    packet *recv_packet = (packet*) malloc(sizeof(packet));
//    ackpacket *ack_packet = (ackpacket*) malloc(sizeof(ackpacket));
//    num = recvfrom(sock, recv_packet, sizeof(packet), 0, (struct sockaddr *)&addr, &addrlen);
//
//    while(num){
//        printf("the length of received number %d \n", num);
//        printf("the filename is %s\n", recv_packet->data);
//        printf("the seq num is %d \n", recv_packet->seq_num);
//        printf(" --------------------------------- \n");
//        num = recvfrom(sock, recv_packet, sizeof(packet), 0, (struct sockaddr *)&addr, &addrlen);
//    }



//    while (receive_correct_file==FALSE)
//    {
//        recvfrom(sock, buffer, sizeof(packet), 0, (struct sockaddr *)&addr, &addrlen);
//        ack_packet->ack_num = recv_packet->seq_num + sizeof(recv_packet->data);
//        if (recv_packet->payload_checksum == crc_16((unsigned char*)recv_packet->data, sizeof(recv_packet->data)))
//        {
//            // send a ack back to the client
//            printf("receive the filename successfully %s \n", recv_packet->data);
//            receive_correct_file = TRUE;
////            printf("send the acknum %lu",  recv_packet->seq_num + sizeof(recv_packet->data));
//
//
//            printf("send the acknum %u", ack_packet->ack_num);
//            sendto(sock, &ack_packet, sizeof(ack_packet), 0, (const struct sockaddr *) &addr, sizeof(addrlen));
//        }
//    }

    FILE *fp = NULL;
    int filesize;

//    fp = fopen(recv_packet->data, "r");

}



