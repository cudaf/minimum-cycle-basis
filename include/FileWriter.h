#pragma once
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "utils.h"
#include "mmio.h"


class FileWriter {
  FILE *OutputFileName;
  int ret_code;
  int M, N, Edges;

  inline void ERROR(const char *ch) {
    std::cerr << RED << ch << " " << RESET;
  }

public:
  FileWriter(const char *OutputFile, int Nodes, int NZ) {
    MM_typecode code;
    M = Nodes;
    Edges = NZ;

    if ((OutputFileName = fopen(OutputFile, "w")) == NULL) {
      ERROR("Unable to open file.\n");
      printf("filename = %s\n", OutputFile);
      exit(1);
    }

    mm_initialize_typecode(&code);
    mm_set_matrix(&code);
    mm_set_coordinate(&code);
    mm_set_integer(&code);
    mm_set_symmetric(&code);
    mm_write_banner(OutputFileName, code);
    mm_write_mtx_crd_size(OutputFileName, Nodes, Nodes, Edges);
  }

  void write_edge(int u, int v, int weight) {
    fprintf(OutputFileName, "%d %d %d\n", u + 1, v + 1, weight);
  }

  FILE *get_file() {
    return OutputFileName;
  }

  void fileClose() {
    fclose(OutputFileName);
  }
};
