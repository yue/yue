// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef YUE_API_SIGNAL_H_
#define YUE_API_SIGNAL_H_

#include "lua/lua.h"
#include "nativeui/signal.h"

namespace yue {

// Push a weak table which records the object's members.
void PushObjectMembersTable(lua::State* state, int index);

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
  virtual int Connect(lua::CallContext* context) = 0;
  virtual void Disconnect(lua::CallContext* context, int id) = 0;
  virtual void DisconnectAll(lua::CallContext* context) = 0;
};

template<typename T>
class Signal : SignalBase {
 public:
  using Type = typename ExtractMemberPointer<T>::Type;
  using Slot = typename ExtractMemberPointer<T>::Member::Slot;

  // The singal doesn't store the object or the member directly, instead it only
  // keeps a weak reference to the object and stores the pointer to member, so
  // it can still work when user copies the signal and uses it after the object
  // gets deleted.
  Signal(lua::State* state, int index, T member)
      : object_ref_(lua::CreateWeakReference(state, index)),
        member_(member) {}

  int Connect(lua::CallContext* context) override {
    Slot slot;
    if (!lua::Pop(context->state, &slot)) {
      context->has_error = true;
      lua::Push(context->state, "first arg must be function");
      return -1;
    }

    Type* object;
    if (!GetObject(context, &object))
      return -1;

    return (object->*member_).Connect(slot);
  }

  void Disconnect(lua::CallContext* context, int id) override {
    Type* object;
    if (!GetObject(context, &object))
      return;

    (object->*member_).Disconnect(id);
  }

  void DisconnectAll(lua::CallContext* context) override {
    Type* object;
    if (!GetObject(context, &object))
      return;

    (object->*member_).DisconnectAll();
  }

 private:
  bool GetObject(lua::CallContext* context, Type** object) {
    lua::PushWeakReference(context->state, object_ref_);
    if (!lua::Pop(context->state, object)) {
      context->has_error = true;
      lua::Push(context->state, "owner of signal is gone");
      return false;
    }
    return true;
  }

  int object_ref_;
  T member_;
};

// Helper to create signal and push it to stack.
template<typename T>
void PushSignal(lua::State* state, int index, const char* name, T member) {
  int top = lua::GetTop(state);
  index = lua::AbsIndex(state, index);
  // Check if the member has already been converted.
  PushObjectMembersTable(state, index);
  lua::RawGet(state, -1, name);
  if (lua::GetType(state, -1) != lua::LuaType::UserData) {
    // Create the wrapper for signal class.
    void* memory = lua_newuserdata(state, sizeof(Signal<T>));
    new(memory) Signal<T>(state, index, member);
    static_assert(std::is_trivially_destructible<Signal<T>>::value,
                  "we are not providing __gc so Signal<T> must be trivial");
    if (luaL_newmetatable(state, "yue.Signal")) {
      // The signal class doesn't have a destructor, so there is no need to add
      // hook to __gc.
      lua::RawSet(state, -1, "__index", lua::ValueOnStack(state, -1),
                             "connect", &SignalBase::Connect,
                             "disconnect", &SignalBase::Disconnect,
                             "disconnectall", &SignalBase::DisconnectAll);
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
