#pragma once
#include <cuda_runtime.h>


struct GpuTimer {
  cudaEvent_t e0;
  cudaEvent_t e1;

  GpuTimer() {
    cudaEventCreate(&e0);
    cudaEventCreate(&e1);
  }

  ~GpuTimer() {
    cudaEventDestroy(e0);
    cudaEventDestroy(e1);
  }

  void start() {
    cudaEventRecord(e0, 0);
  }

  void stop() {
    cudaEventRecord(e1, 0);
  }

  float elapsed() {
    float t;
    cudaEventSynchronize(e1);
    cudaEventElapsedTime(&t, e0, e1);
    return t;
  }
};
