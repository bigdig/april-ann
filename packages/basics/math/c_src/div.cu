/*
 * This file is part of the Neural Network modules of the APRIL toolkit (A
 * Pattern Recognizer In Lua).
 *
 * Copyright 2013, Francisco Zamora-Martinez
 *
 * The APRIL-MLP toolkit is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */
#include "wrapper.h"
#include "cuda_utils.h"
#include "unused_variable.h"

#ifdef USE_CUDA
/***************************************
 ************** CUDA SECTION ***********
 ***************************************/

template<typename T>
__global__ void divKernel(T *v, unsigned int N, unsigned int stride,
			  T value) {
  unsigned int x_idx = blockIdx.x * blockDim.x + threadIdx.x;
  if (x_idx < N) {
    unsigned int x_pos = x_idx*stride;
    v[x_pos] = value/v[x_pos];
  }
}
#endif

/***************************************
 *********** TEMPLATE SECTION **********
 ***************************************/

template <typename T>
void doDiv(unsigned int N,
	   GPUMirroredMemoryBlock<T> *v,
	   unsigned int stride,
	   unsigned int shift,
	   T value,
	   bool use_gpu) {
#ifndef USE_CUDA
  UNUSED_VARIABLE(use_gpu);
#endif
#ifdef USE_CUDA
  if (use_gpu) {
    T *v_ptr = v->getGPUForWrite() + shift;
    dim3 block, grid;
    computeBlockAndGridSizesForAnArray(N, block, grid);
    divKernel<<<grid, block, 0, GPUHelper::getCurrentStream()>>>
      (v_ptr, N, stride, value);
  }
  else {
#endif
    T *v_mem = v->getPPALForWrite() + shift;
    for (unsigned int i=0; i<N; ++i, v_mem += stride)
      *v_mem = value / (*v_mem);
#ifdef USE_CUDA
  }
#endif
}

template void doDiv<float>(unsigned int N,
			   GPUMirroredMemoryBlock<float> *v,
			   unsigned int stride,
			   unsigned int shift,
			   float value,
			   bool use_gpu);