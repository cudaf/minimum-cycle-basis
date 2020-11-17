#pragma once
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <assert.h>

#ifdef __NVCC__
#include <cuda_runtime.h>
#include <cuda.h>
#endif

using std::cerr;
using std::endl;


typedef unsigned* (*fn_alloc_t)(int, int);
typedef void  (*fn_free_t)(unsigned*);

//the following are UBUNTU/LINUX ONLY terminal color codes.
#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */


#ifndef CEILDIV
inline int ceildiv(int x, int y) {
    return (x + y - 1) / y;
}

// Computes rounded-up integer division.
// CEILDIV(6, 3) = 2
// CEILDIV(7, 3) = 3
#define CEILDIV(x, y) ceildiv(x, y)
#endif


#define ASSERTMSG(c, ...) \
  do { \
    if (c) break;  \
    fprintf(stderr, __VA_ARGS__);  \
    exit(1);  \
  } while (0)


#ifdef __NVCC__
#define CudaError(ans) { gpuAssert((ans), __FILE__, __LINE__); }
inline void gpuAssert(cudaError_t code, const char *file, int line, bool abort =
    true) {
  if (code != cudaSuccess) {
    cerr << RED << "Error :" << cudaGetErrorString(code) << " : "
    << file << " : line No = " << line << RESET << endl;
    // fprintf(stderr,"GPUassert: %s %s %d\n", cudaGetErrorString(code), file, line);
    if (abort)
    cudaDeviceReset();
    exit(code);
  }
}
#endif

struct Debugger {
  template<typename T> Debugger& operator ,(const T& v) {
    cerr << CYAN << v << " " << RESET;
    return *this;
  }
};

#define BLOCK_DEFAULT 1024
#define CEIL(SIZE) ((int)ceil(((double)SIZE)/BLOCK_DEFAULT))

extern Debugger dbg;

#ifdef VERBOSE
#define debug(args...)            {dbg,args; cerr<<endl;}
#else
#define debug(args...)            {}
#endif 
