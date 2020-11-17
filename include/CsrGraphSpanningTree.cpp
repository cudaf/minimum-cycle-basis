#include <stack>
#include "CsrGraph.h"

using std::vector;


/**
 * @brief
 * This method is used to obtain the spanning tree of a graph. The spanning tree
 * contains edge offsets from the csr_graph.
 * @details
 * This method may also return the list of non_tree_edges and ear decomposition
 * corresponding to every non-tree edges.
 * @param  address of an vector for storing non_tree_edges, ear decomposition vector
 * @return vector of edge_offsets in bfs ordering.
 */
vector<int> *CsrGraph::get_spanning_tree(vector<int> **non_tree_edges,
    vector<int> *ear_decomposition, int src) {
  struct DFS_HELPER {
    int Nodes;
    vector<int> *spanning_tree;
    vector<bool> *visited;
    vector<int> *ear_decomposition_internal;
    vector<int> *rows_internal;
    vector<int> *columns_internal;
    vector<int> *rowOffsets_internal;
    vector<int> **non_tree_edges_internal;
    vector<int> *stack;
    vector<int> *parent;
    int ear_count;

    DFS_HELPER(vector<int> **non_tree_edges, vector<int> *rows, vector<int> *columns,
        vector<int> *rowOffsets, vector<int> *ear_decomposition, int _nodes) {
      spanning_tree = new vector<int>();
      visited = new vector<bool>();
      stack = new vector<int>();
      parent = new vector<int>();
      Nodes = _nodes;
      ear_count = 0;

      for (int i = 0; i < Nodes; i++) {
        parent->push_back(-1);
        visited->push_back(false);
      }

      non_tree_edges_internal = non_tree_edges;
      rows_internal = rows;
      columns_internal = columns;
      rowOffsets_internal = rowOffsets;
      ear_decomposition_internal = ear_decomposition;
      assert((ear_decomposition_internal->size() == Nodes + 1));
    }

    void dfs(int row) {
      visited->at(row) = true;
      stack->push_back(row);

      for (int offset = rowOffsets_internal->at(row);
          offset < rowOffsets_internal->at(row + 1); offset++) {
        int column = columns_internal->at(offset);
        if (!visited->at(column)) {
          visited->at(column) = true;
          spanning_tree->push_back(offset);
          parent->at(column) = row;
          dfs(column);
        } else {
          bool ear_incremented = false;
          if (column == parent->at(row)) continue;
          (*non_tree_edges_internal)->push_back(offset);

          if (ear_decomposition_internal != NULL) {
            for (auto it = stack->rbegin(); it != stack->rend(); it++) {
              if (ear_decomposition_internal->at(*it) == 0) {
                ear_decomposition_internal->at(*it) = ear_count + 1;
                ear_incremented = true;
              }
              else break;
            }
          }
          if (ear_incremented)
            ear_count++;
        }
      }

      stack->pop_back();
    }

    vector<int> *run_dfs(int row) {
      dfs(row);
      assert(spanning_tree->size() == Nodes - 1);
      if (ear_decomposition_internal != NULL)
        ear_decomposition_internal->at(Nodes) = ear_count;
      return spanning_tree;
    }

    ~DFS_HELPER() {
      visited->clear();
      stack->clear();
      parent->clear();
    }

  };

  DFS_HELPER helper(non_tree_edges, rows, cols, rowOffsets, ear_decomposition, Nodes);
  vector<int> *spanning_tree = helper.run_dfs(src);
  return spanning_tree;
}
