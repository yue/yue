// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef LUA_METATABLE_H_
#define LUA_METATABLE_H_

#include <type_traits>

#include "lua/metatable_internal.h"

namespace lua {

// Generate metatable for native classes.
template<typename T, typename Enable = void>
struct MetaTable;

// Create metatable for RefCounted classes.
template<typename T>
struct MetaTable<T, typename std::enable_if<std::is_base_of<
                        base::subtle::RefCountedBase, T>::value>::type> {
  // Create the metatable for T and push it on stack.
  static void Push(State* state) {
    internal::InheritanceChain<T>::Push(state);
  }

  // Create an instance of T.
  // The returned pointer is managed by lua, so avoid keeping it.
  template<typename... ArgTypes>
  static T* NewInstance(State* state, const ArgTypes&... args) {
    StackAutoReset reset(state);
    T* instance = new T(args...);
    PushNewWrapper(state, instance);
    return instance;
  }

  // Create a new lua wrapper for T.
  static void PushNewWrapper(State* state, T* instance) {
    void* memory = lua_newuserdata(state, sizeof(internal::RefPtrWrapper<T>));
    new(memory) internal::RefPtrWrapper<T>(state, instance);
    static_assert(std::is_standard_layout<internal::RefPtrWrapper<T>>::value,
                  "The internal::RefPtrWrapper<T> must be standard layout");
    luaL_getmetatable(state, Type<T>::name);
    DCHECK_EQ(lua::GetType(state, -1), lua::LuaType::Table)
        << "The class must be created before creating the instance";
    SetMetaTable(state, -2);
  }

  // Check if current metatable is base of the metatable on top of stack.
  static bool IsBaseOf(State* state) {
    StackAutoReset reset(state);
    base::StringPiece name;
    if (RawGetAndPop(state, -1, "__name", &name) && name == Type<T>::name)
      return true;
    if (!GetMetaTable(state, -1))
      return false;
    RawGet(state, -1, "__index");
    return IsBaseOf(state);
  }
};

// The default type information for RefCounted class.
template<typename T>
struct Type<T*, typename std::enable_if<std::is_base_of<
                    base::subtle::RefCountedBase, T>::value>::type> {
  static constexpr const char* name = Type<T>::name;
  static bool To(State* state, int index, T** out) {
    index = AbsIndex(state, index);
    StackAutoReset reset(state);
    // Verify the type and length.
    if (GetType(state, index) != lua::LuaType::UserData ||
        RawLen(state, index) != sizeof(internal::RefPtrWrapper<T>) ||
        !GetMetaTable(state, index))
      return false;
    // Verify fine the inheritance chain.
    if (!MetaTable<T>::IsBaseOf(state))
      return false;
    // Convert pointer to actual class.
    auto* wrapper = static_cast<internal::RefPtrWrapper<T>*>(
        lua_touserdata(state, index));
    *out = wrapper->get();
    return true;
  }
  static inline void Push(State* state, T* ptr) {
    if (!ptr)
      lua::Push(state, nullptr);
    else if (!internal::WrapperBase::Push(state, ptr))
      MetaTable<T>::PushNewWrapper(state, ptr);
  }
};

// Create metatable for classes that produce WeakPtr.
template<typename T>
struct MetaTable<T, typename std::enable_if<std::is_base_of<
                        base::internal::WeakPtrBase,
                        decltype(((T*)nullptr)->GetWeakPtr())>::value>::type> {
  static void Push(State* state) {
    internal::InheritanceChain<T>::Push(state);
  }

  static void PushNewWrapper(State* state, T* instance) {
    void* memory = lua_newuserdata(state, sizeof(internal::WeakPtrWrapper<T>));
    new(memory) internal::WeakPtrWrapper<T>(state, instance->GetWeakPtr());
    luaL_getmetatable(state, Type<T>::name);
    DCHECK_EQ(lua::GetType(state, -1), lua::LuaType::Table)
        << "The class must be created before creating the instance";
    SetMetaTable(state, -2);
  }
};

// The default type information for WeakPtr class.
template<typename T>
struct Type<T*, typename std::enable_if<std::is_base_of<
                    base::internal::WeakPtrBase,
                    decltype(((T*)nullptr)->GetWeakPtr())>::value>::type> {
  static constexpr const char* name = Type<T>::name;
  static bool To(State* state, int index, T** out) {
    index = AbsIndex(state, index);
    StackAutoReset reset(state);
    // Verify the type and length.
    if (GetType(state, index) != lua::LuaType::UserData ||
        RawLen(state, index) != sizeof(internal::WeakPtrWrapper<T>))
      return false;
    // Convert pointer to actual class.
    auto* wrapper = static_cast<internal::WeakPtrWrapper<T>*>(
        lua_touserdata(state, index));
    T* ptr = wrapper->get();
    // WeakPtr might be invalidated.
    if (!ptr)
      return false;
    *out = ptr;
    return true;
  }
  static inline void Push(State* state, T* ptr) {
    if (!ptr)
      lua::Push(state, nullptr);
    // Do not cache the pointer of WeakPtr, because the pointer may point to a
    // variable on stack, which can have same address with previous variable on
    // the stack.
    MetaTable<T>::PushNewWrapper(state, ptr);
  }
};

}  // namespace lua

#endif  // LUA_METATABLE_H_
