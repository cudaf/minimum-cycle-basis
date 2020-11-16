#pragma once
#include <omp.h>

struct HostTimer {
  double t0, t1;

  double start();
  double stop();
  double elapsed() {
    if (t1 < t0) stop();
    return t1 - t0;
  }
};
