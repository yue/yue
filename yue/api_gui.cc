// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "yue/api_gui.h"

#include <string>

#include "nativeui/nativeui.h"
#include "yue/api_signal.h"

namespace lua {

template<>
struct Type<nu::Size> {
  static constexpr const char* name = "yue.Size";
  static inline void Push(State* state, const nu::Size& size) {
    lua::PushNewTable(state);
    lua::RawSet(state, -1, "width", size.width(), "height", size.height());
  }
  static inline bool To(State* state, int index, nu::Size* out) {
    if (GetType(state, index) != LuaType::Table)
      return false;
    int width, height;
    if (!RawGetAndPop(state, index, "width", &width, "height", &height))
      return false;
    *out = nu::Size(width, height);
    return true;
  }
};

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
struct Type<nu::Insets> {
  static constexpr const char* name = "yue.Insets";
  static inline void Push(State* state, const nu::Insets& insets) {
    lua::PushNewTable(state);
    lua::RawSet(state, -1, "top", insets.top(), "left", insets.left(),
                           "bottom", insets.bottom(), "right", insets.right());
  }
  static inline bool To(State* state, int index, nu::Insets* out) {
    if (GetType(state, index) != LuaType::Table)
      return false;
    int top, left, bottom, right;;
    if (!RawGetAndPop(state, index, "top", &top, "left", &left,
                                    "bottom", &bottom, "right", &right))
      return false;
    *out = nu::Insets(top, left, bottom, right);
    return true;
  }
};

template<>
struct Type<nu::View> {
  static constexpr const char* name = "yue.View";
  static void BuildMetaTable(State* state, int index) {
    RawSet(state, index,
           "getpreferredsize", &nu::View::preferred_size,
           "setpreferredsize", &nu::View::SetPreferredSize,
           "setbounds", &nu::View::SetBounds,
           "getbounds", &nu::View::GetBounds,
           "setvisible", &nu::View::SetVisible,
           "isvisible", &nu::View::IsVisible,
           "getparent", &nu::View::parent);
  }
};

template<>
struct Type<nu::Button::Type> {
  static constexpr const char* name = "yue.Button.Type";
  static bool To(State* state, int index, nu::Button::Type* out) {
    std::string type;
    if (!lua::To(state, index, &type))
      return false;
    if (type.empty() || type == "normal") {
      *out = nu::Button::Normal;
      return true;
    } else if (type == "checkbox") {
      *out = nu::Button::CheckBox;
      return true;
    } else if (type == "radio") {
      *out = nu::Button::Radio;
      return true;
    } else {
      return false;
    }
  }
};

template<>
struct Type<nu::Button> {
  using base = nu::View;
  static constexpr const char* name = "yue.Button";
  static void BuildMetaTable(State* state, int index) {
    RawSet(state, index, "new", &New,
                         "settitle", &nu::Button::SetTitle,
                         "gettitle", &nu::Button::GetTitle,
                         "setchecked", &nu::Button::SetChecked,
                         "ischecked", &nu::Button::IsChecked);
  }
  static nu::Button* New(CallContext* context) {
    std::string title;
    if (To(context->state, 1, &title)) {
      return new nu::Button(title);
    } else if (GetType(context->state, 1) == LuaType::Table) {
      RawGetAndPop(context->state, 1, "title", &title);
      nu::Button::Type type = nu::Button::Normal;
      RawGetAndPop(context->state, 1, "type", &type);
      return new nu::Button(title, type);
    } else {
      context->has_error = true;
      Push(context->state, "Button must be created with string or table");
      return nullptr;
    }
  }
  static bool Index(State* state, const std::string& name) {
    return yue::SignalIndex(state, name, "onclick", &nu::Button::on_click);
  }
  static bool NewIndex(State* state, const std::string& name) {
    return yue::MemberNewIndex(state, name, "onclick", &nu::Button::on_click);
  }
};

template<>
struct Type<nu::Entry> {
  using base = nu::View;
  static constexpr const char* name = "yue.Entry";
  static void BuildMetaTable(State* state, int index) {
    RawSet(state, index, "new", &MetaTable<nu::Entry>::NewInstance<>,
                         "settext", &nu::Entry::SetText,
                         "gettext", &nu::Entry::GetText);
  }
  static bool Index(State* state, const std::string& name) {
    return yue::SignalIndex(state, name,
                            "onactivate", &nu::Entry::on_activate,
                            "ontextchange", &nu::Entry::on_text_change);
  }
  static bool NewIndex(State* state, const std::string& name) {
    return yue::MemberNewIndex(state, name,
                               "onactivate", &nu::Entry::on_activate,
                               "ontextchange", &nu::Entry::on_text_change);
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
           "addchildviewat", &AddChildViewAt,
           "removechildview", &nu::Container::RemoveChildView,
           "childcount", &nu::Container::child_count,
           "childat", &ChildAt);
  }
  // Transalte 1-based index to 0-based.
  static inline void AddChildViewAt(nu::Container* c, nu::View* view, int i) {
    c->AddChildViewAt(view, i - 1);
  }
  static inline nu::View* ChildAt(nu::Container* container, int i) {
    return container->child_at(i - 1);
  }
};

template<>
struct Type<nu::Scroll::Policy> {
  static constexpr const char* name = "yue.Scroll.Policy";
  static inline bool To(State* state, int index, nu::Scroll::Policy* out) {
    std::string policy;
    if (!lua::To(state, index, &policy))
      return false;
    if (policy == "automatic") {
      *out = nu::Scroll::Policy::Automatic;
      return true;
    } else if (policy == "always") {
      *out = nu::Scroll::Policy::Always;
      return true;
    } else if (policy == "never") {
      *out = nu::Scroll::Policy::Never;
      return true;
    } else {
      return false;
    }
  }
  static inline void Push(State* state, nu::Scroll::Policy policy) {
    if (policy == nu::Scroll::Policy::Always)
      lua::Push(state, "always");
    else if (policy == nu::Scroll::Policy::Never)
      lua::Push(state, "never");
    else
      lua::Push(state, "automatic");
  }
};

template<>
struct Type<nu::Scroll> {
  using base = nu::View;
  static constexpr const char* name = "yue.Scroll";
  static void BuildMetaTable(State* state, int index) {
    RawSet(state, index,
           "new", &MetaTable<nu::Scroll>::NewInstance<const nu::Size&>,
           "setscrollbarpolicy", &nu::Scroll::SetScrollBarPolicy,
           "getscrollbarpolicy", &nu::Scroll::GetScrollBarPolicy,
           "setcontentsize", &nu::Scroll::SetContentSize,
           "getcontentsize", &nu::Scroll::GetContentSize,
           "setcontentview", &nu::Scroll::SetContentView,
           "getcontentview", &nu::Scroll::GetContentView);
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
  static bool Index(State* state, const std::string& name) {
    return yue::SignalIndex(state, name, "onclose", &nu::Window::on_close);
  }
  static bool NewIndex(State* state, const std::string& name) {
    return yue::MemberNewIndex(state, name,
                               "onclose", &nu::Window::on_close,
                               "shouldclose", &nu::Window::should_close);
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
  static bool To(State* state, int index, nu::BoxLayout::Orientation* out) {
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
struct Type<nu::BoxLayout::AxisAlignment> {
  static constexpr const char* name = "yue.BoxLayout.AxisAlignment";
  static bool To(State* state, int index, nu::BoxLayout::AxisAlignment* out) {
    std::string alignment;
    if (!lua::To(state, index, &alignment))
      return false;
    if (alignment == "start") {
      *out = nu::BoxLayout::Start;
      return true;
    } else if (alignment == "center") {
      *out = nu::BoxLayout::Center;
      return true;
    } else if (alignment == "end") {
      *out = nu::BoxLayout::End;
      return true;
    } else if (alignment == "stretch") {
      *out = nu::BoxLayout::Stretch;
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
    RawSet(state, index, "new", &New,
                         "setflexat", &SetFlexAt,
                         "setorientation", &nu::BoxLayout::set_orientation);
  }
  static nu::BoxLayout* New(CallContext* context) {
    nu::BoxLayout::Orientation orientation = nu::BoxLayout::Vertical;
    if (To(context->state, 1, &orientation)) {
      return new nu::BoxLayout(orientation);
    } else if (GetType(context->state, 1) == LuaType::Table) {
      RawGetAndPop(context->state, 1, "orientation", &orientation);
      auto* box = new nu::BoxLayout(orientation);
      int n;
      if (RawGetAndPop(context->state, 1, "childspacing", &n))
        box->set_child_spacing(n);
      nu::Insets insets;
      if (RawGetAndPop(context->state, 1, "innerpadding", &insets))
        box->set_inner_padding(insets);
      nu::BoxLayout::AxisAlignment alignment;
      if (RawGetAndPop(context->state, 1, "mainaxisalignment", &alignment))
        box->set_main_axis_alignment(alignment);
      if (RawGetAndPop(context->state, 1, "crossaxisalignment", &alignment))
        box->set_cross_axis_alignment(alignment);
      return box;
    } else {
      context->has_error = true;
      Push(context->state, "BoxLayout must be created with string or table");
      return nullptr;
    }
  }
  // Transalte 1-based index to 0-based.
  static inline void SetFlexAt(nu::BoxLayout* layout, int flex, int i) {
    layout->SetFlexAt(flex, i - 1);
  }
};

}  // namespace lua

extern "C" int luaopen_yue_gui(lua::State* state) {
  // Manage the gui state in lua.
  void* memory = lua_newuserdata(state, sizeof(nu::State));
  new(memory) nu::State;
  lua::PushNewTable(state);
  lua::RawSet(state, -1, "__gc", lua::CFunction(lua::OnGC<nu::State>));
  lua::SetMetaTable(state, -2);

  // Put the gui state into registry, so it is alive through whole lua state.
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
  lua::Push(state, "Entry");
  lua::MetaTable<nu::Entry>::Push(state);
  lua_rawset(state, -3);
  lua::Push(state, "Label");
  lua::MetaTable<nu::Label>::Push(state);
  lua_rawset(state, -3);
  lua::Push(state, "Group");
  lua::MetaTable<nu::Group>::Push(state);
  lua_rawset(state, -3);
  lua::Push(state, "Scroll");
  lua::MetaTable<nu::Scroll>::Push(state);
  lua_rawset(state, -3);
  lua::Push(state, "FillLayout");
  lua::MetaTable<nu::FillLayout>::Push(state);
  lua_rawset(state, -3);
  lua::Push(state, "BoxLayout");
  lua::MetaTable<nu::BoxLayout>::Push(state);
  lua_rawset(state, -3);
  return 1;
}
