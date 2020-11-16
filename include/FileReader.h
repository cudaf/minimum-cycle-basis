#pragma once
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "utils.h"
#include "mmio.h"


struct FileReader {
  FILE *file;
  int rows;
  int cols;
  int vals;


  FileReader(const char *name) {
    MM_typecode code;

    file = fopen(name, "r");
    ASSERTMSG(file, "Unable to open file: %s\n", name);

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
    if ((ret_code = mm_read_mtx_crd_size(file, &rows, &cols, &vals)) != 0) {
      fprintf(stderr, "Couldn't find all 3 parameters\n");
      exit(1);
    }
  }

  void get_nodes_edges(int &verts, int &edges) {
    verts = rows;
    edges = vals;
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
