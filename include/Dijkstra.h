#pragma once
#include <queue>
#include <vector>

using std::pair;
using std::vector;
using std::priority_queue;
using std::make_pair;


struct EdgeSorter {
  int edge_offsets;
  int level;

  EdgeSorter(int e, int l) :
      edge_offsets(e), level(l) {
  }

  struct compare {
    bool operator()(const EdgeSorter &a, const EdgeSorter &b) {
      return (a.level < b.level);
    }
  };
};

struct Dijkstra {
  int Nodes;
  vector<int> distance;
  vector<bool> in_tree;
  vector<int> edge_offsets;
  vector<int> level;
  vector<int> parent;
  vector<int> *tree_edges;
  CsrGraphMulti *graph;
  int *fvs_array;

  struct Compare {
    bool operator()(pair<int, int> &a, pair<int, int> &b) {
      return (a.second > b.second);
    }
  };
  priority_queue<pair<int, int>, vector<pair<int, int>>, Compare> pq;

  Dijkstra(int nodes, CsrGraphMulti *input_graph, int *fvs_array) {
    Nodes = nodes;
    graph = input_graph;
    distance.resize(nodes);
    in_tree.resize(nodes);
    parent.resize(nodes);
    level.resize(nodes);
    edge_offsets.resize(nodes);
    this->fvs_array = fvs_array;

    for (int i = 0; i < nodes; i++)
      distance[i] = -1;
  }
  ~Dijkstra() {
    distance.clear();
    in_tree.clear();
    parent.clear();
    level.clear();
    edge_offsets.clear();
  }

  void reset() {
    for (int i = 0; i < Nodes; i++) {
      distance[i] = -1;
      in_tree[i] = false;
      parent[i] = -1;
      level[i] = -1;
      edge_offsets[i] = -1;
    }
    tree_edges = NULL;
  }

  void dijkstra_sp(int src) {
    printf("dijkstra_sp: %d (+1)\n", src+1);
    tree_edges = new vector<int>();
    distance[src] = 0;
    level[src] = 0;
    parent[src] = -1;
    edge_offsets[src] = -1;
    pq.push(make_pair(src, 0));

    while (!pq.empty()) {
      auto [u, d] = pq.top();
      pq.pop();
      if (in_tree[u]) continue;
      if (u != src) tree_edges->push_back(edge_offsets[u]);
      in_tree[u] = true;

      for (int i = graph->rowOffsets->at(u); i < graph->rowOffsets->at(u+1); i++) {
        int v = graph->cols->at(i);
        if (in_tree[v]) continue;
        int wt = graph->weights->at(i);
        printf("u: %d v: %d wt: %d (+1)\n", u+1, v+1, wt);
        if (distance[v] >= 0 && distance[v] < distance[u] + wt) continue;
        distance[v] = distance[u] + wt;
        pq.push(make_pair(v, distance[v]));
        parent[v] = u;
        edge_offsets[v] = i;
        level[v] = level[u] + 1;
      }
    }
#ifndef NDEBUG
    assert_correctness(src);
#endif
  }

  void compute_non_tree_edges(vector<int> **non_tree_edges) {
    int M = graph->rows->size();
    vector<uint8_t> is_tree_edge(M);
    for (int i = 0; i < tree_edges->size(); i++)
      is_tree_edge[tree_edges->at(i)] = 1;

    for (int i=0; i<M; i++) {
      if (is_tree_edge[i] == 1) continue;
      else if (is_tree_edge[graph->reverse_edge->at(i)] == 1) is_tree_edge[i] = 1;
      else if (is_tree_edge[graph->reverse_edge->at(i)] == 2) is_tree_edge[i] = 2;
      else {
        is_tree_edge[i] = 2;
        (*non_tree_edges)->push_back(i);
      }
    }
  }

  void fill_tree_edges(int *csr_rows, int *csr_cols, int *csr_nodes_index,
      int *csr_edge_offset, int *csr_parent, int *csr_distance, int src) {
    vector<EdgeSorter> edges;
    edges.push_back(EdgeSorter(-1, 0));

    for (int i = 0; i < tree_edges->size(); i++) {
      int offset = tree_edges->at(i);
      int row = graph->rows->at(offset);
      int col = graph->cols->at(offset);
      edges.push_back(EdgeSorter(offset, level[col]));
    }

    sort(edges.begin(), edges.end(), EdgeSorter::compare());
    assert(edges.size() == Nodes);

    //edges array
    for (int i = 0; i < edges.size(); i++) {
      csr_rows[edges[i].level]++;
      if (edges[i].edge_offsets == -1) {
        csr_nodes_index[src] = i;
        csr_cols[i] = -1;
        csr_edge_offset[i] = -1;
        csr_parent[src] = -1;
        csr_distance[src] = 0;
      } else {
        int row = graph->rows->at(edges[i].edge_offsets);
        int col = graph->cols->at(edges[i].edge_offsets);
        csr_nodes_index[col] = i;
        csr_cols[i] = csr_nodes_index[row];
        assert(csr_cols[i] >= 0 && csr_cols[i] < i);
        csr_edge_offset[i] = edges[i].edge_offsets;
        csr_parent[col] = edges[i].edge_offsets;
        csr_distance[col] = distance[col];
      }
    }

    int prev = 0, temp;
    for (int i = 0; i <= Nodes; i++) {
      temp = csr_rows[i];
      csr_rows[i] = prev;
      prev += temp;
    }
  }

  bool is_edge_cycle(int edge_offset, int &total_weight, int src) {
    int row, col, orig_row, orig_col;
    total_weight = 0;
    orig_row = row = graph->rows->at(edge_offset);
    orig_col = col = graph->cols->at(edge_offset);

    if ((fvs_array[row] >= 0) && (src > row)) return false;
    if ((fvs_array[col] >= 0) && (src > col)) return false;

    while (level[row] != level[col]) {
      if (level[row] < level[col]) col = parent[col];
      else row = parent[row];
      if ((fvs_array[row] >= 0) && (src > row)) return false;
      if ((fvs_array[col] >= 0) && (src > col)) return false;
    }

    if ((fvs_array[row] >= 0) && (src > row)) return false;
    if ((fvs_array[col] >= 0) && (src > col)) return false;

    while (row != col) {
      row = parent[row];
      col = parent[col];
      if ((fvs_array[row] >= 0) && (src > row)) return false;
      if ((fvs_array[col] >= 0) && (src > col)) return false;
    }

    if (row == src) {
      total_weight += distance[orig_row] + distance[orig_col] + graph->weights->at(edge_offset);
    }
    return (row == src);
  }
  void assert_correctness(int src) {
    for (int i = 0; i < graph->Nodes; i++) {
      if (i == src) {
        assert(distance[i] == 0);
        assert(parent[i] == -1);
        assert(level[i] == 0);
        assert(in_tree[i] == true);
      } else {
        assert(distance[i] > 0);
        assert(parent[i] >= 0);
        assert(level[i] > 0);
        assert(in_tree[i] == true);
      }
    }
  }
};
