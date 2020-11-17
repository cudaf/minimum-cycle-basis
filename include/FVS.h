#pragma once
#include <list>
#include "CsrGraphMulti.h"

using std::list;


struct FVS {
  CsrGraphMulti *input_graph;
  double *W;
  int Nodes;
  bool *node_status;  //Whether this node is present in the graph.
  bool *edge_status; //whether this edge is not associated with any deleted edges
  bool *is_vtx_in_fvs; //is a vertex part of FVS.
  list<int> FVS_SET; //list of vertices in the FVS.

  FVS(CsrGraphMulti *graph);

  ~FVS() {
    delete[] W;
    delete[] node_status;
    delete[] edge_status;
    delete[] is_vtx_in_fvs;
    FVS_SET.clear();
  }

  void pruning(int node_id);
  void MGA();
  bool test_fvs();
  bool contains_cycle(int node_id, bool *visited, int *parent);
  int *get_copy_fvs_array();
  void print_fvs();
  int get_num_elements();
};
