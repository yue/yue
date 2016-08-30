// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "yue/api_gui.h"

#include <string>

#include "nativeui/nativeui.h"
#include "yue/api_signal.h"

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
struct Type<nu::View> {
  static constexpr const char* name = "yue.View";
  static void BuildMetaTable(State* state, int index) {
    RawSet(state, index,
           "setbounds", &nu::View::SetBounds,
           "getbounds", &nu::View::GetBounds,
           "getparent", &nu::View::parent);
  }
};

template<>
struct Type<nu::Button> {
  using base = nu::View;
  static constexpr const char* name = "yue.Button";
  static void BuildMetaTable(State* state, int index) {
    RawSet(state, index,
           "new", &MetaTable<nu::Button>::NewInstance<const std::string&>,
           "settitle", &nu::Button::SetTitle,
           "gettitle", &nu::Button::GetTitle);
  }
};

template<>
struct Type<nu::Label> {
  using base = nu::View;
  static constexpr const char* name = "yue.Label";
  static void BuildMetaTable(State* state, int index) {
    RawSet(state, index,
           "new", &MetaTable<nu::Label>::NewInstance<const std::string&>,
           "settext", &nu::Label::SetText,
           "gettext", &nu::Label::GetText);
  }
};

template<>
struct Type<nu::Group> {
  using base = nu::View;
  static constexpr const char* name = "yue.Group";
  static void BuildMetaTable(State* state, int index) {
    RawSet(state, index,
           "new", &MetaTable<nu::Group>::NewInstance<const std::string&>,
           "setcontentview", &nu::Group::SetContentView,
           "getcontentview", &nu::Group::GetContentView,
           "settitle", &nu::Group::SetTitle,
           "gettitle", &nu::Group::GetTitle);
  }
};

template<>
struct Type<nu::Container> {
  using base = nu::View;
  static constexpr const char* name = "yue.Container";
  static void BuildMetaTable(State* state, int index) {
    RawSet(state, index,
           "new", &MetaTable<nu::Container>::NewInstance<>,
           "setlayout", &nu::Container::SetLayoutManager,
           "getlayout", &nu::Container::GetLayoutManager,
           "addchildview", &nu::Container::AddChildView,
           "addchildviewat", &nu::Container::AddChildViewAt,
           "removechildview", &nu::Container::RemoveChildView,
           "childcount", &nu::Container::child_count,
           "childat", &nu::Container::child_at);
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
           "setcontentview", &nu::Window::SetContentView,
           "getcontentview", &nu::Window::GetContentView,
           "setvisible", &nu::Window::SetVisible,
           "isvisible", &nu::Window::IsVisible);
  }
  static bool Index(State* state, nu::Window* self, const std::string& name) {
    if (name == "onclose") {
      yue::PushSignal(state, 1, "onclose", &nu::Window::on_close);
      return true;
    }
    return false;
  }
};

template<>
struct Type<nu::LayoutManager> {
  static constexpr const char* name = "yue.LayoutManager";
  static void BuildMetaTable(State* state, int index) {
  }
};

template<>
struct Type<nu::FillLayout> {
  using base = nu::LayoutManager;
  static constexpr const char* name = "yue.FillLayout";
  static void BuildMetaTable(State* state, int index) {
    RawSet(state, index, "new", &MetaTable<nu::FillLayout>::NewInstance<>);
  }
};

template<>
struct Type<nu::BoxLayout::Orientation> {
  static constexpr const char* name = "yue.BoxLayout.Orientation";
  static inline bool To(State* state, int index,
                        nu::BoxLayout::Orientation* out) {
    std::string orientation;
    if (!lua::To(state, index, &orientation))
      return false;
    if (orientation == "vertical") {
      *out = nu::BoxLayout::Vertical;
      return true;
    } else if (orientation == "horizontal") {
      *out = nu::BoxLayout::Horizontal;
      return true;
    } else {
      return false;
    }
  }
};

template<>
struct Type<nu::BoxLayout> {
  using base = nu::LayoutManager;
  static constexpr const char* name = "yue.BoxLayout";
  static void BuildMetaTable(State* state, int index) {
    RawSet(state, index,
           "new",
           &MetaTable<nu::BoxLayout>::NewInstance<nu::BoxLayout::Orientation>);
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
  lua::Push(state, "Container");
  lua::MetaTable<nu::Container>::Push(state);
  lua_rawset(state, -3);
  lua::Push(state, "Button");
  lua::MetaTable<nu::Button>::Push(state);
  lua_rawset(state, -3);
  lua::Push(state, "Label");
  lua::MetaTable<nu::Label>::Push(state);
  lua_rawset(state, -3);
  lua::Push(state, "Group");
  lua::MetaTable<nu::Group>::Push(state);
  lua_rawset(state, -3);
  lua::Push(state, "FillLayout");
  lua::MetaTable<nu::FillLayout>::Push(state);
  lua_rawset(state, -3);
  lua::Push(state, "BoxLayout");
  lua::MetaTable<nu::BoxLayout>::Push(state);
  lua_rawset(state, -3);
  return 1;
}
