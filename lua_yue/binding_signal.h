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
      : owner_(lua::CreateWeakReference(state, owner)), signal_(signal) {}

  nu::Signal<Sig>* Get(lua::State* state) {
    if (lua::WeakReferenceExists(state, owner_))
      return signal_;
    else
      return nullptr;
  }

 private:
  int owner_;
  nu::Signal<Sig>* signal_;
};

}  // namespace yue

namespace lua {

template<typename Sig>
struct Type<nu::Signal<Sig>> {
  static constexpr const char* name = "yue.Signal";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "connect", &nu::Signal<Sig>::Connect,
           "disconnect", &nu::Signal<Sig>::Disconnect,
           "disconnectall", &nu::Signal<Sig>::DisconnectAll);
  }
  static bool To(State* state, int value, nu::Signal<Sig>* out) {
    if (lua::GetType(state, value) != lua::LuaType::Function)
      return false;
    base::Callback<Sig> callback;
    if (!lua::To(state, value, &callback))
      return false;
    out->Connect(callback);
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
  static inline nu::Signal<Sig>* From(State* state,
                                      yue::SignalWrapper<Sig>* data) {
    return data->Get(state);
  }
};

template<typename Sig>
struct Type<nu::Signal<Sig>*> {
  static constexpr const char* name = "yue.Signal";
  static bool To(State* state, int index, nu::Signal<Sig>** out) {
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
    nu::Signal<Sig>* ptr = UserData<nu::Signal<Sig>>::From(
        state,
        static_cast<yue::SignalWrapper<Sig>*>(lua_touserdata(state, index)));
    if (!ptr)
      return false;
    *out = ptr;
    return true;
  }
};

template<typename Sig>
struct Type<nu::SignalBase<Sig>*> {
  static constexpr const char* name = "yue.Signal";
  static bool To(State* state, int index, nu::SignalBase<Sig>** out) {
    return Type<nu::Signal<Sig>*>::To(
        state, index, reinterpret_cast<nu::Signal<Sig>**>(out));
  }
};

// Define how the Signal member is converted.
template<typename Sig>
struct MemberTraits<nu::Signal<Sig>> {
  static const bool kShouldCacheValue = false;
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
