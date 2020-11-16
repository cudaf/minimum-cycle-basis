#include "HostTimer.h"


double HostTimer::start() {
  t0 = omp_get_wtime();
  return t0;
}

double HostTimer::stop() {
  t1 = omp_get_wtime();
  return t1;
}
