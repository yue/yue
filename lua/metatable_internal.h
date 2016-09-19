// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef LUA_METATABLE_INTERNAL_H_
#define LUA_METATABLE_INTERNAL_H_

#include <string>

#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "lua/table.h"

namespace lua {

namespace internal {

// Base of wrapper classes that maps pointers to lua references.
class WrapperBase {
 public:
  // Convert the class to Lua and push it on stack.
  static bool Push(State* state, void* ptr);

 protected:
  WrapperBase(State* state, void* ptr);

 private:
  DISALLOW_COPY_AND_ASSIGN(WrapperBase);
};

// The specialized Wrappable class for storing refcounted class.
// We need to guareentee that RefPtrWrapper is standard layout, since we may
// use convertions like RefPtrWrapper<Derive> to RefPtrWrapper<Base>.
template<typename T>
class RefPtrWrapper : public WrapperBase {
 public:
  RefPtrWrapper(State* state, T* t) : WrapperBase(state, t), ptr_(t) {
    ptr_->AddRef();
  }

  ~RefPtrWrapper() {
    ptr_->Release();
  }

  T* get() const { return ptr_; }

 private:
  T* ptr_;
};

// The specialized Wrappable class for storing weakptr class.
template<typename T>
class WeakPtrWrapper : public WrapperBase {
 public:
  WeakPtrWrapper(State* state, base::WeakPtr<T> t)
      : WrapperBase(state, t.get()), ptr_(t) {}

  T* get() const { return ptr_.get(); }

 private:
  base::WeakPtr<T> ptr_;
};

// Find the correct GC function for each type.
template<typename T, typename Enable = void>
struct FindOnGC;

template<typename T>
struct FindOnGC<T, typename std::enable_if<std::is_base_of<
                       base::subtle::RefCountedBase, T>::value>::type> {
  using type = RefPtrWrapper<T>;
};

template<typename T>
struct FindOnGC<T, typename std::enable_if<std::is_base_of<
    base::internal::WeakPtrBase,
    decltype((reinterpret_cast<T*>(nullptr))->GetWeakPtr())>::value>::type> {
  using type = WeakPtrWrapper<T>;
};

// The default __index handler which looks up in the metatable.
int DefaultPropertyLookup(State* state);

// Check whether T has an Index handler, if not use the default property lookup
// that searches in metatable.
template<typename T, typename Enable = void>
struct Indexer {
  static inline void Set(State* state, int index) {
    RawSet(state, index, "__index", ValueOnStack(state, index));
  }
};

template<typename T>
struct Indexer<T, typename std::enable_if<std::is_pointer<
                      decltype(&Type<T>::Index)>::value>::type> {
  static inline void Set(State* state, int index) {
    RawSet(state, index, "__index", CFunction(&Index));
  }
  static int Index(State* state) {
    // The DefaultPropertyLookup may throw error, so wrap the C++ stack.
    {
      std::string name;
      if (To(state, -1, &name) && Type<T>::Index(state, name))
        return 1;
    }
    // Go to the default routine.
    return DefaultPropertyLookup(state);
  }
};

// Check whether T has an NewIndex handler, if it does then set __newindex.
template<typename T, typename Enable = void>
struct NewIndexer {
  static inline void Set(State* state, int index) {
  }
};

template<typename T>
struct NewIndexer<T, typename std::enable_if<std::is_function<
                         decltype(Type<T>::NewIndex)>::value>::type> {
  static inline void Set(State* state, int index) {
    RawSet(state, index, "__newindex", CFunction(&NewIndex));
  }
  static int NewIndex(State* state) {
    // Wrap the C++ stack.
    {
      std::string name;
      if (To(state, 2, &name) && Type<T>::NewIndex(state, name))
        return 0;
    }
    lua::Push(state, "unaccepted assignment");
    lua_error(state);
    return 0;
  }
};

// Create metatable for T, returns true if the metattable has already been
// created.
template<typename T>
bool PushSingleTypeMetaTable(State* state) {
  if (luaL_newmetatable(state, Type<T>::name) == 0)
    return true;

  RawSet(state, -1, "__gc", CFunction(&OnGC<typename FindOnGC<T>::type>));
  Indexer<T>::Set(state, -1);
  NewIndexer<T>::Set(state, -1);
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
