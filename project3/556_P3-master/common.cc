//
// Created by Zequn Jiang on 4/2/20.
//

#include "common.h"

vector<PacketPair>* parsePacketPairs(void *start, int pair_size) {
    auto pairs = new vector<PacketPair>();
    for (int i = 0; i < pair_size; i++) {
        unsigned short first = ntohs(*((unsigned short*) start + i * 2));
        unsigned short second = ntohs(*((unsigned short*) start + i * 2 + 1));
        auto pair = PacketPair(first, second);
        pairs->push_back(pair);
    }
    return pairs;
}

ePacketType getPacketType(void *packet) {
    return (ePacketType)(*((unsigned char *)packet));
}

unsigned short getSize(void *packet) {
    return *((unsigned short *)packet + 1);
}

void checkType(void *packet, ePacketType type) {
    if (getPacketType(packet) != type) {
        cout << "packet type should be " <<  type << endl;
        exit(1);
    }
}
