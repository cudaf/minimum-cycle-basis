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
using std::make_pair;
using std::min;


inline uint64_t merge(uint64_t upper, uint64_t lower) {
  uint64_t result = 0;
  result = ((upper << 32) | lower);
  return result;
}

/**
 * @brief This method is used to store an unordered_map<uint64_t,int>
 * @details This unordered map is generated as src-dest and index of the edge .
 * 
 * @param graph c_graph representation
 * @return [description]
 */
unordered_map<uint64_t, int> *create_map(CsrGraph *graph) {
  unordered_map<uint64_t, int> *edge_map = NULL;

  if (edge_map != NULL)
    return edge_map;

  unordered_map<uint64_t, int> *custom_map = new unordered_map<uint64_t, int>();
  uint64_t result;
  uint64_t src;
  uint64_t dest;

  for (int i = 0; i < graph->cols->size(); i++) {
    src = graph->rows->at(i);
    dest = graph->cols->at(i);
    result = merge(src, dest);
    custom_map->insert(make_pair(result, i));
  }
  edge_map = custom_map;
  return edge_map;

}

struct DFS {
  int component_number;
  int *new_component_number;
  bicc_graph *graph;
  list<int> *bicc_edges;
  dfs_helper *helper;
  int count_bridges, time;  //count indicates number of bridges
  unordered_map<uint64_t, int> *edge_map;
  list<pair<int, list<int>*> > store_biconnected_edges;
  bool keep_bridges = true;

  DFS(int c_number, int *new_c_number, bicc_graph *gr,
      list<int> *bi_edges, dfs_helper *helper_struct,
      unordered_map<uint64_t, int> *bi_map,
      bool keep_bridges_param) {
    component_number = c_number;
    new_component_number = new_c_number;
    graph = gr;
    bicc_edges = bi_edges;
    helper = helper_struct;
    count_bridges = 0;
    edge_map = bi_map;
    keep_bridges = keep_bridges_param;
    time = 0;
  }

  /**
   * @brief This method runs a dfs within a biconnected component in order to obtain biconnected components.
   * The component number is stored in the bicc_graph->bicc_number value. We skip edges which 
   * donot have the same component number.
   * 
   * @param src Vertex to start dfs.
   */
  void dfs(int src) {
    ////debug("Start Src : ",src + 1);
    int dest;
    helper->parent[src] = -1;
    helper->_stack.push(src);

    while (!helper->_stack.empty()) {
      src = helper->_stack.top();
      //State 0 is used to start off with a vertex.
      if (helper->status[src] == 0) {
        helper->low[src] = helper->discovery[src] = ++time;
        helper->status[src] = 1;

        //Make the parent connection here and add the edge
        if (helper->discovery[src] != 1) {
          int edge_index = edge_map->at(merge(helper->parent[src], src));
          bicc_edges->push_back(edge_index);
          ////debug("Tree Edge:",helper->parent[src] + 1,src + 1);
        }
      } else if (helper->status[src] == 1) //traverse the adjacencies
          {
        for (int i = graph->c_graph->rowOffsets->at(src);
            i < graph->c_graph->rowOffsets->at(src + 1); i++) {
          if (graph->bicc_number[i] != component_number)
            continue;

          dest = graph->c_graph->cols->at(i);

          if (helper->status[dest] == 0) {
            helper->parent[dest] = src;
            helper->_stack.push(dest);

          } else if ((dest != helper->parent[src])
              && (helper->discovery[dest] < helper->discovery[src])) {

            helper->low[src] = min(helper->low[src], helper->discovery[dest]);
            bicc_edges->push_back(edge_map->at(merge(src, dest)));

            assert(edge_map->at(merge(src, dest)) < graph->Edges);
            ////debug("Back Edge:",src + 1,dest + 1);
          }
        }
        helper->status[src] = 2;
      } else if (helper->status[src] == 2) {
        helper->status[src] = 3;

        //Here source is the destination. Parent[src] is the actual source.
        if (helper->discovery[src] != 1)
          helper->low[helper->parent[src]] = min(helper->low[src], helper->low[helper->parent[src]]);

        int _edge_src = helper->parent[src];
        int _edge_dest = src;

        //first part is to check if the _edge_src is the root node.
        //second part is to check for a non-root node if low[dest] >= discovery[src]
        if (((helper->discovery[_edge_src] == 1)
            && (time - helper->discovery[_edge_src] >= 2))
            || ((helper->discovery[_edge_src] > 1)
                && (helper->low[_edge_dest] >= helper->discovery[_edge_src]))) {
          ////debug("Articulation Point Detected: src:",_edge_src + 1);
          graph->is_articulation_point[_edge_src] = true;

          if (bicc_edges->empty())
            return;

          list<int> *edges_per_component = new list<int>();
          int edge_index = bicc_edges->back();
          assert(edge_index < graph->Edges);
          int src_vtx = graph->c_graph->rows->at(edge_index);
          int dest_vtx = graph->c_graph->cols->at(edge_index);

          while ((src_vtx != _edge_src) || (dest_vtx != _edge_dest)) {
            edges_per_component->push_back(edge_index);
            bicc_edges->pop_back();

            ////debug("Removed Edge,src:",src_vtx+1,",dest:",dest_vtx+1);
            if (bicc_edges->empty())
              break;

            edge_index = bicc_edges->back();
            assert(edge_index < graph->Edges);
            src_vtx = graph->c_graph->rows->at(edge_index);
            dest_vtx = graph->c_graph->cols->at(edge_index);
          }

          if (!bicc_edges->empty()) {
            edge_index = bicc_edges->back();
            assert(edge_index < graph->Edges);
            src_vtx = graph->c_graph->rows->at(edge_index);
            dest_vtx = graph->c_graph->cols->at(edge_index);
            edges_per_component->push_back(edge_index);
            bicc_edges->pop_back();
            ////debug("Removed Edge,src:",src_vtx+1,",dest:",dest_vtx+1);
          }

          //belongs to new component if number of edges > 1. Else its a bridge.
          if (edges_per_component->size() > 1) {
            //Updated bcc_no for this bicc
            int bcc_no = ++(*new_component_number);
            store_biconnected_edges.push_back(make_pair(bcc_no, edges_per_component));

            ////debug("New component number :",bcc_no);
          } else if (edges_per_component->size() == 1) {
            count_bridges++;
            if (!keep_bridges)
              edges_per_component->clear();
            else {
              int bcc_no = ++(*new_component_number);
              store_biconnected_edges.push_back(make_pair(bcc_no, edges_per_component));
            }

            graph->is_articulation_point[src_vtx] = true;
            graph->is_articulation_point[dest_vtx] = true;
            ////debug("Identified Bridge");
          }
        }
        helper->_stack.pop();
      } else {
        helper->_stack.pop();
      }
    }
  }
};

/**
 * @brief This method internally calls the core dfs routine in the csr_graph necessary for obtaining the biconnected
 * components. The biconnected component number for each edges are marked by keeping track of the edges.
 *  
 * @param src source vertex
 * @param bicc_number This value indicates the component number in the graph which is to be processed.
 * @param edge_map This corresponds to the a hash_map containing indexes->edges.
 * @param bicc_pair This corresponds to a pair containing <new_bcc_number,start> vertex for every component.
 * @return count of new_bccs_formed.
 */
int dfs_bicc_initializer(int src, int bicc_number, int &new_bicc_number,
    bicc_graph *graph, dfs_helper *helper, unordered_map<uint64_t, int> *edge_map,
    unordered_map<int, list<int>*> &edge_list_component, bool keep_bridges) {
  int j = 1;
  list<int> *bicc_edges = new list<int>();
  helper->initialize_arrays();
  //////debug("New Component");
  //////debug("");

  //dfs(src,bicc_number,new_bicc_number,graph,bicc_edges,helper,count,bicc_pair,edge_map);
  DFS *dfs_worker = new DFS(bicc_number, &new_bicc_number, graph, bicc_edges,
      helper, edge_map, keep_bridges);
  dfs_worker->dfs(src);

  list<int> *edges_per_component = new list<int>();
  int src_vtx = -1;
  int dest_vtx = -1;

  while (!bicc_edges->empty()) {
    j = 1;
    int edge_index = bicc_edges->back();
    bicc_edges->pop_back();
    src_vtx = graph->c_graph->rows->at(edge_index);
    dest_vtx = graph->c_graph->cols->at(edge_index);

    ////debug("Removed Edge,src:",src_vtx+1,",dest:",dest_vtx+1);
    edges_per_component->push_back(edge_index);
  }

  //belongs to new component if number of edges > 1. Else its a bridge.
  if (edges_per_component->size() > 1) {
    //Updated bcc_no for this bicc
    int bcc_no = ++new_bicc_number;
    dfs_worker->store_biconnected_edges.push_back(make_pair(bcc_no, edges_per_component));
    ////debug("New component number :",bcc_no);
  } else if (edges_per_component->size() == 1) {
    dfs_worker->count_bridges++;
    if (!keep_bridges) {
      edges_per_component->clear();
    } else {
      int bcc_no = ++new_bicc_number;
      dfs_worker->store_biconnected_edges.push_back(make_pair(bcc_no, edges_per_component));
    }

    assert(src_vtx != -1);
    assert(dest_vtx != -1);

    dfs_worker->graph->is_articulation_point[src_vtx] = true;
    dfs_worker->graph->is_articulation_point[dest_vtx] = true;
    ////debug("Identified Bridge");
  }

  //apply the component labels
  for (auto&& it : dfs_worker->store_biconnected_edges) {
    int component_number = it.first;
    list<int> *edge_lists = it.second;

    int _src_component = -1;
    for (auto&& it : *edge_lists) {
      int edge_index = it;
      int src_vtx = dfs_worker->graph->c_graph->rows->at(edge_index);
      int dest_vtx = dfs_worker->graph->c_graph->cols->at(edge_index);
      _src_component = src_vtx;

      //APPLY TO EDGES in both directions. i.e. src_vtx => dest_vtx and dest_vtx => src_vtx
      dfs_worker->graph->bicc_number[edge_index] = component_number;
      dfs_worker->graph->bicc_number[dfs_worker->edge_map->at(merge(dest_vtx, src_vtx))] = component_number;
    }
    //debug("Inside dfs",component_number,edge_list_component.size());

#pragma omp critical
    {
      edge_list_component[component_number] = edge_lists;
    }
    assert(_src_component != -1);
  }
  dfs_worker->store_biconnected_edges.clear();
  return dfs_worker->count_bridges;
}
