// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.
//
// Puts the value of index to registry and holds a reference to it.

#ifndef LUA_HANDLE_H_
#define LUA_HANDLE_H_

#include <memory>

#include "lua/stack.h"

namespace lua {

// Thin wrapper of lua_gc.
enum class GCOp {
  Stop       = LUA_GCSTOP,
  Restart    = LUA_GCRESTART,
  Collect    = LUA_GCCOLLECT,
  Count      = LUA_GCCOUNT,
  CountBytes = LUA_GCCOUNTB,
  Step       = LUA_GCSTEP,
  SetPause   = LUA_GCSETPAUSE,
  SetStepMul = LUA_GCSETSTEPMUL,
  IsRunning  = LUA_GCISRUNNING,
};
inline int CollectGarbage(State* state, GCOp op = GCOp::Collect, int arg = 0) {
  return lua_gc(state, static_cast<int>(op), arg);
}

// The strong reference to a value.
class Persistent {
 public:
  static std::unique_ptr<Persistent> New(State* state, int index) {
    lua::Push(state, ValueOnStack(state, index));
    return std::unique_ptr<Persistent>(new Persistent(state));
  }

  ~Persistent() {
    luaL_unref(state_, LUA_REGISTRYINDEX, ref_);
  }

  // Puts the value back to stack.
  void Push(State* state) const {
    DCHECK_EQ(state, state_) << "Pushing a handle on wrong thread";
    lua_rawgeti(state_, LUA_REGISTRYINDEX, ref_);
  }

 protected:
  explicit Persistent(State* state)
      : state_(state), ref_(luaL_ref(state, LUA_REGISTRYINDEX)) {}

 private:
  State* state_;
  int ref_;

  DISALLOW_COPY_AND_ASSIGN(Persistent);
};

// The weak reference.
int CreateWeakReference(State* state, int index);
void PushWeakReference(State* state, int ref);

}  // namespace lua

#endif  // LUA_HANDLE_H_
