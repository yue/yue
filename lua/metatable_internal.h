// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef LUA_METATABLE_INTERNAL_H_
#define LUA_METATABLE_INTERNAL_H_

#include "base/memory/ref_counted.h"
#include "lua/table.h"

namespace lua {

namespace internal {

// Base of PointerWrapper that hides implementations.
class PointerWrapperBase {
 public:
  // Convert the class to Lua and push it on stack.
  static bool Push(State* state, void* ptr);

 protected:
  PointerWrapperBase(State* state, void* ptr);

 private:
  DISALLOW_COPY_AND_ASSIGN(PointerWrapperBase);
};

// The specialized Wrappable class for storing refcounted class.
// We need to guareentee that PointerWrapper is standard layout, since we may
// use convertions like PointerWrapper<Derive> to PointerWrapper<Base>.
template<typename T>
class PointerWrapper : public PointerWrapperBase {
 public:
  // Call destructors on garbage collection.
  static int OnGC(State* state) {
    auto* self = static_cast<PointerWrapper*>(lua_touserdata(state, 1));
    self->~PointerWrapper();
    return 0;
  }

  PointerWrapper(State* state, T* t) : PointerWrapperBase(state, t), ptr_(t) {
    ptr_->AddRef();
  }

  T* get() const { return ptr_; }

 private:
  ~PointerWrapper() {
    ptr_->Release();
  }

  T* ptr_;
};

// Create metatable for T, returns true if the metattable has already been
// created.
template<typename T>
bool PushSingleTypeMetaTable(State* state) {
  if (luaL_newmetatable(state, Type<T>::name) == 0)
    return true;

  RawSet(state, -1, "__index", ValueOnStack(state, -1),
                    "__gc", CFunction(PointerWrapper<T>::OnGC));
  Type<T>::BuildMetaTable(state, -1);
  return false;
}

// Create metattable inheritance chain for T and its BaseTypes.
template<typename T, typename Enable = void>
struct InheritanceChain {
  // There is no base type.
  static inline void Push(State* state) {
    PushSingleTypeMetaTable<T>(state);
  }
};

template<typename T>
struct InheritanceChain<T, typename std::enable_if<std::is_class<
                               typename Type<T>::base>::value>::type> {
  static inline void Push(State* state) {
    if (PushSingleTypeMetaTable<T>(state))  // already created.
      return;

    // Inherit from base type's metatable.
    InheritanceChain<typename Type<T>::base>::Push(state);
    PushNewTable(state, 0, 1);
    RawSet(state, -1, "__index", ValueOnStack(state, -2));
    SetMetaTable(state, -3);
    PopAndIgnore(state, 1);
  }
};

}  // namespace internal

}  // namespace lua

#endif  // LUA_METATABLE_INTERNAL_H_
