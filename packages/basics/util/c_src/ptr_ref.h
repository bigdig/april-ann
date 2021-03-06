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
#ifndef PTR_REF_H
#define PTR_REF_H

#include "error_print.h"
#include "referenced.h"
#include "unused_variable.h"

namespace AprilUtils {

  /**
   * Default reference template, uses IncRef.
   */
  template<typename T>
  struct DefaultReferencer {
    DefaultReferencer() { }
    DefaultReferencer(const DefaultReferencer &) { }
    void operator()(T *ptr) const {
      if (ptr != 0) IncRef(ptr);
    }
    void checkUnique(T *ptr) const {
      if (ptr != 0 && ptr->getRef() != 1) {
        ERROR_EXIT(128, "checkUnique test failure\n");
      }
    }
  };
  
  /**
   * Default deleter template, uses DecRef.
   */
  template<typename T>
  struct DefaultDeleter {
    DefaultDeleter() { }
    DefaultDeleter(const DefaultDeleter &) { }
    void operator()(T *ptr) const {
      if (ptr != 0) DecRef(ptr);
    }
  };

  /**
   * Standard referencer template, does nothing.
   */
  template<typename T>
  struct StandardReferencer {
    StandardReferencer() { }
    StandardReferencer(const StandardReferencer &) { }
    void operator()(T *ptr) const {
      UNUSED_VARIABLE(ptr);
    }
    void checkUnique(T *ptr) const {
      UNUSED_VARIABLE(ptr);
    }
  };
  

  /**
   * Standard referencer template specialization for new[].
   */
  template<typename T>
  struct StandardReferencer<T[]> {
    StandardReferencer() { }
    StandardReferencer(const StandardReferencer &) { }
    void operator()(T *ptr) const {
      UNUSED_VARIABLE(ptr);
    }
    void checkUnique(T *ptr) const {
      UNUSED_VARIABLE(ptr);
    }
  };
  
  
  /**
   * Standard deleter template, uses delete.
   */
  template<typename T>
  struct StandardDeleter {
    StandardDeleter() { }
    StandardDeleter(const StandardDeleter &) { }
    void operator()(T *ptr) const {
      delete ptr;
    }
  };

  /**
   * Standard deleter template specialization for delete[].
   */
  template<typename T>
  struct StandardDeleter<T[]> {
    StandardDeleter() { }
    StandardDeleter(const StandardDeleter &) { }
    void operator()(T *ptr) const {
      delete[] ptr;
    }
  };
  
}

#endif // PTR_REF_H
