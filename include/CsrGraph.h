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
	vector<unsigned> *rows;
	vector<unsigned> *cols;
	vector<int> *weights;
	vector<unsigned> *degree;
	vector<unsigned> *rowOffsets;
	int initial_edge_count;
	int Nodes;

	CsrGraph() {
		rowOffsets = new vector<unsigned>();
		cols = new vector<unsigned>();
		rows = new vector<unsigned>();
		degree = new vector<unsigned>();
		weights = new vector<int>();
	}

	~CsrGraph() {
		rowOffsets->clear();
		cols->clear();
		rows->clear();
		degree->clear();
		weights->clear();
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

	void insert(int r, int c, int wt, bool dir=false) {
		rows->push_back(r);
		cols->push_back(c);
		weights->push_back(wt);
		if (!dir)	insert(c, r, wt, true);
	}

	vector<unsigned> *get_spanning_tree(vector<unsigned> **non_tree_edges,
			vector<unsigned> *ear_decomposition, int src);
	vector<unsigned> *mark_degree_two_chains(vector<vector<unsigned> > **chain, int &src);

	inline void getEdge(int i, unsigned &row, unsigned &col, int &weight) {
		assert(i < rows->size());
		row = rows->at(i);
		col = cols->at(i);
		weight = weights->at(i);
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
					new Edge(rows->at(i), cols->at(i), weights->at(i)));
		//Sort the elements first by row, then by column
		std::sort(combined.begin(), combined.end(), compare());
		//copy back the elements into row and columns
		for (int i = 0; i < rows->size(); i++) {
			rows->at(i) = combined[i]->row;
			cols->at(i) = combined[i]->col;
			weights->at(i) = combined[i]->weight;
			assert(rows->at(i) != cols->at(i));
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
	void writeToFile(string &name, int verts) {
		int N = rows->size();
		if (degree->size() == 0) return;
		FileWriter file(name.c_str(), verts, N/2);
		for (int i=0; i<N; i++) {
			int r = rows->at(i);
			int c = cols->at(i);
			int w = weights->at(i);
			if (r>c) file.write_edge(r, c, w);
		}
		file.close();
	}

	unsigned pathWeight(vector<unsigned> &edges, unsigned &row, unsigned &col) {
		int a = 0;
		int N = edges.size();
		col = cols->at(edges.at(0));
		row = rows->at(edges.at(edges.size() - 1));
		for (int i=0; i<N; i++)
			a += weights->at(edges.at(i));
		return a;
	}

	void print() {
		int N = rows->size();
		printf("=================================================================================\n");
		printf("Number of nodes = %d,edges = %d\n", Nodes, N/2);
		for (int i=0; i<N; i++) {
			int r = rows->at(i);
			int c = cols->at(i);
			int w = weights->at(i);
			if (r<c) printf("%d %d - %d\n", r+1, c+1, w);
		}
		printf("=================================================================================\n");
	}
};
