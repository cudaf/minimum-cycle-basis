#pragma once
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "utils.h"
#include "mmio.h"


class FileWriter {
  FILE *file;
  int M, N, Edges;

  inline void ERROR(const char *ch) {
    std::cerr << RED << ch << " " << RESET;
  }

public:
  FileWriter(const char *name, int Nodes, int NZ) {
    MM_typecode code;
    M = Nodes;
    Edges = NZ;

    if ((file = fopen(name, "w")) == NULL) {
      ERROR("Unable to open file.\n");
      printf("filename = %s\n", name);
      exit(1);
    }

    mm_initialize_typecode(&code);
    mm_set_matrix(&code);
    mm_set_coordinate(&code);
    mm_set_integer(&code);
    mm_set_symmetric(&code);
    mm_write_banner(file, code);
    mm_write_mtx_crd_size(file, Nodes, Nodes, Edges);
  }

  void write_edge(int u, int v, int weight) {
    fprintf(file, "%d %d %d\n", u + 1, v + 1, weight);
  }

  FILE *get_file() {
    return file;
  }

  void fileClose() {
    fclose(file);
  }
};
