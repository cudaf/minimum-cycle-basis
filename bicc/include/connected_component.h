#pragma once
#include <atomic>
#include <stack>
#include <unordered_map>
#include <utility>
#include <vector>
#include <cmath>
#include <list>

#include "CsrGraph.h"
#include "dfs_helper.h"
#include "bicc.h"

using std::pair;
using std::list;
using std::unordered_map;


inline uint64_t merge_32bits(uint64_t upper, uint64_t lower) {
  uint64_t result = 0;
  result = ((upper << 32) | lower);
  return result;
}

struct Connected_Components {
  int component_number;
  int *new_component_number;
  bicc_graph *graph;
  dfs_helper *helper;
  int count_components, time;  //count indicates number of bridges
  unordered_map<unsigned long long, int> *edge_map;
  list<pair<int, list<int>*> > store_biconnected_edges;

  Connected_Components(int c_number, int *new_c_number, bicc_graph *gr,
      dfs_helper *helper_struct, unordered_map<unsigned long long, int> *bi_map) {
    component_number = c_number;
    new_component_number = new_c_number;
    graph = gr;
    helper = helper_struct;
    count_components = 0;
    time = 0;
    edge_map = bi_map;
  }

  void dfs(int src) {
    helper->low[src] = helper->discovery[src] = ++time;
    for (int j = graph->c_graph->rowOffsets->at(src); j < graph->c_graph->rowOffsets->at(src + 1); j++) {
      int dest = graph->c_graph->cols->at(j);
      if (helper->discovery[dest] == -1) {
        graph->bicc_number[j] = *new_component_number;
        graph->bicc_number[edge_map->at(merge_32bits(dest, src))] = *new_component_number;
        dfs(dest);
      } else {
        graph->bicc_number[j] = *new_component_number;
        graph->bicc_number[edge_map->at(merge_32bits(dest, src))] = *new_component_number;
      }
    }
  }
};

/**
 * @brief This method is used to mark the connected components of the initial graph.
 * @details We do a dfs across the entire graph and mark the vertices by the current component
 * number as and when we get a new component. We mark both the forward and reverse edges of the components.
 * 
 * @param bicc_number 
 * @param new_bicc_number storage for new connected component number.
 * @param graph Input Graph.
 * @param helper helper struct for dfs 
 * @param edge_map mapping from <src,dest> ==> edge_index
 */
int obtain_connected_components(int bicc_number, int &new_bicc_number,
    bicc_graph *graph, dfs_helper *helper, unordered_map<unsigned long long, int> *edge_map) {
  helper->initialize_arrays();
  Connected_Components component(bicc_number, &new_bicc_number, graph, helper, edge_map);
  debug("graph->nodes", graph->Nodes);

  for (int src = 0; src < graph->Nodes; src++) {
    if ((graph->c_graph->rowOffsets->at(src + 1) - graph->c_graph->rowOffsets->at(src)) > 0) {
      if (helper->discovery[src] == -1) {
        component.count_components++;
        new_bicc_number++;
        component.dfs(src);
      }
    }
  }
  return component.count_components;
}
