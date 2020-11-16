#pragma once
#include <omp.h>

class HostTimer {
  double elapsedTime;

public:
  double start_timer();
  double stop_timer();
  double get_event_time() {
    double initial_time = elapsedTime;
    elapsedTime = stop_timer();
    return (elapsedTime - initial_time);
  }
};
