// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "lua_yue/builtin_loader.h"

#include <algorithm>
#include <string>
#include <utility>

#include "lua_yue/binding_gui.h"
#include "lua_yue/binding_sys.h"
#include "lua_yue/binding_util.h"

#if LUA_VERSION_NUM >= 502
# define PACKAGE_SEARCHERS "searchers"
#else
# define PACKAGE_SEARCHERS "loaders"
#endif

namespace yue {

namespace {

// The search table, it must be manually kept in sorted order.
std::pair<const char*, lua_CFunction> kLoadersMap[] = {
  std::make_pair("yue.gui", luaopen_yue_gui),
  std::make_pair("yue.sys", luaopen_yue_sys),
  std::make_pair("yue.util", luaopen_yue_util),
};

// Compare function to compare elements.
template<typename T>
bool ElementCompare(const T& e1, const T& e2) {
  return base::StringPiece(std::get<0>(e1)) < std::get<0>(e2);
}

// Use the first element of tuple as comparing key.
bool TupleCompare(const std::pair<const char*, lua_CFunction>& element,
                  const std::string& key) {
  return element.first < key;
}

int SearchBuiltin(lua::State* state) {
  DCHECK(std::is_sorted(std::begin(kLoadersMap), std::end(kLoadersMap),
                        ElementCompare<std::pair<const char*, lua_CFunction>>))
      << "The builtin loaders map must be in sorted order";
  std::string name;
  lua::To(state, 1, &name);
  auto* iter = std::lower_bound(std::begin(kLoadersMap), std::end(kLoadersMap),
                                name, TupleCompare);
  if (iter == std::end(kLoadersMap) || name != iter->first) {
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
  CHECK_EQ(lua::GetType(state, -1), lua::LuaType::Table)
      << "package should be a table";
  lua::RawGet(state, -1, PACKAGE_SEARCHERS);
  CHECK_EQ(lua::GetType(state, -1), lua::LuaType::Table)
      << "package." PACKAGE_SEARCHERS " should be a table";

  // table.insert(pacakge.searchers, 2, search_builtin)
  int len = static_cast<int>(lua::RawLen(state, -1));
  for (int i = len; i >= 2; --i) {
    lua::RawGet(state, -1, i);
    lua_rawseti(state, -2, i + 1);
  }
  lua::RawSet(state, -1, 2, lua::CFunction(&SearchBuiltin));
}

}  // namespace yue
