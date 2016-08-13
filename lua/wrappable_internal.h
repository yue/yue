// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef LUA_WRAPPABLE_INTERNAL_H_
#define LUA_WRAPPABLE_INTERNAL_H_

#include "lua/handle.h"

namespace lua {

namespace internal {

// Base class of Wrappable for handling object lifetime.
class WrappableBase {
 public:
  // Convert the class to Lua and push it on stack.
  void Push(State* state) const;

 protected:
  explicit WrappableBase(State* state);
  virtual ~WrappableBase();

  static int OnGC(State* state);

 private:
  const Weak handle_;

  DISALLOW_COPY_AND_ASSIGN(WrappableBase);
};

}  // namespace internal

}  // namespace lua

#endif  // LUA_WRAPPABLE_INTERNAL_H_
