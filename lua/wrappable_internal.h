// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef LUA_WRAPPABLE_INTERNAL_H_
#define LUA_WRAPPABLE_INTERNAL_H_

#include "base/memory/ref_counted.h"

namespace lua {

namespace internal {

// Call destructors on garbage collection.
template<typename T>
int OnGC(State* state) {
  auto* self = static_cast<T*>(lua_touserdata(state, 1));
  self->~T();
  return 0;
}

// Base of PointerWrapper that hides implementations.
class PointerWrapperBase {
 public:
  // Convert the class to Lua and push it on stack.
  static void Push(State* state, void* ptr);

 protected:
  PointerWrapperBase(State* state, void* ptr);
  virtual ~PointerWrapperBase();

 private:
  DISALLOW_COPY_AND_ASSIGN(PointerWrapperBase);
};

// The specialized Wrappable class for storing refcounted class.
template<typename T>
class PointerWrapper : public PointerWrapperBase {
 public:
  PointerWrapper(State* state, T* t) : PointerWrapperBase(state, t), ptr_(t) {}

  T* get() const { return ptr_.get(); }

 private:
  scoped_refptr<T> ptr_;
};

}  // namespace internal

}  // namespace lua

#endif  // LUA_WRAPPABLE_INTERNAL_H_
