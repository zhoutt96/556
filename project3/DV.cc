//
// Created by zhoutt96 on 4/6/20.
//


#include "utils.h"
#include "RoutingProtocolImpl.h"
#include "utils.h"
#include <unordered_map>
#include <vector>
using namespace std;

void RoutingProtocolImpl::init_DV_alarm(){
    void* ptr = malloc(sizeof(alarmType));
    *((alarmType*)ptr) = DV_ALARM;
    this->sys->set_alarm(this, 30*1000, ptr);
}

void RoutingProtocolImpl::DV_alarm_handler(void *data) {
    DV_sendUpdateMsg();
    sys->set_alarm(this, 30*1000, data);
}

void RoutingProtocolImpl::setLastUpdateTime(unsigned short des){
    if(dvp_refresh_time_map.count(des) == 0){
        dvp_refresh_time_map.insert(make_pair(des,sys->time()));
    }else{
        dvp_refresh_time_map[des] = sys->time();
    }
}

void RoutingProtocolImpl::updateDVTableByMsg(unsigned short port, vector<pair<unsigned short, unsigned short>> updateMsg){
    bool isUpdate = false;
    printf("[Update] DV table \n");
    unsigned short sendNode = port;
    unsigned short jumpCost;

    for(auto &a:updateMsg){
        if(a.first == router_id){
            if(dv_cost_map.count(sendNode) == 0){
                DVTable newRec(sendNode,sendNode,a.second);
                dv_cost_map.insert(make_pair(sendNode,newRec));
                setLastUpdateTime(sendNode);
                jumpCost = a.second;
                isUpdate = true;
            }else{
                DVTable jumpNode = dv_cost_map[sendNode];
                if(jumpNode.cost != a.second){
                    jumpNode.cost = a.second;
                    dv_cost_map[sendNode] = jumpNode;
                    isUpdate = true;
                }
                setLastUpdateTime(sendNode);
                jumpCost = a.second;
            }
            break;
        }
    }


    for(auto &a:updateMsg){

        if(a.first == router_id)
            continue;


        if(dv_cost_map.count(a.first) == 0 ){
            if(a.second != INFINITY_COST){
                DVTable newRec(a.first,sendNode,jumpCost+a.second);
                dv_cost_map.insert(make_pair(a.first,newRec));
                setLastUpdateTime(a.first);
                isUpdate = true;
            }

        }else{
            DVTable neighbor = dv_cost_map[a.first];
            if(neighbor.nextHop == sendNode){
                if(a.second == INFINITY_COST){
                    dv_cost_map.erase(a.first);
                    forwarding_table.erase(a.first);

                }else{
                    neighbor.cost = jumpCost+a.second;
                    dv_cost_map[a.first] = neighbor;
                    setLastUpdateTime(a.first);
                }
                isUpdate = true;
            }else{
                if(a.second!=INFINITY_COST){
                    unsigned short newCost = jumpCost+a.second;
                    if(newCost<=neighbor.cost) {
                        neighbor.nextHop = sendNode;
                        neighbor.cost = newCost;
                        dv_cost_map[a.first] = neighbor;
                        setLastUpdateTime(a.first);
                        isUpdate = true;
                    }
                }
            }

        }

    }
    if(isUpdate){
        updateForwardingTable();
        //DV_sendUpdateMsg();
        print_DV_table();
        printPortStatus();
    }

}

void RoutingProtocolImpl::updateForwardingTable(){
    for(auto &a:dv_cost_map){
        DVTable tempRec = a.second;
        if(forwarding_table.count(a.first)==0){
            forwarding_table.insert(make_pair(tempRec.des,tempRec.nextHop));
        }else{
            forwarding_table[a.first] = tempRec.nextHop;
        }
    }
}


void RoutingProtocolImpl::init_DV_Protocol(){
//    printf("Init DV now \n");
    init_DV_alarm();
    for(auto &a:port_map){
        PORT port = a.second;
        DVTable tempRec(port.nei_id,port.nei_id,(unsigned short)port.link_cost);
        dv_cost_map.insert(make_pair(port.nei_id,tempRec));
        setLastUpdateTime(port.nei_id);
    }

}

void RoutingProtocolImpl::DV_message_handler(unsigned short port, void *packet,unsigned short size){
    printf("[RECV] DV Message \n");
    unsigned short actual_size =  ntohs(*(unsigned short *)((char*)packet + 2));
    unsigned short source_id = ntohs(*(unsigned short *)((char*)packet + 4));
//    unsigned short des_id = ntohs(*(unsigned short *)((char*)packet + 6));

    vector<pair<unsigned short, unsigned short>> updateMsg;
    int i = 8;
    unsigned short nodeID;
    unsigned short nodeCost;
    while(i<actual_size){
        nodeID = ntohs(*(unsigned short *)((char*)packet + i));
        nodeCost = ntohs(*(unsigned short *)((char*)packet + i + 2));
        updateMsg.push_back(make_pair(nodeID,nodeCost));
        i = i + 4;
    }
    updateDVTableByMsg(source_id,updateMsg);


}

void RoutingProtocolImpl::updateLocalDVTable(unsigned short nei_id, unsigned short old_rtt, unsigned short new_rtt, bool isNew) {
    bool isUpdate = false;
    if(isNew){
        if(dv_cost_map.count(nei_id)==0){
            DVTable newRec(nei_id,nei_id,new_rtt);
            dv_cost_map.insert(make_pair(nei_id,newRec));
            forwarding_table.insert(make_pair(newRec.des,newRec.nextHop));
            setLastUpdateTime(newRec.des);
            isUpdate = true;
        }else{
            DVTable rec = dv_cost_map[nei_id];
            if(rec.cost>new_rtt){
                rec.cost = new_rtt;
                rec.nextHop = nei_id;
                dv_cost_map[nei_id] = rec;
                forwarding_table[rec.des] = rec.nextHop;
                setLastUpdateTime(rec.des);
                isUpdate = true;
            }
        }
    }else{
        for(auto &r:dv_cost_map){
            DVTable table = r.second;
            if(table.nextHop == nei_id){
                if(new_rtt == INFINITY_COST){
                    dv_cost_map.erase(r.first);
                    forwarding_table.erase(table.des);
                    isUpdate = true;
                    continue;
                }else{
                    table.cost = table.cost-old_rtt+new_rtt;
                    dv_cost_map[r.first] = table;
                    isUpdate = true;
                }
                setLastUpdateTime(r.first);
            }
            if(table.des == nei_id){
                if(table.cost>new_rtt){
                    table.cost = new_rtt;
                    table.nextHop = nei_id;
                    dv_cost_map[table.des] = table;
                    forwarding_table[table.des] = table.nextHop;
                    isUpdate = true;
                    setLastUpdateTime(table.des);
                }
            }

        }
    }
    if(isUpdate){
        DV_sendUpdateMsg();
        print_DV_table();
        printPortStatus();
    }
}




void RoutingProtocolImpl::DV_sendUpdateMsg(){
    if(get_nei_num()<=0)
        return;
    int numDVTable = dv_cost_map.size();
    unsigned short total_size = 4*numDVTable+8;
    char* buffer = (char *)malloc(total_size);
    *(ePacketType *)(buffer) = (ePacketType) DV;
    *(unsigned short *) (buffer+2) = htons((unsigned short) total_size);
    *(unsigned short *) (buffer+4) =htons((unsigned short) this->router_id);

    for(auto &p : port_map){
        PORT port = p.second;
        if(port.status == CONNECTED){
            *(unsigned short *) (buffer+6) = htons((unsigned short) port.nei_id);
            int i = 8;
            for(auto &a:dv_cost_map){
                unsigned short node_id = a.second.des;
                unsigned short cost = a.second.cost;
                if(port.nei_id == a.second.nextHop && a.second.des!=a.second.nextHop)  //poison reverse
                    cost = INFINITY_COST;
                *(unsigned short*) (buffer+i) = htons(node_id);
                *(unsigned short*) (buffer+i+2) = htons(cost);
                i = i + 4;
            }
            char* send_buffer = (char*) malloc(total_size);
            memcpy(send_buffer,buffer,total_size);
            sys->send(port.port_id,send_buffer,total_size);
        }
    }


}

void RoutingProtocolImpl::print_DV_table(){
    printf("\n******************** DV Table *****************************\n");
    cout<<"time= "<<this->sys->time()/1000.0<<" ";
    cout<<"Print DV Table on Node "<<this->router_id<<"\n";
    cout<<"Des Node \t Next Hop \t Cost \n";
    for(auto &a:dv_cost_map){
        DVTable table = a.second;
        cout<<table.des<<" \t\t "<<table.nextHop<<" \t\t "<<table.cost<<" \n";
    }
    printf("**************************** End ****************************\n");

}

void RoutingProtocolImpl::DV_expire_alarm_handler(void *data){
    vector<unsigned short> to_be_deleted;
    for(auto &rec : dv_cost_map){
        unsigned int duration = sys->time() - dvp_refresh_time_map[rec.first];
        if(duration > 45*1000){
            to_be_deleted.push_back(rec.first);
        }
    }
    for(auto &v:to_be_deleted){
        dv_cost_map.erase(v);
        forwarding_table.erase(v);
    }
    if(to_be_deleted.size()>0){
        print_DV_table();
        printPortStatus();
    }
}