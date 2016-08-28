// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "yue/api_gui.h"

#include "nativeui/nativeui.h"

namespace lua {

template<>
struct Type<nu::Rect> {
  static constexpr const char* name = "yue.Rect";
  static inline void Push(State* state, const nu::Rect& rect) {
    lua::PushNewTable(state);
    lua::RawSet(state, -1,
                "x", rect.x(), "y", rect.y(),
                "width", rect.width(), "height", rect.height());
  }
  static inline bool To(State* state, int index, nu::Rect* out) {
    if (GetType(state, index) != LuaType::Table)
      return false;
    int x, y, width, height;
    if (!RawGetAndPop(state, index,
                      "x", &x, "y", &y, "width", &width, "height", &height))
      return false;
    *out = nu::Rect(x, y, width, height);
    return true;
  }
};

template<>
struct Type<nu::Window::Options> {
  static constexpr const char* name = "yue.Window.Options";
  static inline bool To(State* state, int index, nu::Window::Options* out) {
    if (GetType(state, index) == LuaType::Table)
      RawGetAndPop(state, index, "bounds", &out->bounds);
    return true;
  }
};

template<>
struct Type<nu::Window> {
  static constexpr const char* name = "yue.Window";
  static void BuildMetaTable(State* state, int index) {
    RawSet(state, index,
           "new", &MetaTable<nu::Window>::NewInstance<nu::Window::Options>,
           "close", &nu::Window::Close,
           "setcontentbounds", &nu::Window::SetContentBounds,
           "getcontentbounds", &nu::Window::GetContentBounds,
           "setbounds", &nu::Window::SetBounds,
           "getbounds", &nu::Window::GetBounds,
           "setvisible", &nu::Window::SetVisible,
           "isvisible", &nu::Window::IsVisible);
  }
};

}  // namespace lua

extern "C" int luaopen_yue_gui(lua::State* state) {
  // Put the gui state into registry, so it is alive through whole lua state.
  void* memory = lua_newuserdata(state, sizeof(nu::State));
  new(memory) nu::State;
  luaL_ref(state, LUA_REGISTRYINDEX);

  // Populate the table with GUI elements.
  lua::PushNewTable(state);
  lua::Push(state, "Window");
  lua::MetaTable<nu::Window>::Push(state);
  lua_rawset(state, -3);
  return 1;
}
