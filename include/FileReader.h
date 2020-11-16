#pragma once
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "utils.h"
#include "mmio.h"


class FileReader {
  FILE *file;
  int ret_code;
  int M, N, Edges;

public:
  FileReader(const char *name) {
    MM_typecode code;
    file = fopen(name, "r");
    if (file == NULL) {
      fprintf(stderr, "Unable to open file: %s\n", name);
      exit(1);
    }
    if (mm_read_banner(file, &code) != 0) {
      fprintf(stderr, "Could not process Matrix Market banner.\n");
      exit(1);
    }
    if (!(mm_is_matrix(code) && mm_is_coordinate(code)
        && (mm_is_integer(code) || mm_is_real(code))
        && (mm_is_symmetric(code) || mm_is_general(code)))) {
      fprintf(stderr, "Sorry, this application does not support this mtx file. \n");
      exit(1);
    }
    if ((ret_code = mm_read_mtx_crd_size(file, &M, &N, &Edges)) != 0) {
      fprintf(stderr, "Couldn't find all 3 parameters\n");
      exit(1);
    }
  }

  void get_nodes_edges(int &nodes, int &edges) {
    nodes = M;
    edges = Edges;
  }

  void read_edge(int &u, int &v, int &weight) {
    fscanf(file, "%d %d %d", &u, &v, &weight);
    u--;
    v--;
    // if(u >= v)
    // {
    //   ERROR("u >= v\n");
    //   exit(1);
    // }
  }

  void fileClose() {
    fclose(file);
  }
};
