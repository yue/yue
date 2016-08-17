// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef LUA_WRAPPABLE_H_
#define LUA_WRAPPABLE_H_

#include "lua/table.h"
#include "lua/wrappable_internal.h"

namespace lua {

// Generate metatable for native classes.
template<typename T>
struct MetaTable {
  // Create the metatable for T and push it on stack.
  static void Push(State* state) {
    internal::InheritanceChain<T>::Push(state);
  }
  template<typename... BaseTypes>
  static void Push(State* state) {
    internal::InheritanceChain<T>::template Push<BaseTypes...>(state);
  }

  // Create an instance of T.
  // The returned pointer is managed by lua, so avoid keeping it.
  template<typename... ArgTypes>
  static T* NewInstance(State* state, const ArgTypes&... args) {
    StackAutoReset reset(state);
    T* instance = new T(args...);
    void* memory = lua_newuserdata(state, sizeof(internal::PointerWrapper<T>));
    new(memory) internal::PointerWrapper<T>(state, instance);
    luaL_getmetatable(state, Type<T>::name);
    DCHECK_EQ(lua::GetType(state, -1), lua::LuaType::Table)
        << "The class must be created before creating the instance";
    SetMetaTable(state, -2);
    return instance;
  }
};

// The default type information for RefCounted class.
template<typename T>
struct Type<T*, typename std::enable_if<std::is_convertible<
                    T*, base::subtle::RefCountedBase*>::value>::type> {
  static constexpr const char* name = Type<T>::name;
  static inline bool To(State* state, int index, T** out) {
    index = AbsIndex(state, index);
    StackAutoReset reset(state);
    // Verify the type and length.
    if (GetType(state, index) != lua::LuaType::UserData ||
        RawLen(state, index) != sizeof(internal::PointerWrapper<T>) ||
        !GetMetaTable(state, index))
      return false;
    // Convert pointer to actual class.
    auto* wrapper = static_cast<internal::PointerWrapper<T>*>(
        lua_touserdata(state, index));
    *out = wrapper->get();
    return true;
  }
  static inline void Push(State* state, T* ptr) {
    internal::PointerWrapperBase::Push(state, ptr);
  }
};

}  // namespace lua

#endif  // LUA_WRAPPABLE_H_
