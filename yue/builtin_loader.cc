// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "yue/builtin_loader.h"

#include <string>
#include <unordered_map>

#include "yue/api_message_loop.h"

namespace yue {

namespace {

// The search table.
std::unordered_map<std::string, lua_CFunction> loaders_map = {
  { "yue.MessageLoop", luaopen_yue_message_loop },
};

int SearchBuiltin(lua::State* state) {
  std::string name;
  lua::To(state, 1, &name);
  auto iter = loaders_map.find(name);
  if (iter == loaders_map.end()) {
    lua::PushFormatedString(state, "\n\tno builtin '%s'", name.c_str());
    return 1;
  } else {
    lua::Push(state, lua::CFunction(iter->second));
    return 1;
  }
}

}  // namespace

void InsertBuiltinModuleLoader(lua::State* state) {
  lua::StackAutoReset reset(state);
  lua_getglobal(state, "package");
  DCHECK_EQ(lua::GetType(state, -1), lua::LuaType::Table)
      << "package should be a table";
  lua::RawGet(state, -1, "searchers");
  DCHECK_EQ(lua::GetType(state, -1), lua::LuaType::Table)
      << "package.searchers should be a table";

  // table.insert(pacakge.searchers, 2, search_builtin)
  int len = lua::RawLen(state, -1);
  for (int i = len; i >= 2; --i) {
    lua::RawGet(state, -1, i);
    lua_rawseti(state, -2, i + 1);
  }
  lua::RawSet(state, -1, 2, lua::CFunction(&SearchBuiltin));
}

}  // namespace yue
