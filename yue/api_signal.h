// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef YUE_API_SIGNAL_H_
#define YUE_API_SIGNAL_H_

#include "base/memory/ref_counted.h"
#include "lua/handle.h"
#include "nativeui/signal.h"

namespace yue {

// Get type from member pointer.
template<typename T> struct ExtractMemberPointer;
template<typename TType, typename TMember>
struct ExtractMemberPointer<TMember(TType::*)> {
  using Type = TType;
  using Member = TMember;
};

// The wrapper for nu::Signal.
class SignalBase {
 public:
  virtual void Connect(lua::CallContext* context) = 0;
};

template<typename T>
class Signal : SignalBase {
 public:
  using Type = typename ExtractMemberPointer<T>::Type;
  using Slot = typename ExtractMemberPointer<T>::Member::Slot;
  using Ref = typename ExtractMemberPointer<T>::Member::Ref;

  Signal(lua::State* state, int index, T member)
      : object_ref_(lua::CreateWeakReference(state, index)),
        member_(member) {}

  void Connect(lua::CallContext* context) override {
    Slot slot;
    if (!lua::To(context->state, -1, &slot)) {
      context->has_error = true;
      lua::Push(context->state, "first arg must be function");
      return;
    }

    lua::PushWeakReference(context->state, object_ref_);
    Type* object;
    if (!lua::To(context->state, -1, &object)) {
      context->has_error = true;
      lua::Push(context->state, "owner of signal is gone");
      return;
    }

    (object->*member_).Connect(slot);
  }

 private:
  int object_ref_;
  T member_;
};

// Push a weak table which records the object's signals.
void PushObjectSignalsTable(lua::State* state, int index) {
  int top = lua::GetTop(state);
  // Get the table for recording objects.
  if (luaL_newmetatable(state, "yue.SignalMap")) {
    lua::PushNewTable(state);
    lua::RawSet(state, -1, "__mode", "k");
    lua::SetMetaTable(state, -2);
  }
  lua::RawGet(state, -1, lua::ValueOnStack(state, index));
  if (lua::GetType(state, -1) != lua::LuaType::Table) {
    // This is the first record.
    lua::PushNewTable(state);
    lua::RawSet(state, -3, lua::ValueOnStack(state, index),
                           lua::ValueOnStack(state, -1));
  }
  // Pop the metatable and keep the signal table.
  lua::Insert(state, top + 1);
  lua::SetTop(state, top + 1);
  DCHECK_EQ(lua::GetType(state, -1), lua::LuaType::Table);
}

// Helper to create signal and push it to stack.
template<typename T>
void PushSignal(lua::State* state, int index, const char* name, T member) {
  int top = lua::GetTop(state);
  index = lua::AbsIndex(state, index);
  // Check if the member has already been converted.
  PushObjectSignalsTable(state, index);
  lua::RawGet(state, -1, name);
  if (lua::GetType(state, -1) != lua::LuaType::UserData) {
    // Create the wrapper for signal class.
    void* memory = lua_newuserdata(state, sizeof(Signal<T>));
    new(memory) Signal<T>(state, index, member);
    if (luaL_newmetatable(state, "yue.Signal")) {
      // The signal class doesn't have a destructor, so there is no need to add
      // hook to __gc.
      lua::RawSet(state, -1, "__index", lua::ValueOnStack(state, -1));
      lua::RawSet(state, -1, "connect", &SignalBase::Connect);
    }
    lua::SetMetaTable(state, -2);
    lua::RawSet(state, top + 1, name, lua::ValueOnStack(state, -1));
  }
  // Pop the table and keep the signal.
  lua::Insert(state, top + 1);
  lua::SetTop(state, top + 1);
  DCHECK_EQ(lua::GetType(state, -1), lua::LuaType::UserData);
}

}  // namespace yue

namespace lua {

template<>
struct Type<yue::SignalBase*> {
  static constexpr const char* name = "yue.Signal";
  static inline bool To(State* state, int index, yue::SignalBase** out) {
    index = AbsIndex(state, index);
    StackAutoReset reset(state);
    if (GetType(state, index) != lua::LuaType::UserData ||
        !GetMetaTable(state, index))
      return false;
    base::StringPiece iname;
    if (!RawGetAndPop(state, -1, "__name", &iname) || iname != name)
      return false;
    // Signal inherites SignalBase singlely, so it is safe to cast.
    *out = reinterpret_cast<yue::SignalBase*>(lua_touserdata(state, index));
    return true;
  }
};

}  // namespace lua

#endif  // YUE_API_SIGNAL_H_
