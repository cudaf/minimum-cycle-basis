#pragma once
#include <utility>
#include <algorithm>
#include <vector>
#include <string>
#include "FileWriter.h"

using std::string;
using std::vector;


class CsrGraph {
protected:
	struct Edge {
		unsigned row;
		unsigned col;
		int weight;

		Edge(unsigned &r, unsigned &c, int &w) {
			row = r;
			col = c;
			weight = w;
		}
	};

	struct compare {
		bool operator()(const Edge *a, const Edge *b) const {
			if (a->row == b->row)
				return (a->col < b->col);
			else
				return (a->row < b->row);
		}
	};

public:
	int Nodes;
	int initial_edge_count;
	vector<unsigned> *rowOffsets;
	vector<unsigned> *columns;
	vector<unsigned> *rows;
	vector<unsigned> *degree;
	vector<int> *weights;

	CsrGraph() {
		rowOffsets = new vector<unsigned>();
		columns = new vector<unsigned>();
		rows = new vector<unsigned>();
		degree = new vector<unsigned>();
		weights = new vector<int>();
	}

	int verticesOfDegree(int d) {
		int a = 0;
		int N = degree->size();
		for (int i=0; i<N; i++)
			if (degree->at(i) == d) a++;
		return a;
	}

	int totalWeight() {
		int a = 0;
		int N = rows->size();
		for (int i=0; i<N; i++)
			a += weights->at(i);
		return a/2;
	}

	void insert(int a, int b, int wt, bool direction) {
		columns->push_back(b);
		rows->push_back(a);
		weights->push_back(wt);
		if (!direction)
			insert(b, a, wt, true);
	}

	void insert(int a, int b, bool direction) {
		columns->push_back(b);
		rows->push_back(a);
		if (!direction)
			insert(b, a, true);
	}

	~CsrGraph() {
		rowOffsets->clear();
		columns->clear();
		rows->clear();
		degree->clear();
		weights->clear();
	}

	vector<unsigned> *get_spanning_tree(
			vector<unsigned> **non_tree_edges,
			vector<unsigned> *ear_decomposition, int src);

	vector<unsigned> *mark_degree_two_chains(vector<vector<unsigned> > **chain, int &src);

	inline void get_edge_endpoints(unsigned &row, unsigned &col, int &weight,	unsigned &index) {
		assert(index < rows->size());
		row = rows->at(index);
		col = columns->at(index);
		weight = weights->at(index);
	}

	//Calculate the degree of the vertices and create the rowOffset
	void calculateDegreeandRowOffset() {
		rowOffsets->resize(Nodes + 1);
		degree->resize(Nodes);
		for (int i = 0; i < Nodes; i++) {
			rowOffsets->at(i) = 0;
			degree->at(i) = 0;
		}
		rowOffsets->at(Nodes) = 0;
		//Allocate a pair array for rows and columns array
		vector<Edge*> combined;
		//copy the elements from the row and column array
		for (int i = 0; i < rows->size(); i++)
			combined.push_back(
					new Edge(rows->at(i), columns->at(i), weights->at(i)));
		//Sort the elements first by row, then by column
		std::sort(combined.begin(), combined.end(), compare());
		//copy back the elements into row and columns
		for (int i = 0; i < rows->size(); i++) {
			rows->at(i) = combined[i]->row;
			columns->at(i) = combined[i]->col;
			weights->at(i) = combined[i]->weight;
			assert(rows->at(i) != columns->at(i));
		}
		for (int i = 0; i < rows->size(); i++)
			delete combined[i];
		combined.clear();
		//Now calculate the row_offset
		for (int i = 0; i < rows->size(); i++) {
			unsigned curr_row = rows->at(i);
			rowOffsets->at(curr_row)++;}
		unsigned prev = 0, current;
		for (int i = 0; i <= Nodes; i++) {
			current = rowOffsets->at(i);
			rowOffsets->at(i) = prev;
			prev += current;
		}
		for (int i = 0; i < Nodes; i++) {
			degree->at(i) = rowOffsets->at(i + 1) - rowOffsets->at(i);
		}
		assert(rowOffsets->at(Nodes) == rows->size());
#ifdef INFO
		printf("row_offset size = %d,columns size = %d\n",rowOffsets->size(),columns->size());
#endif
	}

	//Print to a file.
	void PrintToFile(string &name, int verts) {
		if (degree->size() == 0) return;
		FileWriter file(name.c_str(), verts, rows->size()/2);
		for (int i = 0; i < rows->size(); i++) {
			if (rows->at(i) > columns->at(i))
				file.write_edge(rows->at(i), columns->at(i), weights->at(i));
		}
		file.close();
	}

	unsigned sum_edge_weights(vector<unsigned> &edges_list, unsigned &row, unsigned &col) {
		unsigned edge_weight = 0;
		for (int i = 0; i < edges_list.size(); i++)
			edge_weight += weights->at(edges_list.at(i));
		col = columns->at(edges_list.at(0));
		row = rows->at(edges_list.at(edges_list.size() - 1));
		return edge_weight;
	}

	void print() {
		printf("=================================================================================\n");
		printf("Number of nodes = %d,edges = %d\n", Nodes, rows->size() / 2);
		for (int i = 0; i < rows->size(); i++) {
			if (rows->at(i) < columns->at(i))
				printf("%u %u - %u\n", rows->at(i) + 1, columns->at(i) + 1, weights->at(i));
		}
		printf("=================================================================================\n");
	}
};
