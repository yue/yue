// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "lua/wrappable_internal.h"

namespace lua {

namespace internal {

WrappableBase::WrappableBase() {
}

WrappableBase::~WrappableBase() {
}

// static
int WrappableBase::OnGC(State* state) {
  auto* self = static_cast<WrappableBase*>(lua_touserdata(state, 1));
  self->~WrappableBase();
  return 0;
}

}  // namespace internal

}  // namespace lua
