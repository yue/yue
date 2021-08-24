// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef LUA_YUE_BINDING_SIGNAL_H_
#define LUA_YUE_BINDING_SIGNAL_H_

#include <string>
#include <utility>

#include "lua/lua.h"
#include "nativeui/signal.h"

namespace yue {

// A simple structure that records the signal pointer and owner reference.
template<typename Sig>
class SignalWrapper : public base::RefCounted<SignalWrapper<Sig>> {
 public:
  SignalWrapper(lua::State* state, int owner, nu::Signal<Sig>* signal)
      : ref_(state, owner), signal_(signal) {
  }

  int Connect(lua::CallContext* context) {
    if (!PushOwner(context))
      return -1;
    // Must not reference signal handler in C++.
    std::function<Sig> slot;
    if (!lua::ToWeakFunction(context->state, 2, &slot)) {
      context->has_error = true;
      lua::PushFormatedString(
          context->state, "error converting arg at index %d from %s to %s",
          2, lua::GetTypeName(context->state, 2), "function");
      return -1;
    }
    int id = signal_->Connect(std::move(slot));
    // self.__yuesignals[signal][id] = slot
    lua::PushRefsTable(context->state, "__yuesignals", -1);
    lua::RawGetOrCreateTable(context->state, -1, static_cast<void*>(signal_));
    lua::RawSet(context->state, -1, id, lua::ValueOnStack(context->state, 2));
    return id;
  }

  void Disconnect(lua::CallContext* context, int id) {
    if (!PushOwner(context))
      return;
    signal_->Disconnect(id);
    // self.__yuesignals[signal][id] = nil
    lua::PushRefsTable(context->state, "__yuesignals", -1);
    lua::RawGetOrCreateTable(context->state, -1, static_cast<void*>(signal_));
    lua::RawSet(context->state, -1, id, nullptr);
  }

  void DisconnectAll(lua::CallContext* context) {
    if (!PushOwner(context))
      return;
    signal_->DisconnectAll();
    // self.__yuesignals[signal] = {}
    lua::PushRefsTable(context->state, "__yuesignals", -1);
    lua::RawSet(context->state, -1, static_cast<void*>(signal_), nullptr);
  }

 private:
  ~SignalWrapper() {}

  bool PushOwner(lua::CallContext* context) {
    ref_.Push();
    if (lua::GetType(context->state, -1) == lua::LuaType::Nil) {
      context->has_error = true;
      lua::Push(context->state, "Owner of signal is gone");
      return false;
    }
    return true;
  }

  friend class base::RefCounted<SignalWrapper<Sig>>;

  lua::Weak ref_;
  nu::Signal<Sig>* signal_;
};

}  // namespace yue

namespace lua {

template<typename Sig>
struct Type<yue::SignalWrapper<Sig>> {
  static constexpr const char* name = "Signal";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "connect", &yue::SignalWrapper<Sig>::Connect,
           "disconnect", &yue::SignalWrapper<Sig>::Disconnect,
           "disconnectall", &yue::SignalWrapper<Sig>::DisconnectAll);
  }
};

template<typename Sig>
struct Type<nu::Signal<Sig>> {
  static constexpr const char* name = "Signal";
};

// Define how the Signal member is converted.
template<typename Sig>
struct MemberTraits<nu::Signal<Sig>> {
  static const RefMode kRefMode = RefMode::FirstGet;
  static inline void Push(State* state, int owner,
                          const nu::Signal<Sig>& signal) {
    lua::Push(state,
              new yue::SignalWrapper<Sig>(
                  state, owner, const_cast<nu::Signal<Sig>*>(&signal)));
  }

  static inline bool To(State* state, int owner, int value,
                        nu::Signal<Sig>* out) {
    if (lua::GetType(state, value) != lua::LuaType::Function)
      return false;
    // Must not reference signal handler in C++.
    std::function<Sig> slot;
    if (!lua::ToWeakFunction(state, value, &slot))
      return false;
    int id = out->Connect(std::move(slot));
    // self.__yuesignals[signal][id] = slot
    lua::PushRefsTable(state, "__yuesignals", owner);
    lua::RawGetOrCreateTable(state, -1, static_cast<void*>(out));
    lua::RawSet(state, -1, id, lua::ValueOnStack(state, value));
    return true;
  }
};

}  // namespace lua

#endif  // LUA_YUE_BINDING_SIGNAL_H_
