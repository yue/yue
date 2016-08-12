// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef LUA_WRAPPABLE_INTERNAL_H_
#define LUA_WRAPPABLE_INTERNAL_H_

#include "lua/state.h"

namespace lua {

namespace internal {

// Base class of Wrappable for handling object lifetime.
class WrappableBase {
 protected:
  WrappableBase();
  virtual ~WrappableBase();

 private:
  DISALLOW_COPY_AND_ASSIGN(WrappableBase);
};

}  // namespace internal

}  // namespace lua

#endif  // LUA_WRAPPABLE_INTERNAL_H_
