/*
 * This file is part of APRIL-ANN toolkit (A
 * Pattern Recognizer In Lua with Artificial Neural Networks).
 *
 * Copyright 2012, Salvador España-Boquera, Adrian Palacios Corella, Francisco
 * Zamora-Martinez
 *
 * The APRIL-ANN toolkit is free software; you can redistribute it and/or modify it
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
#ifndef _CUBLAS_HANDLER_H_
#define _CUBLAS_HANDLER_H_

#ifdef USE_CUDA
#include <cuda.h>
#include <cuda_runtime_api.h>
#include <cublas_v2.h>
#include <cusparse_v2.h>

#include "april_assert.h"
#include "cublas_error.h"
#include "cusparse_error.h"
#include "vector.h"

namespace AprilMath {
  namespace CUDA {
    
    /**
     * @brief A class to deal with CUDA initialization, streams and handlers.
     */
    class GPUHelper {
      static cublasHandle_t handler;
      static cusparseHandle_t sparse_handler;
      static bool initialized;
      static cudaDeviceProp properties;
      static CUdevice  device;
      static CUcontext context;
      static AprilUtils::vector<CUstream> streams;
      static unsigned int current_stream;
    public:
      
      /// If not initialized before, this method initializes CUDA.
      static void initHelper() {
        if (!initialized) {
          cublasStatus_t state;	
          state = cublasCreate(&handler);
          if (state != CUBLAS_STATUS_SUCCESS)
            ERROR_EXIT(150, "Cublas couldn't be initialized\n");
          cusparseStatus_t sparse_state = cusparseCreate(&sparse_handler);
          if (sparse_state != CUSPARSE_STATUS_SUCCESS)
            ERROR_EXIT(150, "Cusparse couldn't be initialized\n");
          //
          initialized = true;
          if (cuDeviceGet (&device, 0) != CUDA_SUCCESS)
            ERROR_EXIT(152, "Failed to get device\n");
          if (cudaGetDeviceProperties(&properties, 0) != cudaSuccess)
            ERROR_EXIT(153, "Failed to get properties\n");

          streams.push_back(0);
          current_stream = 0;
#ifndef NDEBUG
          fprintf(stderr,
                  "# Initialized CUDA, CUBLAS and CUSPARSE for GPU capabilitites "
                  "of version %d.%d\n", properties.major, properties.minor);
#endif
        }
      }
      
      /// Destroy a previously initialized CUDA environment.
      static void destroyHandler() {
        destroyStreams();
        if (initialized != true)
          ERROR_EXIT(151, "Destroying CUBLAS handler, that it's not initialized\n");
        // Need to free all the allocated memory in GPU
        cublasDestroy(handler);
        cusparseDestroy(sparse_handler);
        initialized = false;
      } 

      /// Returns the cublas handler.
      static cublasHandle_t &getHandler() {
        initHelper();
        return handler;
      }

      /// Returns the cusparse handler.
      static cusparseHandle_t &getSparseHandler() {
        initHelper();
        return sparse_handler;
      }
  
      /// Returns the CUDA device handler.
      static CUdevice &getCUdevice() {
        initHelper();
        return device;
      }
  
      /// Returns the maximum number of threads per block in the active device.
      static unsigned int getMaxThreadsPerBlock() {
        initHelper();
        return properties.maxThreadsPerBlock;
      }
  
      /// Push @c n streams into the collection GPUHelper::streams .
      static void createNStreams(unsigned int n) {
        initHelper();
        cuStreamSynchronize(0);
        for (unsigned int i=0; i<n; ++i) {
          CUstream stream;
          cuStreamCreate(&stream, 0);
          streams.push_back(stream);
        }
      }
  
      /// Destroy all the streams in the collection GPUHelper::streams .
      static void destroyStreams() {
        initHelper();
        for (unsigned int i=streams.size(); i>1; --i) {
          cuStreamSynchronize(streams[i-1]);
          cuStreamDestroy(streams[i-1]);
          streams.pop_back();
        }
        current_stream = 0;
        april_assert(streams.size() == 1);
      }
      
      /// Changes current stream number.
      static void setCurrentStream(unsigned int i) {
        april_assert(i < streams.size());
        current_stream = i;
      }
  
      /// Returns current stream handler.
      static CUstream getCurrentStream() {
        initHelper();
        return streams[current_stream];
      }
    };

  } // namespace CUDA
} // namespace AprilMath
#endif

#endif // _CUBLAS_HANDLER_H_
