// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef LUA_INDEX_H_
#define LUA_INDEX_H_

#include "lua/table.h"
#include "lua/user_data.h"

namespace lua {

namespace internal {

// The default __index handler which looks up in the metatable.
int DefaultPropertyLookup(State* state);

// Look into the members set by RawSetProperty.
int MemberLookup(State* state);

// Assign to the members set by RawSetProperty.
int MemberAssign(State* state);

// Get type from member pointer.
template<typename T> struct ExtractMemberPointer;
template<typename TType, typename TMember>
struct ExtractMemberPointer<TMember(TType::*)> {
  using ClassType = TType;
  using MemberType = TMember;
};

// Holds the C++ member pointer.
class MemberHolderBase {
 public:
  virtual ~MemberHolderBase() {}
  virtual int Index(State* state) = 0;
  virtual int NewIndex(State* state) = 0;
};

template<typename T>
class MemberHolder : public MemberHolderBase {
 public:
  explicit MemberHolder(T ptr) : ptr_(ptr) {}
  ~MemberHolder() override {}

  int Index(State* state) override;
  int NewIndex(State* state) override;

 private:
  using ClassType = typename ExtractMemberPointer<T>::ClassType;
  using MemberType = typename ExtractMemberPointer<T>::MemberType;

  T ptr_;

  DISALLOW_COPY_AND_ASSIGN(MemberHolder);
};

template<typename T>
int MemberHolder<T>::Index(State* state) {
  ClassType* owner;
  if (!To(state, 1, &owner))
    return 0;
  Push(state, owner->*ptr_);
  return 1;
}

template<typename T>
int MemberHolder<T>::NewIndex(State* state) {
  ClassType* owner;
  if (!To(state, 1, &owner))
    return 0;
  if (!To(state, 3, &(owner->*ptr_))) {
    PushFormatedString(state, "error converting %s to %s",
                       GetTypeName(state, 3), Type<MemberType>::name);
    lua_error(state);
    NOTREACHED() << "Code after lua_error() gets called";
  }
  return 1;
}

// Create holder class for the member.
template<typename Member>
void SetMemberHolder(State* state, int table,
                     base::StringPiece key, Member member) {
  StackAutoReset reset(state);
  NewUserData<MemberHolder<Member>>(state, member);
  RawSet(state, table, key, ValueOnStack(state, -1));
}

template<typename Member, typename... ArgTypes>
void SetMemberHolder(State* state, int table,
                     base::StringPiece key, Member member, ArgTypes... args) {
  SetMemberHolder(state, table, key, member);
  SetMemberHolder(state, table, args...);
}

}  // namespace internal

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
    int r = Type<T>::Index(state);
    if (r > 0)
      return r;
    // Go to the default routine.
    return internal::DefaultPropertyLookup(state);
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
    int r = Type<T>::NewIndex(state);
    if (r > 0)
      return r;
    lua::Push(state, "unaccepted assignment");
    lua_error(state);
    return 0;
  }
};

// Define properties for the metatable.
template<typename... ArgTypes>
void RawSetProperty(State* state, int metatable, ArgTypes... args) {
  StackAutoReset reset(state);
  // Upvalue for storing pre-defined members.
  NewTable(state, 0, sizeof...(args));
  internal::SetMemberHolder(state, AbsIndex(state, -1), args...);
  // Define the __index and __newindex handlers.
  RawSet(state, metatable,
         "__index", CClosure(state, &internal::MemberLookup, 1),
         "__newindex", CClosure(state, &internal::MemberAssign, 1));
}

}  // namespace lua

#endif  // LUA_INDEX_H_
