// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "lua/wrappable.h"

namespace lua {

Wrappable::Wrappable(State* state) : handle_(state, -1) {
  DCHECK_EQ(GetType(state, -1), LuaType::UserData);
}

Wrappable::~Wrappable() {
}

void Wrappable::Push(State* state) const {
  handle_.Push(state);
  DCHECK_EQ(GetType(state, -1), LuaType::UserData);
}

// static
int Wrappable::OnGC(State* state) {
  auto* self = static_cast<Wrappable*>(lua_touserdata(state, 1));
  self->~Wrappable();
  return 0;
}

}  // namespace lua
