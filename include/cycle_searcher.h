#pragma once
#include <vector>
#include <unordered_map>
#include <utility>
#include <climits>
#include "Cycle.h"

using std::vector;
using std::unordered_map;


struct CommonCycles {
  vector<Cycle*> listed_cycles;

  CommonCycles(Cycle *cle) {
    listed_cycles.push_back(cle);
  }

  inline void add_cycle(Cycle *cle) {
    listed_cycles.push_back(cle);
  }
};

struct cycle_storage {
  int Nodes;
  vector<unordered_map<uint64_t, CommonCycles*> > list_cycles;

  inline uint64_t combine(int u, int v) {
    uint64_t value = u;
    value <<= 32;
    value = value | v;
    return value;
  }

  cycle_storage(int N) {
    Nodes = N;
    list_cycles.resize(Nodes);
  }

  ~cycle_storage() {
    list_cycles.clear();
  }

  void add_cycle(int root, int u, int v, Cycle *cle) {
    uint64_t index = combine(std::min(u, v), std::max(u, v));
    if (list_cycles[root].find(index) == list_cycles[root].end())
      list_cycles[root].insert(std::make_pair(index, new CommonCycles(cle)));
    else
      list_cycles[root][index]->add_cycle(cle);
  }

  void clear_cycles() {
    for (int i = 0; i < list_cycles.size(); i++) {
      list_cycles[i].clear();
    }
    list_cycles.clear();
  }
};
