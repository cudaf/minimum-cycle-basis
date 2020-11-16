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
    int err;
    MM_typecode code;

    file = fopen(name, "r");
    ASSERTMSG(file, "Unable to open file: %s\n", name);

    err = mm_read_banner(file, &code);
    ASSERTMSG(!err, "Could not process Matrix Market banner.\n");
    ASSERTMSG(
      mm_is_matrix(code) &&
      mm_is_coordinate(code) &&
      (mm_is_integer(code) || mm_is_real(code)) ||
      (mm_is_symmetric(code) || mm_is_general(code)),
      "This .mtx is not supported.\n");
    err = mm_read_mtx_crd_size(file, &rows, &cols, &vals);
    ASSERTMSG(err, "Could not find all 3 parameters.\n");
  }


  void get_nodes_edges(int &verts, int &edges) {
    verts = rows;
    edges = vals;
  }


  void read_edge(int &u, int &v, int &wt) {
    fscanf(file, "%d %d %d", &u, &v, &wt);
    u--;
    v--;
  }


  void close() {
    fclose(file);
  }
};
