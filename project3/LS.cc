//
// Created by zhoutt96 on 4/6/20.
//


#include "RoutingProtocolImpl.h"

unsigned int RoutingProtocolImpl::ls_seq_num = 0;

void RoutingProtocolImpl::get_ls_forwarding_table(){
//    printf("Calculate the forwarding table of the LS \n");
}

void RoutingProtocolImpl::init_LS_alarm(){
    void* ptr = malloc(sizeof(alarmType));
    *((alarmType*)ptr) = LS_ALARM;
    this->sys->set_alarm(this, 30*1000, ptr);
}

void RoutingProtocolImpl::updateLS(){
    printf("[Update] LS Table \n");
    this->lsp_topology_map.clear();
    this->flooding_lsp();
}

void RoutingProtocolImpl::init_LS_Protocol(){
    this->init_LS_alarm();
    this->flooding_lsp();
}

void RoutingProtocolImpl::LS_message_handler(unsigned short port, void *packet,unsigned short size){
    unsigned int lsp_seq_id =ntohl( *(unsigned int *)((char*)packet + 8));
    unsigned short source_id = ntohs(*(unsigned short *)((char*)packet + 4));

    if (this->lsp_seq_set.count(lsp_seq_id) || source_id == router_id){
        // receive this seq_id before, discard and do nothing
        free(packet);
        return;
    }else{
        // and forward to all neighbors except the source id
        this->lsp_seq_set.insert(lsp_seq_id);
        this->lsp_refresh_time_map[source_id] = sys->time();

        this->lsp_topology_map[source_id].clear();
        for(int i=0; i<num_of_port;i++){
            char* send_buffer = (char *) malloc(size);
            memcpy(send_buffer, packet, size);
            this->sys->send(i, send_buffer, size);
        }

        // store topology to local
        int i_position;
        for (i_position = PACKET_BASE_SIZE; i_position < size; i_position+=4){
            unsigned short nei_id =  ntohs(*(unsigned short *)((char*)packet + i_position));
            unsigned short cost =  ntohs(*(unsigned short *)((char*)packet + i_position+2));
//            printf("Recv nei id %u, and cost %u on node %u\n", nei_id, cost, this->router_id);
            lsp_topology_map[source_id].insert(Topology_Info(nei_id,cost));
            lsp_refresh_time_map[source_id] = sys->time();
        }

    }
    this->print_flooding_table();
//    this->Dijkstra();
//    print_forwarding_table();
    free(packet);
}

void RoutingProtocolImpl::forward_message_LS(unsigned short port, void *packet,unsigned short size){

}

void RoutingProtocolImpl::flooding_lsp(){

    if (get_nei_num() > 0){
        unsigned short payload_size = get_nei_num()*4;
        unsigned short total_size = payload_size+PACKET_BASE_SIZE;
        char* buffer = (char *) malloc(total_size);
        *(ePacketType *)(buffer) = (ePacketType) LS;
        *(unsigned short *) (buffer+2) = htons((unsigned short) total_size);
        *(unsigned short *) (buffer+4) =htons((unsigned short) this->router_id);
        *(unsigned int *) (buffer+8) =  htonl(this->ls_seq_num);

        this->lsp_seq_set.insert(this->ls_seq_num);
        this->ls_seq_num++;

        int cur_start_position = PACKET_BASE_SIZE;

        for (auto it=this->port_map.begin(); it!=this->port_map.end(); ++it) {
            if (it->second.status == CONNECTED){
                // it this neighbour is valid, add it to the
                *(unsigned short*) (buffer+cur_start_position) = htons(it->second.nei_id);
                *(unsigned short*) (buffer+cur_start_position+2) = htons(it->second.link_cost);
                cur_start_position+=4;
//                lsp_topology_map[router_id].insert(Topology_Info(it->second.nei_id, it->second.link_cost));
            }
        }

        for (int i=0; i<num_of_port;i++){
            char* send_buffer = (char *) malloc(total_size);
            memcpy(send_buffer, buffer, total_size);
            sys->send(i, send_buffer, total_size);
        }
    }
}

void RoutingProtocolImpl::LS_alarm_handler(void *data) {
    this->flooding_lsp();
    sys->set_alarm(this, 30*1000, data);
}

void RoutingProtocolImpl::print_flooding_table(){
    printf("\n******************** LS Flooding Table *********************\n");
    cout<<"time = "<<this->sys->time()/1000.0<<" ";
    cout<<"Print Flooding Table on Node "<<this->router_id<<"\n";
    for (auto it=this->lsp_topology_map.begin(); it!=this->lsp_topology_map.end(); ++it) {
        cout << " ****** Source id is " << it->first <<", it has "<<it->second.size()<<" neighbors \n";
        for (auto itr = it->second.begin(); itr != it->second.end(); ++itr) {
            cout << "source id "<< it->first << ",nei id " << itr->nei_id << ",cost is " << itr->cost <<"\n";
        }
    }
    printf("**************************** End ****************************\n");
}

void RoutingProtocolImpl::LS_expire_alarm_handler(void *data){
    int is_delete = 0;
    printf("LS expire alarm on node %d ######## \n", router_id);
    std::vector<unsigned short> to_be_deleted;
    for (auto it=this->lsp_topology_map.begin(); it!=this->lsp_topology_map.end(); ++it) {
        unsigned int duration = sys->time() - lsp_refresh_time_map[it->first];
        printf("duration is %d \n",duration);
        if (duration> 45*1000){
            is_delete  = 1;
            printf("this ls entry expires %d, duration is %d \n", it->first, duration);
            to_be_deleted.push_back(it->first);
        }
    }

    for (std::vector<unsigned short>::iterator it=to_be_deleted.begin(); it !=to_be_deleted.end(); ++it){
        printf("Delete info of node %d on node %d", *it, router_id);
        lsp_topology_map.erase(*it);
        lsp_refresh_time_map.erase(*it);
    }
    if (is_delete == 1){
        print_flooding_table();
    }
}


void RoutingProtocolImpl::delete_nei_in_lsp(unsigned short nei_id){
    if (lsp_topology_map.count(nei_id)){
        lsp_topology_map.erase(nei_id);
        lsp_refresh_time_map.erase(nei_id);
    }
}

void RoutingProtocolImpl:: Dijkstra() {
    // initialize
    /*
     * adjacent matrix: row: source, col: target, value: cost
     * */
    int node_size=lsp_topology_map.size(); // all the nodes we can get
    unordered_map<unsigned short, unsigned short> des_map; // <target_id, cost>
    auto comparator =[] (unsigned short*link1 , unsigned short *link2) {
        return link1[2]>link2[2];
    };
    priority_queue<unsigned short*, vector<unsigned short*>, decltype(comparator)> pq (comparator);
    unsigned short source[]={router_id,router_id,0};
    pq.push(source);

    while(des_map.size()<node_size && !pq.empty())
    {
        int curDes=INFINITY_COST;
        while(!pq.empty())
        {
            unsigned short *link= pq.top();
            pq.pop();
            //put it into destinations
            if(!des_map.count(link[1]))
            {
                des_map[link[1]]=link[2];
                curDes=link[1];
                // update the forwarding table
                if(link[0]!=router_id)
                {
//                    (*forwarding_table)[link[1]]=(*forwarding_table)[link[0]]; // destionation: nexthop to go to des
                    //then continue find the new destination
                    forwarding_table[link[1]]=forwarding_table[link[0]];
                    break;
                }
            }
        }

        // when no avaiable nodes to add->break
        if(curDes==INFINITY_COST)
            break;

        // update the neighbors info
        auto neighbors= this->lsp_topology_map[curDes];
        for(auto neighborNode: neighbors )
        {
            if(neighborNode.cost==INFINITY_COST)
                continue;
            // add into pq
            if(neighborNode.nei_id!=curDes && des_map.count(neighborNode.nei_id)==0)
            {
                unsigned short curLink[] = {(unsigned short)curDes, neighborNode.nei_id,(unsigned short)neighborNode.cost};
                pq.push(curLink);
            }
        }
    }
    print_forwarding_table();
}

void RoutingProtocolImpl::print_forwarding_table(){
    printf("\n------------------ Forwarding Table --------------------\n");
    cout<<"time = "<<this->sys->time()/1000.0<<" ";
    cout<<"Print Forwarding Table on Node "<<this->router_id<<"length is "<<this->forwarding_table.size()<<"\n";
    for (auto it=this->forwarding_table.begin(); it!=this->forwarding_table.end(); ++it) {
        cout << "source id " << it->first <<", next node is "<<it->second<<"\n";
    }
    printf("------------------ end  --------------------\n");
}




