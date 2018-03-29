// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef LUA_YUE_BINDING_SIGNAL_H_
#define LUA_YUE_BINDING_SIGNAL_H_

#include <string>

#include "lua/lua.h"
#include "nativeui/signal.h"

namespace yue {

// A simple structure that records the signal pointer and owner reference.
template<typename Sig>
class SignalWrapper {
 public:
  SignalWrapper(lua::State* state, int owner, nu::Signal<Sig>* signal)
      : signal_(signal) {
    lua::CreateWeakReference(state, this, owner);
  }

  bool IsOwnerAlive(lua::State* state) {
    return lua::WeakReferenceExists(state, this);
  }

  int Connect(lua::CallContext* context, const std::function<Sig>& slot) {
    if (!IsOwnerAlive(context->state)) {
      context->has_error = true;
      lua::Push(context->state, "Owner of signal is gone");
      return -1;
    }
    int id = signal_->Connect(slot);
    lua::PushWeakReference(context->state, this);
    lua::PushRefsTable(context->state, "__yuesignals", -1);
    lua::RawSet(context->state, -1, id, lua::ValueOnStack(context->state, 2));
    return id;
  }

  void Disconnect(lua::CallContext* context, int id) {
    if (!IsOwnerAlive(context->state)) {
      context->has_error = true;
      lua::Push(context->state, "Owner of signal is gone");
      return;
    }
    signal_->Disconnect(id);
    lua::PushWeakReference(context->state, this);
    lua::PushRefsTable(context->state, "__yuesignals", -1);
    lua::RawSet(context->state, -1, id, nullptr);
  }

  void DisconnectAll(lua::CallContext* context) {
    if (!IsOwnerAlive(context->state)) {
      context->has_error = true;
      lua::Push(context->state, "Owner of signal is gone");
      return;
    }
    signal_->DisconnectAll();
    lua::PushWeakReference(context->state, this);
    lua::PushCustomDataTable(context->state, -1);
    lua::RawSet(context->state, -1, "__yuesignals", nullptr);
  }

 private:
  nu::Signal<Sig>* signal_;
};

}  // namespace yue

namespace lua {

template<typename Sig>
struct Type<nu::Signal<Sig>> {
  static constexpr const char* name = "yue.Signal";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "connect", &yue::SignalWrapper<Sig>::Connect,
           "disconnect", &yue::SignalWrapper<Sig>::Disconnect,
           "disconnectall", &yue::SignalWrapper<Sig>::DisconnectAll);
  }
  static bool To(State* state, int value, nu::Signal<Sig>* out) {
    if (lua::GetType(state, value) != lua::LuaType::Function)
      return false;
    std::function<Sig> callback;
    if (!lua::To(state, value, &callback))
      return false;
    int id = out->Connect(callback);
    lua::PushRefsTable(state, "__yuesignals", 1);
    lua::RawSet(state, -1, id, lua::ValueOnStack(state, 3));
    return true;
  }
};

template<typename Sig>
struct UserData<nu::Signal<Sig>> {
  using Type = yue::SignalWrapper<Sig>;
  static inline void Construct(State* state,
                               yue::SignalWrapper<Sig>* data,
                               nu::Signal<Sig>* ptr) {
    new(data) yue::SignalWrapper<Sig>(state, 1, ptr);
  }
  static inline void Destruct(yue::SignalWrapper<Sig>* data) {
    data->~Type();
  }
};

template<typename Sig>
struct Type<yue::SignalWrapper<Sig>*> {
  static constexpr const char* name = "yue.Signal";
  static bool To(State* state, int index, yue::SignalWrapper<Sig>** out) {
    index = AbsIndex(state, index);
    StackAutoReset reset(state);
    // Verify the type and length.
    if (GetType(state, index) != lua::LuaType::UserData ||
        RawLen(state, index) != sizeof(yue::SignalWrapper<Sig>))
      return false;
    // Verify the inheritance chain.
    if (!GetMetaTable(state, index) ||
        !IsMetaTableInheritedFrom<nu::Signal<Sig>>(state))
      return false;
    // Convert pointer to actual class.
    *out = static_cast<yue::SignalWrapper<Sig>*>(lua_touserdata(state, index));
    return true;
  }
};

// Define how the Signal member is converted.
template<typename Sig>
struct MemberTraits<nu::Signal<Sig>> {
  static const bool kShouldReferenceValue = false;
  static inline void Push(State* state, int cache,
                          const nu::Signal<Sig>& signal) {
    RawGet(state, cache, ValueOnStack(state, 2));
    if (GetType(state, -1) != LuaType::Nil)
      return;  // cache match.
    else
      PopAndIgnore(state, 1);
    NewUserData(state, const_cast<nu::Signal<Sig>*>(&signal));
    lua::Push(state, MetaTable<nu::Signal<Sig>>());
    SetMetaTable(state, -2);
    RawSet(state, cache, ValueOnStack(state, 2), ValueOnStack(state, -1));
  }
};

}  // namespace lua

#endif  // LUA_YUE_BINDING_SIGNAL_H_
