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
    float width, height;
    if (GetTop(state) - index == 1 && lua::To(state, index, &width, &height)) {
      *out = nu::Size(width, height);
      return true;
    }
    if (GetType(state, index) != LuaType::Table)
      return false;
    if (!RawGetAndPop(state, index, "width", &width, "height", &height))
      return false;
    *out = nu::Size(width, height);
    return true;
  }
};

template<>
struct Type<nu::SizeF> {
  static constexpr const char* name = "yue.Size";
  static inline void Push(State* state, const nu::SizeF& size) {
    lua::PushNewTable(state);
    lua::RawSet(state, -1, "width", size.width(), "height", size.height());
  }
  static inline bool To(State* state, int index, nu::SizeF* out) {
    float width, height;
    if (GetTop(state) - index == 1 && lua::To(state, index, &width, &height)) {
      *out = nu::SizeF(width, height);
      return true;
    }
    if (GetType(state, index) != LuaType::Table)
      return false;
    if (!RawGetAndPop(state, index, "width", &width, "height", &height))
      return false;
    *out = nu::SizeF(width, height);
    return true;
  }
};

template<>
struct Type<nu::RectF> {
  static constexpr const char* name = "yue.Rect";
  static inline void Push(State* state, const nu::RectF& rect) {
    lua::PushNewTable(state);
    lua::RawSet(state, -1,
                "x", rect.x(), "y", rect.y(),
                "width", rect.width(), "height", rect.height());
  }
  static inline bool To(State* state, int index, nu::RectF* out) {
    float x, y, width, height;
    if (GetTop(state) - index == 3 &&
        lua::To(state, index, &x, &y, &width, &height)) {
      *out = nu::RectF(x, y, width, height);
      return true;
    }
    if (GetType(state, index) != LuaType::Table)
      return false;
    if (!RawGetAndPop(state, index,
                      "x", &x, "y", &y, "width", &width, "height", &height))
      return false;
    *out = nu::RectF(x, y, width, height);
    return true;
  }
};

template<>
struct Type<nu::Vector2dF> {
  static constexpr const char* name = "yue.Vector2d";
  static inline void Push(State* state, const nu::Vector2dF& vec) {
    lua::PushNewTable(state);
    lua::RawSet(state, -1, "x", vec.x(), "y", vec.y());
  }
  static inline bool To(State* state, int index, nu::Vector2dF* out) {
    float x, y;
    if (GetTop(state) - index == 1 && lua::To(state, index, &x, &y)) {
      *out = nu::Vector2dF(x, y);
      return true;
    }
    if (GetType(state, index) != LuaType::Table)
      return false;
    if (!RawGetAndPop(state, index, "x", &x, "y", &y))
      return false;
    *out = nu::Vector2dF(x, y);
    return true;
  }
};

template<>
struct Type<nu::Color> {
  static constexpr const char* name = "yue.Color";
  static inline bool To(State* state, int index, nu::Color* out) {
    std::string hex;
    if (!lua::To(state, index, &hex))
      return false;
    *out = nu::Color(hex);
    return true;
  }
};

template<>
struct Type<nu::Accelerator> {
  static constexpr const char* name = "yue.Accelerator";
  static inline bool To(State* state, int index, nu::Accelerator* out) {
    std::string description;
    if (!lua::To(state, index, &description))
      return false;
    nu::Accelerator tmp(description);
    if (tmp.empty())
      return false;
    *out = tmp;
    return true;
  }
};

template<>
struct Type<nu::View> {
  static constexpr const char* name = "yue.View";
  static void BuildMetaTable(State* state, int index) {
    RawSet(state, index,
           "setbackgroundcolor", &nu::View::SetBackgroundColor,
           "setstyle", &SetStyle,
           "printstyle", &nu::View::PrintStyle,
           "layout", &nu::View::Layout,
           "setbounds", &nu::View::SetBounds,
           "getbounds", &nu::View::GetBounds,
           "setvisible", &nu::View::SetVisible,
           "isvisible", &nu::View::IsVisible,
           "getparent", &nu::View::parent);
  }
  static void SetStyle(CallContext* context, nu::View* view) {
    if (GetType(context->state, 2) != LuaType::Table) {
      context->has_error = true;
      Push(context->state, "first arg must be table");
      return;
    }
    Push(context->state, nullptr);
    while (lua_next(context->state, 2) != 0) {
      std::string key, value;
      To(context->state, -2, &key, &value);
      view->SetStyle(key, value);
      PopAndIgnore(context->state, 1);
    }
    // Refresh the view.
    view->Layout();
  }
};

template<>
struct Type<nu::App> {
  static constexpr const char* name = "yue.App";
  static void BuildMetaTable(State* state, int index) {
#if defined(OS_MACOSX)
    RawSet(state, index,
           "setapplicationmenu", &nu::App::SetApplicationMenu);
#endif
  }
};

template<>
struct Type<nu::Lifetime> {
  static constexpr const char* name = "yue.Lifetime";
  static void BuildMetaTable(State* state, int index) {
    RawSet(state, index,
           "run", &nu::Lifetime::Run,
           "quit", &nu::Lifetime::Quit,
           "post", &nu::Lifetime::PostTask,
           "postdelayed", &nu::Lifetime::PostDelayedTask);
  }
  static int NewIndex(State* state) {
    std::string name;
    if (!To(state, 2, &name))
      return 0;
    return yue::MemberNewIndex(state, name, "onready", &nu::Lifetime::on_ready);
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
  static int Index(State* state) {
    std::string name;
    if (!To(state, 2, &name))
      return 0;
    return yue::SignalIndex(state, name, "onclick", &nu::Button::on_click);
  }
  static int NewIndex(State* state) {
    std::string name;
    if (!To(state, 2, &name))
      return 0;
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
  static int Index(State* state) {
    std::string name;
    if (!To(state, 2, &name))
      return 0;
    return yue::SignalIndex(state, name,
                            "onactivate", &nu::Entry::on_activate,
                            "ontextchange", &nu::Entry::on_text_change);
  }
  static int NewIndex(State* state) {
    std::string name;
    if (!To(state, 2, &name))
      return 0;
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
struct Type<nu::Progress> {
  using base = nu::View;
  static constexpr const char* name = "yue.Progress";
  static void BuildMetaTable(State* state, int index) {
    RawSet(state, index, "new", &MetaTable<nu::Progress>::NewInstance<>,
                         "setvalue", &nu::Progress::SetValue,
                         "getvalue", &nu::Progress::GetValue,
                         "setindeterminate", &nu::Progress::SetIndeterminate,
                         "isindeterminate", &nu::Progress::IsIndeterminate);
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
           "getpreferredsize", &nu::Container::GetPreferredSize,
           "getpreferredwidthforheight",
           &nu::Container::GetPreferredWidthForHeight,
           "getpreferredheightforwidth",
           &nu::Container::GetPreferredHeightForWidth,
           "addchildview", &nu::Container::AddChildView,
           "addchildviewat", &AddChildViewAt,
           "removechildview", &nu::Container::RemoveChildView,
           "childcount", &nu::Container::child_count,
           "childat", &ChildAt);
  }
  static int Index(State* state) {
    std::string name;
    if (!To(state, 2, &name))
      return 0;
    return yue::SignalIndex(state, name, "ondraw", &nu::Container::on_draw);
  }
  static int NewIndex(State* state) {
    std::string name;
    if (!To(state, 2, &name))
      return 0;
    return yue::MemberNewIndex(state, name, "ondraw", &nu::Container::on_draw);
  }
  // Transalte 1-based index to 0-based.
  static inline void AddChildViewAt(nu::Container* c, nu::View* view, int i) {
    c->AddChildViewAt(view, i - 1);
  }
  static inline nu::View* ChildAt(nu::Container* container, int i) {
    return container->child_at(i - 1);
  }
};

#if defined(OS_MACOSX)
template<>
struct Type<nu::Vibrant> {
  using base = nu::Container;
  static constexpr const char* name = "yue.Vibrant";
  static void BuildMetaTable(State* state, int index) {
    RawSet(state, index, "new", &MetaTable<nu::Vibrant>::NewInstance<>);
  }
  static int Index(State* state) {
    return Type<base>::Index(state);
  }
  static int NewIndex(State* state) {
    return Type<base>::NewIndex(state);
  }
};
#endif

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
           "new", &MetaTable<nu::Scroll>::NewInstance<>,
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
           "new",
           &MetaTable<nu::Window>::NewInstance<const nu::Window::Options&>,
           "close", &nu::Window::Close,
           "setcontentbounds", &nu::Window::SetContentBounds,
           "getcontentbounds", &nu::Window::GetContentBounds,
           "setbounds", &nu::Window::SetBounds,
           "getbounds", &nu::Window::GetBounds,
           "setcontentview", &nu::Window::SetContentView,
           "getcontentview", &nu::Window::GetContentView,
           "setvisible", &nu::Window::SetVisible,
           "isvisible", &nu::Window::IsVisible,
#if defined(OS_WIN) || defined(OS_LINUX)
           "setmenubar", &nu::Window::SetMenuBar,
           "getmenubar", &nu::Window::GetMenuBar,
#endif
           "setbackgroundcolor", &nu::Window::SetBackgroundColor);
  }
  static int Index(State* state) {
    std::string name;
    if (!To(state, 2, &name))
      return 0;
    return yue::SignalIndex(state, name, "onclose", &nu::Window::on_close);
  }
  static int NewIndex(State* state) {
    std::string name;
    if (!To(state, 2, &name))
      return 0;
    return yue::MemberNewIndex(state, name,
                               "onclose", &nu::Window::on_close,
                               "shouldclose", &nu::Window::should_close);
  }
};

template<>
struct Type<nu::Font> {
  static constexpr const char* name = "yue.Font";
  static void BuildMetaTable(State* state, int index) {
    RawSet(state, index,
           "new", &MetaTable<nu::Font>::NewInstance<const std::string&, int>,
           "default", &GetDefault,
           "getname", &nu::Font::GetName,
           "getsize", &nu::Font::GetSize);
  }
  static nu::Font* GetDefault() {
    return nu::State::current()->GetDefaultFont();
  }
};

template<>
struct Type<nu::Image> {
  static constexpr const char* name = "yue.Image";
  static void BuildMetaTable(State* state, int index) {
    RawSet(state, index,
           "newfromfile", &MetaTable<nu::Image>::NewInstance<const nu::String&>,
           "getsize", &nu::Image::GetSize);
  }
};

template<>
struct Type<nu::Painter::CombineMode> {
  static constexpr const char* name = "yue.Painter.CombineMode";
  static inline bool To(State* state, int index,
                        nu::Painter::CombineMode* out) {
    std::string mode;
    if (!lua::To(state, index, &mode))
      return false;
    if (mode == "replace")
      *out = nu::Painter::CombineMode::Replace;
    else if (mode == "intersect")
      *out = nu::Painter::CombineMode::Intersect;
    else if (mode == "union")
      *out = nu::Painter::CombineMode::Union;
    else if (mode == "exclude")
      *out = nu::Painter::CombineMode::Exclude;
    else
      return false;
    return true;
  }
};

template<>
struct Type<nu::Painter> {
  static constexpr const char* name = "yue.Painter";
  static void BuildMetaTable(State* state, int index) {
    RawSet(state, index,
           "save", &nu::Painter::Save,
           "restore", &nu::Painter::Restore,
           "cliprect", &nu::Painter::ClipRect,
           "translate", &nu::Painter::Translate,
           "setcolor", &nu::Painter::SetColor,
           "drawrect", &nu::Painter::DrawRect,
           "fillrect", &nu::Painter::FillRect,
           "drawtext", &nu::Painter::DrawText);
  }
};

template<>
struct Type<nu::MenuBase> {
  static constexpr const char* name = "yue.MenuBase";
  static void BuildMetaTable(State* state, int index) {
    RawSet(state, index, "append", &nu::MenuBase::Append,
                         "insert", &Insert,
                         "itemcount", &nu::MenuBase::item_count,
                         "itemat", &ItemAt);
  }
  static inline void Insert(nu::MenuBase* menu, nu::MenuItem* item, int i) {
    menu->Insert(item, i - 1);
  }
  static inline nu::MenuItem* ItemAt(nu::MenuBase* menu, int i) {
    return menu->item_at(i - 1);
  }
  // Used by subclasses.
  static void ReadMembers(State* state, nu::MenuBase* menu) {
    if (GetType(state, 1) != LuaType::Table)
      return;
    Push(state, nullptr);
    while (lua_next(state, 1) != 0) {
      nu::MenuItem* item;
      if (Pop(state, &item))
        menu->Append(item);
      else
        PopAndIgnore(state, 1);
    }
  }
};

template<>
struct Type<nu::MenuBar> {
  using base = nu::MenuBase;
  static constexpr const char* name = "yue.MenuBar";
  static void BuildMetaTable(State* state, int index) {
    RawSet(state, index, "new", &New);
  }
  static nu::MenuBar* New(CallContext* context) {
    nu::MenuBar* menu = new nu::MenuBar;
    Type<nu::MenuBase>::ReadMembers(context->state, menu);
    return menu;
  }
};

template<>
struct Type<nu::Menu> {
  using base = nu::MenuBase;
  static constexpr const char* name = "yue.Menu";
  static void BuildMetaTable(State* state, int index) {
    RawSet(state, index, "new", &New, "popup", &nu::Menu::Popup);
  }
  static nu::Menu* New(CallContext* context) {
    nu::Menu* menu = new nu::Menu;
    Type<nu::MenuBase>::ReadMembers(context->state, menu);
    return menu;
  }
};

template<>
struct Type<nu::MenuItem::Type> {
  static constexpr const char* name = "yue.MenuItem.Type";
  static bool To(State* state, int index, nu::MenuItem::Type* out) {
    std::string type;
    if (!lua::To(state, index, &type))
      return false;
    if (type == "label") {
      *out = nu::MenuItem::Label;
      return true;
    } else if (type == "checkbox") {
      *out = nu::MenuItem::CheckBox;
      return true;
    } else if (type == "radio") {
      *out = nu::MenuItem::Radio;
      return true;
    } else if (type == "separator") {
      *out = nu::MenuItem::Separator;
      return true;
    } else if (type == "submenu") {
      *out = nu::MenuItem::Submenu;
      return true;
    } else {
      return false;
    }
  }
};

template<>
struct Type<nu::MenuItem> {
  static constexpr const char* name = "yue.MenuItem";
  static void BuildMetaTable(State* state, int index) {
    RawSet(state, index,
           "new", &New,
           "setlabel", &nu::MenuItem::SetLabel,
           "getlabel", &nu::MenuItem::GetLabel,
           "setchecked", &nu::MenuItem::SetChecked,
           "ischecked", &nu::MenuItem::IsChecked,
           "setenabled", &nu::MenuItem::SetEnabled,
           "isenabled", &nu::MenuItem::IsEnabled,
           "setvisible", &nu::MenuItem::SetVisible,
           "isvisible", &nu::MenuItem::IsVisible,
           "setsubmenu", &nu::MenuItem::SetSubmenu,
           "getsubmenu", &nu::MenuItem::GetSubmenu,
           "setaccelerator", &nu::MenuItem::SetAccelerator);
  }
  static int Index(State* state) {
    std::string name;
    if (!To(state, 2, &name))
      return 0;
    return yue::SignalIndex(state, name, "onclick", &nu::MenuItem::on_click);
  }
  static int NewIndex(State* state) {
    std::string name;
    if (!To(state, 2, &name))
      return 0;
    return yue::MemberNewIndex(state, name, "onclick", &nu::MenuItem::on_click);
  }
  static nu::MenuItem* New(CallContext* context) {
    nu::MenuItem::Type type = nu::MenuItem::Label;
    if (lua::To(context->state, 1, &type) ||  // 'type'
        GetType(context->state, 1) != LuaType::Table)  // {type='type'}
      return new nu::MenuItem(type);
    // Use label unless "type" is specified.
    nu::MenuItem* item = nullptr;
    if (RawGetAndPop(context->state, 1, "type", &type))
      item = new nu::MenuItem(type);
    // Read table fields and set attributes.
    bool b = false;
    if (RawGetAndPop(context->state, 1, "checked", &b)) {
      if (!item) item = new nu::MenuItem(nu::MenuItem::CheckBox);
      item->SetChecked(b);
    }
    nu::Menu* submenu = nullptr;
    if (RawGetAndPop(context->state, 1, "submenu", &submenu)) {
      if (!item) item = new nu::MenuItem(nu::MenuItem::Submenu);
      item->SetSubmenu(submenu);
    }
    if (!item)  // can not deduce type from property, assuming Label item.
      item = new nu::MenuItem(nu::MenuItem::Label);
    if (RawGetAndPop(context->state, 1, "visible", &b))
      item->SetVisible(b);
    if (RawGetAndPop(context->state, 1, "enabled", &b))
      item->SetEnabled(b);
    std::string label;
    if (RawGetAndPop(context->state, 1, "label", &label))
      item->SetLabel(label);
    nu::Accelerator accelerator;
    if (RawGetAndPop(context->state, 1, "accelerator", &accelerator))
      item->SetAccelerator(accelerator);
    return item;
  }
};

}  // namespace lua

namespace {

template<typename T>
void SetTable(lua::State* state, const char* name) {
  lua::Push(state, name);
  lua::MetaTable<T>::Push(state);
  lua_rawset(state, -3);
}

}  // namespace

extern "C" int luaopen_yue_gui(lua::State* state) {
  // Populate the table with GUI elements.
  lua::PushNewTable(state);
  SetTable<nu::App>(state, "App");
  SetTable<nu::Lifetime>(state, "Lifetime");
  SetTable<nu::Window>(state, "Window");
  SetTable<nu::Container>(state, "Container");
  SetTable<nu::Button>(state, "Button");
  SetTable<nu::Entry>(state, "Entry");
  SetTable<nu::Label>(state, "Label");
  SetTable<nu::Progress>(state, "Progress");
  SetTable<nu::Group>(state, "Group");
  SetTable<nu::Scroll>(state, "Scroll");
  SetTable<nu::Font>(state, "Font");
  SetTable<nu::Image>(state, "Image");
  SetTable<nu::Painter>(state, "Painter");
  SetTable<nu::MenuBar>(state, "MenuBar");
  SetTable<nu::Menu>(state, "Menu");
  SetTable<nu::MenuItem>(state, "MenuItem");
#if defined(OS_MACOSX)
  SetTable<nu::Vibrant>(state, "Vibrant");
#endif

  // Create APIs that only available as instances.
  lua::Push(state, "app");
  lua::MetaTable<nu::App>::PushNewWrapper(state, nu::State::current()->app());
  lua_rawset(state, -3);
  lua::Push(state, "lifetime");
  lua::MetaTable<nu::Lifetime>::PushNewWrapper(state, nu::Lifetime::current());
  lua_rawset(state, -3);
  return 1;
}
