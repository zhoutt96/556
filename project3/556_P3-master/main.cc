#include <iostream>

#include <vector>
#include <queue>
#include <unordered_map>
#include "global.h"

using namespace std;
int networkDelayTime(vector<vector<int>>& times, int N, int K);
int main() {

    auto compare = [](int* lhs, int* rhs) {
        return lhs[2] > rhs[2];
    };
    priority_queue<int*, vector<int*>, decltype(compare)> pq(compare);
    unordered_map<unsigned short, unsigned short> map;
    int n = destinations.size();
    int tmp[]{router_id, router_id, 0};
    pq.push(tmp);
    while (map.size() < n && !pq.empty()) {
        unsigned short cur = INFINITY_COST;
        while (!pq.empty()) {
            int* a = pq.top();
            pq.pop();
            if (map.count(a[1]) == 0) {
                map[a[1]] = a[2];
                cur = a[1];
                break;
            }
        }
        if (cur == INFINITY_COST) {
            break;
        }
        auto adjs = (*LS_table)[cur];
        for (auto it : adjs) {
            if (it.second.cost == INFINITY_COST) {
                continue;
            }
            if (it.first != cur && map.count(it.first) == 0) {
                int *tmp1 {new int[3]{cur, it.first, map[cur] + it.second.cost}};
                pq.push(tmp1);
            }
        }
    }

}
