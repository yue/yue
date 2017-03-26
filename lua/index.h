// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef LUA_INDEX_H_
#define LUA_INDEX_H_

#include "lua/table.h"
#include "lua/user_data.h"

namespace lua {

// Describe how a member T can be converted.
template<typename T, typename Enable = void>
struct MemberTraits {
  // Decides should we return cached value or converted value.
  static const bool kShouldCacheValue = true;
  // Converter used when we decide not to use cached value.
  static inline void Push(State* state, int cache, const T& ptr) {
    lua::Push(state, nullptr);
  }
};

// For primary types we just do normal conversion.
template<typename T>
struct MemberTraits<T, typename std::enable_if<
                           std::is_fundamental<T>::value>::type> {
  static const bool kShouldCacheValue = false;
  static inline void Push(State* state, int cache, const T& ptr) {
    lua::Push(state, ptr);
  }
};

namespace internal {

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

// Provides the virtual interface which will be called by MemberLookup/Assign.
class MemberHolderBase {
 public:
  virtual ~MemberHolderBase() {}
  virtual int Index(State* state) = 0;
  virtual int NewIndex(State* state) = 0;
};

// Holds the type information and pointer to a member data.
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
  int cache = lua_upvalueindex(3);
  if (MemberTraits<MemberType>::kShouldCacheValue) {
    RawGet(state, cache, ValueOnStack(state, 2));
  } else {
    MemberTraits<MemberType>::Push(state, cache, owner->*ptr_);
  }
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
  if (MemberTraits<MemberType>::kShouldCacheValue) {
    int cache = lua_upvalueindex(3);
    RawSet(state, cache, ValueOnStack(state, 2), ValueOnStack(state, 3));
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

// Define the default __index and __newindex.
void RawSetDefaultPropertyHandler(State* state, int metatable);

// Define properties for the metatable.
template<typename... ArgTypes>
void RawSetProperty(State* state, int metatable, ArgTypes... args) {
  StackAutoReset reset(state);
  // Upvalue for storing user-added members.
  NewTable(state, 0, 0);
  // Upvalue for storing pre-defined members.
  NewTable(state, 0, sizeof...(args) / 2);
  internal::SetMemberHolder(state, AbsIndex(state, -1), args...);
  // Upvalue for cache.
  NewTable(state, 0, sizeof...(args) / 2);
  // Define the __index and __newindex handlers.
  RawSet(state, metatable,
         "__index", CClosure(state, &internal::MemberLookup, 3),
         "__newindex", CClosure(state, &internal::MemberAssign, 3));
}

}  // namespace lua

#endif  // LUA_INDEX_H_
