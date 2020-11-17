#include <stack>
#include "CsrGraph.h"

using std::vector;


/**
 * @brief
 * This method is used to mark the degree 2 chains of a graph for removal.
 * @details
 * This method uses the ear-decomposition of the nodes to obtain the end
 * points of a chain. The first node and the last node of the vectors are
 * the end points of the chain.
 * @param address of an vector for storing non_tree_edges, ear decomposition vector
 * @return vector of edge_offsets in bfs ordering.
 */
vector<unsigned> *CsrGraph::mark_degree_two_chains(
    vector<vector<unsigned> > **chain, int &src) {
  struct DFS_HELPER {
    int Nodes;

    vector<unsigned> *edge_removal_list;
    vector<bool> *visited;

    vector<vector<unsigned> > **chains_internal;

    vector<unsigned> *rows_internal;
    vector<unsigned> *columns_internal;
    vector<unsigned> *rowOffsets_internal;
    vector<unsigned> *degree_internal;

    vector<int> *parent;

    vector<unsigned> *temp_vector;

    DFS_HELPER(vector<unsigned> *rows, vector<unsigned> *columns,
        vector<unsigned> *rowOffsets,
        vector<unsigned> *degree,
        vector<vector<unsigned> > **chain, int _nodes) {
      edge_removal_list = new vector<unsigned>();
      visited = new vector<bool>();
      parent = new vector<int>();

      temp_vector = NULL;

      Nodes = _nodes;

      for (int i = 0; i < Nodes; i++) {
        parent->push_back(-1);
        visited->push_back(false);
      }

      rows_internal = rows;
      columns_internal = columns;
      rowOffsets_internal = rowOffsets;
      degree_internal = degree;
      chains_internal = chain;
    }

    void dfs(unsigned row) {
      visited->at(row) = true;
      for (unsigned offset = rowOffsets_internal->at(row);
          offset < rowOffsets_internal->at(row + 1); offset++) {
        unsigned column = columns_internal->at(offset);

        //printf("dfs visit = %d - %d \n",row + 1,column + 1);

        if (!visited->at(column)) {
          visited->at(column) = true;
          parent->at(column) = row;
          dfs(column);
        } else {
          if (column == parent->at(row)) {
            if ((degree_internal->at(column) == 2)
                || (degree_internal->at(row) == 2))
              edge_removal_list->push_back(offset);
            continue;
          }
          if (degree_internal->at(column) == 2) {
            edge_removal_list->push_back(offset);
            continue;
          }
        }
        //Remove Degree 2 Chains.
        if ((degree_internal->at(column) == 2)
            && (degree_internal->at(row) == 2)) {
          //debug(row+1,column+1,"both");
          temp_vector->push_back(offset);
          edge_removal_list->push_back(offset);
        } else if ((degree_internal->at(row) == 2)) {
          //debug(row+1,column+1,"row");
          temp_vector = new vector<unsigned>();
          temp_vector->push_back(offset);
          edge_removal_list->push_back(offset);
        } else if ((degree_internal->at(column) == 2)) {
          //debug(row+1,column+1,"column");
          temp_vector->push_back(offset);
          (*chains_internal)->push_back(*temp_vector);
          temp_vector = NULL;

          edge_removal_list->push_back(offset);
        } else {
          //debug(row+1,column+1,"None");
        }
      }
    }

    vector<unsigned> *run_dfs(int &src) {
      for (int i = 0; i < Nodes; i++) {
        if (degree_internal->at(i) > 2) {
          src = i;
          dfs(i);
          break;
        }
      }
      return edge_removal_list;
    }

    ~DFS_HELPER() {
      visited->clear();
      parent->clear();
    }

  };

  DFS_HELPER helper(rows, cols, rowOffsets, degree, chain, Nodes);

  vector<unsigned> *edge_removal_list = helper.run_dfs(src);

  return edge_removal_list;
}
