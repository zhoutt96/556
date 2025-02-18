//
// Created by zhoutt96 on 3/13/20.
//

//#ifndef INC_556PJ2_LINKLIST_H
//#define INC_556PJ2_LINKLIST_H

#ifndef PROJECT2_LINKLIST_H
#define PROJECT2_LINKLIST_H
#include "utils.h"
#include <string.h>
#include <stdlib.h>

class ListNode{
public:
    unsigned int seq;
    char* data;
    ListNode* next;
    unsigned long payload_size;

    ListNode(int seq_num,char* content, unsigned long length){
        seq = seq_num;
        data = (char*)malloc(length);
        memcpy(data,content,length);
        next = NULL;
        payload_size = length;
    }
    ListNode(){
        seq = 0;
        data = NULL;
        next = NULL;
    }
    ~ListNode(){
        if(data!=NULL){
            free(data);
        }
    }

};

class LinkList{
public:
    ListNode* head;
    int count;
    LinkList(){
        head = new ListNode();
        count = 0;
    }
    ~LinkList(){
        while(head->next!=NULL){
            ListNode* temp = head->next;
            head->next = head->next->next;
            delete temp;
        }
    }
    int add(unsigned int seq_num,char*content, unsigned long length){
        ListNode* ptr = head;
        int flag = 1;
        while(ptr->next!=NULL){
            if(ptr->next->seq<seq_num){
                ptr = ptr->next;
            }else if(ptr->next->seq==seq_num){
                return 1;
            }else{
                ListNode* node = new ListNode(seq_num,content,length);
                node->next = ptr->next;
                ptr->next = node;
                count++;
                flag = 0;
            }
        }
        if(flag){
            ListNode* node = new ListNode(seq_num,content,length);
            node->next = ptr->next;
            ptr->next = node;
            count++;
        }
        return 1;
    }
    bool isEmpty(){
        return head->next == NULL;
    }
    ListNode* pop(unsigned int ack){
        if(head->next==NULL)
            return NULL;
        if(head->next->seq!=ack+1)
            return NULL;
        ListNode* result = head->next;
        ListNode* ptr = result;
        int flag = 1;
        while(ptr->next!=NULL){
            if(ptr->seq == ptr->next->seq-1){
                ptr = ptr->next;
                count--;
            }else{
                head->next = ptr->next;
                ptr->next = NULL;
                count--;
                flag = 0;
                break;
            }
        }
        if(flag){
            head->next = NULL;
            count = 0;
        }
        return result;
    }


};
#endif //INC_556PJ2_LINKLIST_H
