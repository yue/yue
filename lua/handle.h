// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.
//
// Puts the value of index to registry and holds a reference to it.

#ifndef LUA_HANDLE_H_
#define LUA_HANDLE_H_

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
#if LUA_VERSION_NUM >= 503
  IsRunning  = LUA_GCISRUNNING,
#endif
};
inline int CollectGarbage(State* state, GCOp op = GCOp::Collect, int arg = 0) {
  return lua_gc(state, static_cast<int>(op), arg);
}

// Comman handle class.
class Handle {
 public:
  explicit Handle(State* state) : state_(state) {}
  virtual ~Handle() {}

  Handle& operator=(const Handle&) = delete;
  Handle(const Handle&) = delete;

  // Puts the value back to stack.
  virtual void Push() const = 0;

  State* state() const { return state_; }

 private:
  State* state_;
};

// The strong reference to a value.
class Persistent : public Handle {
 public:
  Persistent(State* state, int index);
  ~Persistent() override;

  void Push() const override;

 private:
  int ref_;
};

// The weak reference.
class Weak : public Handle {
 public:
  Weak(State* state, int index);
  ~Weak() override;

  void Push() const override;
};

// It is impossible in lua to get a always unique key for weak references, so
// it is caller's reponsibility to provide an unique key. Usually should just
// pick the address of the native object that stores the reference.
void CreateWeakReference(State* state, void* key, int val);
void PushWeakReference(State* state, void* key);
bool WeakReferenceExists(State* state, void* key);

}  // namespace lua

#endif  // LUA_HANDLE_H_
