/*
 * This file is part of APRIL-ANN toolkit (A
 * Pattern Recognizer In Lua with Artificial Neural Networks).
 *
 * Copyright 2013, Francisco Zamora-Martinez
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
#include "unused_variable.h"
#include "error_print.h"
#include "table_of_token_codes.h"
#include "token_vector.h"
#include "token_matrix.h"
#include "copy_component.h"

using namespace AprilMath;
using namespace AprilMath::MatrixExt::BLAS;
using namespace AprilUtils;
using namespace Basics;

namespace ANN {
  
  CopyANNComponent::CopyANNComponent(unsigned int times, const char *name,
				     unsigned int input_size,
				     unsigned int output_size) :
    ANNComponent(name, 0, input_size, output_size),
    input(0),
    error_output(0),
    output(0),
    error_input(0),
    times(times) {
    if (times < 2)
      ERROR_EXIT1(128, "CopyANNComponent for less than 2 copies is forbidden [%s]\n",
		  this->name.c_str());
  }
  
  CopyANNComponent::~CopyANNComponent() {
    if (input) DecRef(input);
    if (error_input) DecRef(error_input);
    if (output) DecRef(output);
    if (error_output) DecRef(error_output);
  }
  
  Token *CopyANNComponent::doForward(Token* _input, bool during_training) {
    UNUSED_VARIABLE(during_training);
    AssignRef(input, _input);
    switch(input->getTokenCode()) {
    case table_of_token_codes::token_matrix:
      {
	AssignRef(output, new TokenBunchVector(times));
	for (unsigned int i=0; i<times; ++i) {
	  // FIXME: clone or not to clone? that is the question :(
	  (*output)[i] = input;
	  IncRef(input);
	}
      }
      break;
    case table_of_token_codes::vector_Tokens:
      {
	TokenBunchVector *input_bunch = input->convertTo<TokenBunchVector*>();
	AssignRef(output, new TokenBunchVector(input_bunch->size()));
	for (unsigned int i=0; i<input_bunch->size(); ++i) {
	  TokenBunchVector *current = new TokenBunchVector(times);
	  (*output)[i] = current;
	  IncRef(current);
	  for (unsigned int j=0; j<times; ++j) {
	    (*current)[j] = (*input_bunch)[j];
	    // FIXME: clone or not to clone? that is the question :(
	    IncRef((*current)[j]);
	  }
	}
      }
      break;
    default:
      ERROR_EXIT2(128, "Incorrect input token type %d [%s]\n",
		  input->getTokenCode(), name.c_str());
    }
    return output;
  }

  Token *CopyANNComponent::doBackprop(Token *_error_input) {
    if (_error_input == 0) {
      if (error_input)  { DecRef(error_input);  error_input  = 0; }
      if (error_output) { DecRef(error_output); error_output = 0; }
      return 0;
    }
    if (_error_input->getTokenCode() != table_of_token_codes::vector_Tokens)
      ERROR_EXIT1(128, "Incorrect error input token type, "
		  "expected TokenBunchVector [%s]\n",
		  name.c_str());
    AssignRef(error_input, _error_input->convertTo<TokenBunchVector*>());
    if (error_input->size() != times)
      ERROR_EXIT3(128, "Incorrect error input size, found %d, expected %d [%s]\n",
		  error_input->size(), times,
		  name.c_str());
    
    // the first is only copied
    Token *current = (*error_input)[0];
    if (current->getTokenCode() != table_of_token_codes::token_matrix)
      ERROR_EXIT1(128, "Incorrect token type, expected token matrix [%s]\n",
		  name.c_str());
    TokenMatrixFloat *current_token;
    current_token = current->convertTo<TokenMatrixFloat*>();
    MatrixFloat *current_mat = current_token->getMatrix();
#ifdef USE_CUDA
    current_mat->setUseCuda(use_cuda);
#endif
    ASSERT_MATRIX(current_mat);
    unsigned int bunch_size = current_mat->getDimSize(0);
    
    // output token
    MatrixFloat *error_output_mat;
    switch(input->getTokenCode()) {
    case table_of_token_codes::token_matrix:
      {
        TokenMatrixFloat *aux = input->convertTo<TokenMatrixFloat*>();
        april_assert(aux != 0);
        MatrixFloat *input_mat = aux->getMatrix();
        if (input_mat->getDimSize(0) != static_cast<int>(bunch_size)) {
          ERROR_EXIT(128, "Different bunch size between forward and backprop\n");
        }
        error_output_mat = new MatrixFloat(input_mat->getNumDim(),
                                           input_mat->getDimPtr());
        break;
      }
    case table_of_token_codes::vector_Tokens:
      {
        int dims[2] = { static_cast<int>(bunch_size),
                        static_cast<int>(input_size) };
        error_output_mat = new MatrixFloat(2, dims);
        break;
      }
    default:
      error_output_mat = 0;
      ERROR_EXIT2(128, "Incorrect input token type %d [%s]\n",
		  input->getTokenCode(), name.c_str());
    }
#ifdef USE_CUDA
    error_output_mat->setUseCuda(use_cuda);
#endif
    TokenMatrixFloat *error_output_token = new TokenMatrixFloat(error_output_mat);
    AssignRef<Token>(error_output, error_output_token);
    matCopy(error_output_mat, current_mat);
    
    // The rest of tokens
    for (unsigned int i=1; i<times; ++i) {
      Token *current = (*error_input)[i];
      if (current->getTokenCode() != table_of_token_codes::token_matrix)
	ERROR_EXIT1(128, "Incorrect token type, expected token matrix [%s]\n",
		    name.c_str());
      current_token = current->convertTo<TokenMatrixFloat*>();
      current_mat = current_token->getMatrix();
#ifdef USE_CUDA
      current_mat->setUseCuda(use_cuda);
#endif
      ASSERT_MATRIX(current_mat);
      matAxpy(error_output_mat, 1.0f, current_mat);
    }
    return error_output;
  }
  
  void CopyANNComponent::reset(unsigned int it) {
    UNUSED_VARIABLE(it);
    if (input) DecRef(input);
    if (error_input) DecRef(error_input);
    if (output) DecRef(output);
    if (error_output) DecRef(error_output);
    input	 = 0;
    error_input	 = 0;
    output	 = 0;
    error_output = 0;
  }

  ANNComponent *CopyANNComponent::clone() {
    CopyANNComponent *copy_component = new CopyANNComponent(times,
							    name.c_str(),
							    input_size,
							    output_size);
    return copy_component;
  }

  void CopyANNComponent::build(unsigned int _input_size,
			       unsigned int _output_size,
			       AprilUtils::LuaTable &weights_dict,
			       AprilUtils::LuaTable &components_dict) {
    ANNComponent::build(_input_size, _output_size,
			weights_dict, components_dict);
    if (output_size == 0) output_size = input_size * times;
    if (input_size  == 0) input_size  = output_size / times;
    if (input_size * times != output_size)
      ERROR_EXIT3(128, "Incorrect input/output sizes: input=%d output=%d [%s]\n",
		  input_size, output_size,
		  name.c_str());
  }
  
  char *CopyANNComponent::toLuaString() {
    buffer_list buffer;
    buffer.printf("ann.components.copy{ name='%s',times=%d,input=%d,output=%d }",
		  name.c_str(), times, input_size, output_size);
    return buffer.to_string(buffer_list::NULL_TERMINATED);
  }
}
