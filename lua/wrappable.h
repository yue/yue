// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef LUA_WRAPPABLE_H_
#define LUA_WRAPPABLE_H_

#include "lua/table.h"
#include "lua/wrappable_internal.h"

namespace lua {

// Inherit this to provide metatable based class in lua.
template<typename T>
class Wrappable : public internal::WrappableBase {
 public:
  static void PushNewClass(State* state) {
    PushNewTable(state);
    RawSet(state, -1, "__index", ValueOnStack(state, -1));
    T::SetMetaTable(state, -1);
  }

 protected:
  Wrappable() {}
  ~Wrappable() override {}

 private:
  DISALLOW_COPY_AND_ASSIGN(Wrappable);
};

// The default type information for any subclass of Wrappable.
template<typename T>
struct Type<T*, typename std::enable_if<std::is_convertible<
                    T*, internal::WrappableBase*>::value>::type> {
  static constexpr const char* name = "userdata";
  static inline bool To(State* state, int index, T** out) {
    if (GetType(state, index) != lua::LuaType::UserData ||
        RawLen(state, index) != sizeof(T))
      return false;
    *out = static_cast<T*>(lua_touserdata(state, index));
    return true;
  }
};

}  // namespace lua

#endif  // LUA_WRAPPABLE_H_
