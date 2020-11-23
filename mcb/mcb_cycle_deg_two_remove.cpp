#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <utility>
#include <set>
#include <omp.h>
#include <string>
#include <algorithm>
#include <atomic>
#include <list>
#include <cstring>
#include <unordered_set>
#include <unordered_map>
#include <utility>

#include "FileReader.h"
#include "Files.h"
#include "utils.h"
#include "HostTimer.h"
#include "CsrGraph.h"
#include "CsrTree.h"
#include "CsrGraphMulti.h"
#include "BitVector.h"
#include "WorkerThread.h"
#include "CycleStorage.h"
#include "Stats.h"
#include "FVS.h"
#include "CompressedTrees.h"
#include "GpuTask.h"
#include <gpu/common.cuh>

using std::string;
using std::list;
using std::vector;
using std::fill;

Debugger dbg;
HostTimer timer;

string InputFileName;
string OutputFileDirectory;

Stats info(true);

int num_threads;

int main(int argc, char* argv[]) {
  if (argc < 4) {
    printf("1st Argument should indicate the InputFile\n");
    printf("2nd Argument should indicate the OutputFile\n");
    printf("3th argument should indicate the number of threads.(Optional) (1 default)\n");
    printf("4th Argument is chunk_size.(Optional) (720 default)");
    exit(1);
  }

  num_threads = 1;
  if (argc >= 4)
    num_threads = atoi(argv[3]);
  omp_set_num_threads(num_threads);

  // read the Inputfile.
  InputFileName = argv[1];
  string InputFilePath = InputFileName;
  FileReader Reader(InputFilePath.c_str());

  int v1, v2, Initial_Vertices, weight;
  int nodes, edges, chunk_size = 720, nstreams = 1;
  if(argc >= 5)
    chunk_size = atoi(argv[4]);

  Reader.get_nodes_edges(nodes, edges);
  debug("InputFileName:", InputFileName);
  debug("chunk_size:", chunk_size);
  debug("nodes:", nodes);
  debug("edges:", edges);

  CsrGraph *graph = new CsrGraph();
  graph->Nodes = nodes;
  graph->initial_edge_count = edges;

  // fill edges
  for (int i = 0; i < edges; i++) {
    Reader.read_edge(v1, v2, weight);
    graph->insert(v1, v2, weight, false);
  }
  graph->calculateDegreeandRowOffset();
  info.setNumNodesTotal(graph->Nodes);
  info.setEdges(graph->rows->size());
  Reader.close();

  // already a cycle?
  if (graph->verticesOfDegree(2) == graph->Nodes) {
    info.setCycleNumFVS(1);
    info.setNumNodesRemoved(graph->Nodes - 1);
    info.setNumFinalCycles(1);
    info.setNumInitialCycles(1);
    info.setTotalWeight(graph->totalWeight());
    info.print_stats(argv[2]);
    return 0;
  }

  init_cuda();
  int source_vertex;
  vector<vector<int>> *chains = new vector<vector<int>>();
  vector<int> *remove_edge_list = graph->mark_degree_two_chains(&chains, source_vertex);
  vector<vector<int>> *edges_new_list = new vector<vector<int>>();

  int nodes_removed = 0;
  for (int i = 0; i < chains->size(); i++) {
    int row, col;
    int total_weight = graph->pathWeight(chains->at(i), row, col);
    nodes_removed += chains->at(i).size() - 1;

    vector<int> new_edge = vector<int>();
    new_edge.push_back(row);
    new_edge.push_back(col);
    new_edge.push_back(total_weight);
    edges_new_list->push_back(new_edge);
  }

  //Record the number of nodes removed in the graph.
  assert(nodes_removed == graph->verticesOfDegree(2));
  info.setNumNodesRemoved(nodes_removed);
  debug("Graph");
  graph->print();

  debug("REMOVE: dont reduce graph");
  debug("remove_edge_list:", remove_edge_list->size());
  debug("edges_new_list:", edges_new_list->size());
  debug("nodes_removed:", nodes_removed);
  remove_edge_list->clear();
  edges_new_list->clear();
  nodes_removed = 0;

  CsrGraphMulti *reduced_graph = CsrGraphMulti::get_modified_graph(graph,
      remove_edge_list, edges_new_list, nodes_removed);
  debug("Reduced graph");
  reduced_graph->print();

  FVS fvs_helper(reduced_graph);
  fvs_helper.MGA();
  fvs_helper.print_fvs();

  //Record the number of new edges in the graph.
  info.setNewEdges(reduced_graph->rows->size());
  //Record the number of FVS vertices in the graph.
  info.setCycleNumFVS(fvs_helper.get_num_elements());

  int *fvs_array = fvs_helper.get_copy_fvs_array();
  CsrTree *initial_spanning_tree = new CsrTree(reduced_graph);
  initial_spanning_tree->populate_tree_edges(true, source_vertex);
  int num_non_tree_edges = initial_spanning_tree->non_tree_edges->size();
  assert(num_non_tree_edges == edges - nodes + 1);
  assert(graph->totalWeight() == reduced_graph->totalWeight());

  vector<int> non_tree_edges_map(reduced_graph->rows->size());
  fill(non_tree_edges_map.begin(), non_tree_edges_map.end(), -1);
  for (int i = 0; i < initial_spanning_tree->non_tree_edges->size(); i++)
    non_tree_edges_map[initial_spanning_tree->non_tree_edges->at(i)] = i;

  // copy the edges into the reverse edges as well.
  for (int i = 0; i < reduced_graph->rows->size(); i++) {
    if (non_tree_edges_map[i] < 0)
      if (non_tree_edges_map[reduced_graph->reverse_edge->at(i)] >= 0)
        non_tree_edges_map[i] = non_tree_edges_map[reduced_graph->reverse_edge->at(i)];
  }

  chunk_size = 720;
  nstreams = CEILDIV(fvs_helper.get_num_elements(), chunk_size);
  int max_chunk_size = calculate_chunk_size(reduced_graph->Nodes, non_tree_edges_map.size(),
      CEILDIV(num_non_tree_edges, 64), nstreams);
  bool multiple_transfers = true;

  if(chunk_size <= max_chunk_size) {
    multiple_transfers = false;
    debug("Multiple transfers are turned off and the entire graph is copied first.");
    debug("max_chunk_size:", max_chunk_size);
    debug("chunk_size:", chunk_size);
    debug("nstreams:", nstreams);
    debug("");
  }

  info.setLoadEntireMemory(!multiple_transfers);
  info.setNchunks(nstreams);
  info.setNstreams(nstreams);

  // construct the initial
  debug("Construct the initial ...");
  CompressedTrees trees(chunk_size, fvs_helper.get_num_elements(), fvs_array,
      reduced_graph, allocate_pinned_memory, free_pinned_memory);
  CycleStorage *storage = new CycleStorage(reduced_graph->Nodes);
  WorkerThread **multi_work = new WorkerThread*[num_threads];
  for (int i = 0; i < num_threads; i++)
    multi_work[i] = new WorkerThread(reduced_graph, storage, fvs_array, &trees);

  debug("Produce shortest path trees across all the nodes.");
  timer.start();
  int count_cycles = 0;

debug("trees.fvs_size:", trees.fvs_size);
#pragma omp parallel for reduction(+:count_cycles)
  for (int i = 0; i < trees.fvs_size; ++i) {
    int threadId = omp_get_thread_num();
    count_cycles += multi_work[threadId]->produce_sp_tree_and_cycles_warp(i, reduced_graph);
  }
  info.setTimeConstructionTrees(timer.elapsed());

  for (int i=0; i<trees.fvs_size; i++)  
    for (auto&& d : multi_work[i]->helper->distance)
      printf("%d : %d\n", i, d);

  debug("Collection of cycles ...");
  timer.start();
  vector<Cycle*> list_cycle_vec;
  list<Cycle*> list_cycle;

  for (int j = 0; j < storage->list_cycles.size(); j++) {
    for (auto&& it : storage->list_cycles[j]) {
      for (int k = 0; k < it.second->listed_cycles.size(); k++) {
        list_cycle_vec.push_back(it.second->listed_cycles[k]);
        list_cycle_vec.back()->ID = list_cycle_vec.size() - 1;
      }
    }
  }
  sort(list_cycle_vec.begin(), list_cycle_vec.end(), Cycle::compare());
  info.setNumInitialCycles(list_cycle_vec.size());
  for (int i = 0; i < list_cycle_vec.size(); i++) {
    if (list_cycle_vec[i] != NULL)
      list_cycle.push_back(list_cycle_vec[i]);
  }
  list_cycle_vec.clear();
  info.setTimeCollectCycles(timer.elapsed());
  debug("At this stage we have shortest path trees and the cycles sorted in increasing order of length.");

  // generate the bit vectors
  BitVector **support_vectors = new BitVector*[num_non_tree_edges];
  for (int i = 0; i < num_non_tree_edges; i++) {
    support_vectors[i] = new BitVector(num_non_tree_edges);
    support_vectors[i]->set(i, true);
  }

  GpuTask gpu_compute(&trees, (int*) trees.final_vertices,
      non_tree_edges_map, support_vectors, num_non_tree_edges);
  gpu_struct device_struct(non_tree_edges_map.size(), num_non_tree_edges,
      support_vectors[0]->capacity, gpu_compute.original_nodes,
      gpu_compute.fvs_size, chunk_size, nstreams, &info);
  configure_grid(0, gpu_compute.fvs_size);

  vector<Cycle*> final_mcb;
  double precompute_time = 0;
  double cycle_inspection_time = 0;
  double hybrid_time = 0;

  if(!multiple_transfers)
    device_struct.initialize_memory(&gpu_compute);

  BitVector *cycle_vector = new BitVector(num_non_tree_edges,
      allocate_pinned_memory, free_pinned_memory);
  BitVector *current_vector = new BitVector(num_non_tree_edges,
      allocate_pinned_memory, free_pinned_memory);
  BitVector *next_vector = new BitVector(num_non_tree_edges,
      allocate_pinned_memory, free_pinned_memory);
  BitVector *temp_bitvec_ptr;

  current_vector->init();
  current_vector->set(0, true);
  precompute_time += device_struct.copy_support_vector(current_vector);
  precompute_time += device_struct.process_shortest_path(&gpu_compute,multiple_transfers);

  //Main Outer Loop of the Algorithm.
  info.print_stats(argv[2]);
  debug("Main Outer Loop of the Algorithm.");
  for (int e = 0; e < num_non_tree_edges; e++) {
    timer.start();
    int *node_rowoffsets, *node_columns, *precompute_nodes, *nodes_index;
    int *node_edgeoffsets, *node_parents, *node_distance;
    int src, edge_offset, reverse_edge, row, col, position, bit;
    int src_index;

    for (auto cycle = list_cycle.begin(); cycle != list_cycle.end(); cycle++) {
      src = (*cycle)->get_root();
      src_index = trees.vertices_map[src];

      trees.get_node_arrays_warp(&node_rowoffsets, &node_columns,
          &node_edgeoffsets, &node_parents, &node_distance,
          &nodes_index, src_index);
      trees.get_precompute_array(&precompute_nodes, src_index);
      edge_offset = (*cycle)->non_tree_edge_index;
      bit = 0;

      int row = reduced_graph->rows->at(edge_offset);
      int col = reduced_graph->cols->at(edge_offset);

      if (non_tree_edges_map[edge_offset] >= 0) {
        bit = current_vector->get(non_tree_edges_map[edge_offset]);
      }

      bit = (bit ^ precompute_nodes[nodes_index[row]]);
      bit = (bit ^ precompute_nodes[nodes_index[col]]);

      if (bit == 1) {
        final_mcb.push_back(*cycle);
        list_cycle.erase(cycle);
        break;
      }
    }

    final_mcb.back()->get_cycle_vector(non_tree_edges_map,
        initial_spanning_tree->non_tree_edges->size(), cycle_vector);

    cycle_inspection_time += timer.elapsed();
    if((e + 1) >= num_non_tree_edges) break;
    timer.start();


  #pragma omp parallel
  {
    #pragma omp master
    {
      int product = cycle_vector->dot_product(support_vectors[e + 1]);
      if (product == 1)
        support_vectors[e + 1]->do_xor(current_vector);

      next_vector->copy_from(support_vectors[e + 1]);
      precompute_time += device_struct.copy_support_vector(next_vector);
      precompute_time += device_struct.process_shortest_path(&gpu_compute,multiple_transfers);
    }
    #pragma omp for
    for (int j = e + 2; j < num_non_tree_edges; j++) {
      int product = cycle_vector->dot_product(support_vectors[j]);
      if (product == 1)
        support_vectors[j]->do_xor(current_vector);
    }
  }
    // exchange the support vector pointers.
    temp_bitvec_ptr = current_vector;
    current_vector = next_vector;
    next_vector = temp_bitvec_ptr;
    hybrid_time += timer.elapsed();
  }

  debug("Clear vector data.");
  cycle_vector->free();
  next_vector->free();
  current_vector->free();
  list_cycle.clear();

  debug("Set GPU timings ...");
  info.setGpuTimings(precompute_time / 1000);
  info.setCycleInspectionTime(cycle_inspection_time);
  info.setIndependenceTestTime(hybrid_time);
  info.setTotalTime();

  int total_weight = 0;
  for (int i = 0; i < final_mcb.size(); i++) {
    total_weight += final_mcb[i]->total_length;
  }

  debug("Set final num cycles ...");
  info.setNumFinalCycles(final_mcb.size());
  info.setTotalWeight(total_weight);

  debug("Print stats.");
  info.print_stats(argv[2]);

  delete[] fvs_array;
  debug("Clear all data.");
  device_struct.clear_memory();
  trees.clear_memory();

  for (int i = 0; i < num_non_tree_edges; i++)
    support_vectors[i]->free();

  delete[] support_vectors;
  return 0;
}
