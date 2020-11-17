#include "gpu_struct.cuh"
#include "common.cuh"


template<typename T>
__device__  __forceinline__
 const T* get_pointer_const(const T *data, int node, int nodes, int chunk_size, int stream) {
  return (data + (stream * chunk_size * nodes) + (node * nodes));
}

template<typename T>
__device__  __forceinline__
T* get_pointer(T* data, int node, int nodes, int chunk_size, int stream) {
  return (data + (stream * chunk_size * nodes) + (node * nodes));
}

__device__ __forceinline__
unsigned getBit(unsigned long long val, int pos) {
  unsigned long long ret;
  asm("bfe.u64 %0, %1, %2, 1;" : "=l"(ret) : "l"(val), "r"(pos));
  return (unsigned) ret;
}

__global__
void __kernel_init_edge(const int* __restrict__ d_non_tree_edges,
    const int* __restrict__ d_edge_offsets, int *d_precompute_array,
    const int* __restrict__ d_fvs_vertices,
    const uint64_t *d_si_vector, int start, int end,
    int stream, int chunk_size, int original_nodes, int size_vector,
    int fvs_size, int num_non_tree_edges, int num_edges) {
  int si_index = -1;

  uint64_t si_value;
  int src_index = blockIdx.x + start;

  if (src_index >= end)
    return;

  int *d_row = get_pointer(d_precompute_array, src_index - start,
      original_nodes, chunk_size, stream);
  const int* __restrict__ d_edge = get_pointer_const(d_edge_offsets,
      src_index - start, original_nodes, chunk_size, stream);

  for (int edge_index = threadIdx.x; edge_index < original_nodes;
      edge_index += blockDim.x) {
    int edge_offset = __ldg(&d_edge[edge_index]);
    //tree edges
    if (edge_offset >= 0) {
      int non_tree_edge_loc = __ldg(&d_non_tree_edges[edge_offset]);

      //non_tree_edge
      if (non_tree_edge_loc >= 0) {
        int p_idx = non_tree_edge_loc >> 6;
        if (si_index != p_idx) {
          si_index = p_idx;
          si_value = __ldg(&d_si_vector[si_index]);
        }
        d_row[edge_index] = getBit(si_value, non_tree_edge_loc & 63);
      } else
        //tree edge
        d_row[edge_index] = 0;
    } else {
      d_row[edge_index] = 0;
    }
  }
}

/**
 * @brief
 * This method is used to invoke a kernel whose function is defined in the details section.
 * @details
 * This method invokes a Kernel. The Kernel's task is to parallely do the following things
 * in the order.
 * a) For each source vertex between start and end (15 at a time(grid dimension)). We fill
 *    the precompute_array edges
 * b)The precompute array is filled in the following way.
 *   If the edge is a tree edge in the original spanning tree. then its value is 0.
 *   else if Si contains 1 in the corresponding non-tree edge position then 1 else 0.
 *
 * @param start index of vertex from 0 - fvs_size - 2
 * @param end index of vertex from 1 to fvs_size - 1
 * @param stream 0 or 1
 */
void gpu_struct::Kernel_init_edges_helper(int start, int end, int stream) {
  int total_length = end - start;

  __kernel_init_edge<<<total_length, 512, 0, streams[stream]>>>(
      d_non_tree_edges, d_edge_offsets, d_precompute_array,
      d_fvs_vertices, d_si_vector, start, end, stream, chunk_size,
      original_nodes, size_vector, fvs_size, num_non_tree_edges, num_edges);
}
