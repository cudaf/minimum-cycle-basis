#include "CsrGraphMulti.h"

using std::vector;


/**
 * @brief
 * This method is used to obtain the spanning tree of a graph. The spanning tree contains
 * edge offsets from the csr_graph.
 * @details
 * This method may also return the list of non_tree_edges and ear decomposition
 * corresponding to every non-tree edges.
 * @param  address of an vector for storing non_tree_edges,ear decomposition vector;
 * @return vector of edge_offsets in bfs ordering.
 */
vector<unsigned> *CsrGraphMulti::get_spanning_tree(
    vector<unsigned> **non_tree_edges, int src) {
  struct DFS_HELPER {
    int Nodes;
    vector<unsigned> *spanning_tree;
    vector<bool> *visited;
    vector<unsigned char> *is_tree_edge;
    vector<unsigned> *rows_internal;
    vector<unsigned> *columns_internal;
    vector<unsigned> *rowOffsets_internal;
    vector<unsigned> **non_tree_edges_internal;
    vector<unsigned> *reverse_edge_internal;
    vector<int> *parent;

    DFS_HELPER(vector<unsigned> **non_tree_edges,
        vector<unsigned> *rows, vector<unsigned> *columns,
        vector<unsigned> *rowOffsets,
        vector<unsigned> *reverse_edge, int _nodes) {
      spanning_tree = new vector<unsigned>();
      visited = new vector<bool>();
      parent = new vector<int>();
      is_tree_edge = new vector<unsigned char>();
      Nodes = _nodes;

      for (int i = 0; i < Nodes; i++) {
        parent->push_back(-1);
        visited->push_back(false);
      }

      for (int i = 0; i < rows->size(); i++)
        is_tree_edge->push_back(0);

      non_tree_edges_internal = non_tree_edges;
      rows_internal = rows;
      columns_internal = columns;
      rowOffsets_internal = rowOffsets;
      reverse_edge_internal = reverse_edge;
    }

    void dfs(int row) {
      visited->at(row) = true;

      for (int offset = rowOffsets_internal->at(row);
          offset < rowOffsets_internal->at(row + 1); offset++) {
        int column = columns_internal->at(offset);
        if (!visited->at(column)) {
          visited->at(column) = true;
          spanning_tree->push_back(offset);
          parent->at(column) = row;
          is_tree_edge->at(offset) = 1;
          dfs(column);
        } else {
          if (column == parent->at(row)) {
            int reverse_index = reverse_edge_internal->at(offset);
            if (is_tree_edge->at(reverse_index) == 1) {
              is_tree_edge->at(offset) = 1;
              continue;
            } else if (is_tree_edge->at(reverse_index) == 2) {
              is_tree_edge->at(offset) = 2;
              continue;
            }

            else {
              is_tree_edge->at(offset) = 2;
              is_tree_edge->at(reverse_index) = 2;
              if (row < column)
                (*non_tree_edges_internal)->push_back(offset);
              else
                (*non_tree_edges_internal)->push_back(reverse_index);
              continue;
            }
          } else if (is_tree_edge->at(offset) == 0) {
            int reverse_index = reverse_edge_internal->at(offset);
            is_tree_edge->at(offset) = 2;
            is_tree_edge->at(reverse_index) = 2;

            if (row < column)
              (*non_tree_edges_internal)->push_back(offset);
            else
              (*non_tree_edges_internal)->push_back(reverse_index);
            continue;
          } else
            continue;
        }
      }

    }

    vector<unsigned> *run_dfs(int row) {
      dfs(row);
      assert(spanning_tree->size() == Nodes - 1);
      return spanning_tree;
    }

    ~DFS_HELPER() {
      visited->clear();
      parent->clear();
      is_tree_edge->clear();
    }
  };

  DFS_HELPER helper(non_tree_edges, rows, cols, rowOffsets, reverse_edge, Nodes);
  vector<unsigned> *spanning_tree = helper.run_dfs(src);
  return spanning_tree;
}
