#pragma once
#include <vector>
#include "CompressedTrees.h"
#include "BitVector.h"


struct GpuTask {
  int fvs_size;
  int original_nodes;
  int num_non_tree_edges;
  int edge_size;
  int *fvs_array;
  int *non_tree_edges_array;
  CompressedTrees *host_tree;
  BitVector **support_vectors;

  GpuTask(CompressedTrees *ht, int *fvs,
      std::vector<int> &non_tree_edges_map, BitVector **s_vectors,
      int num_non_tree) {
    fvs_array = fvs;
    host_tree = ht;
    fvs_size = ht->fvs_size;
    original_nodes = ht->parent_graph->Nodes;
    num_non_tree_edges = num_non_tree;
    edge_size = non_tree_edges_map.size();
    non_tree_edges_array = non_tree_edges_map.data();
    support_vectors = s_vectors;
  }

  ~GpuTask() {
  }
};
