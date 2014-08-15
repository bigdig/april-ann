/*
 * This file is part of APRIL-ANN toolkit (A
 * Pattern Recognizer In Lua with Artificial Neural Networks).
 *
 * Copyright 2014, Francisco Zamora-Martinez
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
#ifndef MATRIXINPUTSWITCHANNCOMPONENT_H
#define MATRIXINPUTSWITCHANNCOMPONENT_H

#include "ann_component.h"
#include "april_assert.h"
#include "cblas_headers.h"
#include "component_properties.h"
#include "sparse_matrixFloat.h"
#include "token_matrix.h"
#include "token_sparse_matrix.h"

namespace ANN {

  /**
   * An abstract component which defines basic interface for components which
   * receives sparse or dense matrix and produce dense matrix.
   */
  class MatrixInputSwitchANNComponent : public ANNComponent,
                                        public ComponentPropertiesAndAsserts {
    APRIL_DISALLOW_COPY_AND_ASSIGN(MatrixInputSwitchANNComponent);
    
    basics::TokenMatrixFloat *input, *output, *error_input, *error_output;
    basics::TokenSparseMatrixFloat *sparse_input, *sparse_error_output;
    bool is_sparse_input;
    
    // Auxiliary methods
    basics::Token *doDenseForward(basics::Token *_input, bool during_training);
    basics::Token *doSparseForward(basics::Token *_input, bool during_training);
    basics::Token *doDenseBackprop(basics::Token *_error_input);
    basics::Token *doSparseBackprop(basics::Token *_error_input);

  protected:

    // Auxiliary methods
    
    basics::MatrixFloat *getInputMatrix() {
      april_assert(!is_sparse_input);
      return input->getMatrix();
    }
    basics::SparseMatrixFloat *getSparseInputMatrix() {
      april_assert(is_sparse_input);
      return sparse_input->getMatrix();
    }
    basics::MatrixFloat *getOutputMatrix() {
      return output->getMatrix();
    }
    basics::MatrixFloat *getErrorInputMatrix() {
      return error_input->getMatrix();
    }
    basics::MatrixFloat *getErrorOutputMatrix() {
      april_assert(!is_sparse_input);
      return error_output->getMatrix();
    }
    basics::SparseMatrixFloat *getSparseErrorOutputMatrix() {
      april_assert(is_sparse_input);
      return sparse_error_output->getMatrix();
    }
    
    virtual void computeGradients(april_utils::SharedPtr<basics::MatrixFloat> & grads_mat);

    // Abstract methods

    virtual basics::MatrixFloat *privateDoDenseForward(basics::MatrixFloat *input, bool during_training) = 0;
    virtual basics::MatrixFloat *privateDoDenseBackprop(basics::MatrixFloat *input_error) = 0;
    virtual void privateDenseReset(unsigned int it=0) = 0;
    virtual void privateDenseComputeGradients(april_utils::SharedPtr<basics::MatrixFloat> & grads_mat) = 0;

    virtual basics::MatrixFloat *privateDoSparseForward(basics::SparseMatrixFloat *input, bool during_training) = 0;
    virtual basics::SparseMatrixFloat *privateDoSparseBackprop(basics::MatrixFloat *input_error) = 0;
    virtual void privateSparseReset(unsigned int it=0) = 0;
    virtual void privateSparseComputeGradients(april_utils::SharedPtr<basics::MatrixFloat> & grads_mat) = 0;

  public:
    MatrixInputSwitchANNComponent(const char *name, const char *weights_name,
                                  unsigned int input_size,
                                  unsigned int output_size);
    virtual ~MatrixInputSwitchANNComponent();
    virtual basics::Token *getInput() {
      if (!is_sparse_input) return input;
      else return sparse_input;
    }
    virtual basics::Token *getOutput() {
      return output;
    }
    virtual basics::Token *getErrorInput() {
      return error_input;
    }
    virtual basics::Token *getErrorOutput() {
      if (!is_sparse_input) return error_output;
      else return sparse_error_output;
    }
    
    virtual basics::Token *doForward(basics::Token* input, bool during_training);
    virtual basics::Token *doBackprop(basics::Token *input_error);
    virtual void   reset(unsigned int it=0);
    //
    
    // The following methods are not implemented, derived classes had to
    //
    // virtual ANNComponent *clone() = 0;
    /*
      virtual void build(unsigned int input_size,
      unsigned int output_size,
      MatrixFloatSet *weights_dict,
      hash<string,ANNComponent*> &components_dict) = 0;
    */
    // virtual void copyWeights(MatrixFloatSet *weights_dict) = 0;
    // virtual char *toLuaString() = 0;
    
  };
}

#endif // MATRIXINPUTSWITCHANNCOMPONENT_H