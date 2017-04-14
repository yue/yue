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
    lua::NewTable(state);
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
    lua::NewTable(state);
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
    lua::NewTable(state);
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
    lua::NewTable(state);
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
struct Type<nu::PointF> {
  static constexpr const char* name = "yue.Point";
  static inline void Push(State* state, const nu::PointF& p) {
    lua::NewTable(state);
    lua::RawSet(state, -1, "x", p.x(), "y", p.y());
  }
  static inline bool To(State* state, int index, nu::PointF* out) {
    float x, y;
    if (GetTop(state) - index == 1 && lua::To(state, index, &x, &y)) {
      *out = nu::PointF(x, y);
      return true;
    }
    if (GetType(state, index) != LuaType::Table)
      return false;
    if (!RawGetAndPop(state, index, "x", &x, "y", &y))
      return false;
    *out = nu::PointF(x, y);
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
    if (tmp.IsEmpty())
      return false;
    *out = tmp;
    return true;
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
    RawSetProperty(state, index, "onready", &nu::Lifetime::on_ready);
  }
};

template<>
struct Type<nu::App> {
  static constexpr const char* name = "yue.App";
  static void BuildMetaTable(State* state, int metatable) {
#if defined(OS_MACOSX)
    RawSet(state, metatable,
           "setapplicationmenu", &nu::App::SetApplicationMenu);
#endif
  }
};

template<>
struct Type<nu::Font> {
  static constexpr const char* name = "yue.Font";
  static void BuildMetaTable(State* state, int index) {
    RawSet(state, index,
           "create", &CreateOnHeap<nu::Font, const std::string&, float>,
           "default", &GetDefault,
           "getname", &nu::Font::GetName,
           "getsize", &nu::Font::GetSize);
  }
  static nu::Font* GetDefault() {
    return nu::State::GetCurrent()->GetDefaultFont();
  }
};

template<>
struct Type<nu::Canvas> {
  static constexpr const char* name = "yue.Canvas";
  static void BuildMetaTable(State* state, int index) {
    RawSet(state, index,
           "create", &CreateOnHeap<nu::Canvas, const nu::SizeF&, float>,
           "createformainscreen", &CreateOnHeap<nu::Canvas, const nu::SizeF&>,
           "getscalefactor", &nu::Canvas::GetScaleFactor,
           "getpainter", &nu::Canvas::GetPainter,
           "getsize", &nu::Canvas::GetSize);
  }
};

template<>
struct Type<nu::SystemColor> {
  static constexpr const char* name = "yue.SystemColor";
  static inline bool To(State* state, int index, nu::SystemColor* out) {
    std::string id;
    if (!lua::To(state, index, &id))
      return false;
    if (id == "text")
      *out = nu::SystemColor::Text;
    else
      return false;
    return true;
  }
};

template<>
struct Type<nu::Color> {
  static constexpr const char* name = "yue.Color";
  static inline void Push(State* state, nu::Color color) {
    lua::Push(state, color.value());
  }
  static inline bool To(State* state, int index, nu::Color* out) {
    // Direct value.
    if (GetType(state, index) == LuaType::Number) {
      uint32_t value;
      if (lua::To(state, index, &value)) {
        *out = nu::Color(value);
        return true;
      }
    }
    // String representation.
    std::string hex;
    if (!lua::To(state, index, &hex))
      return false;
    *out = nu::Color(hex);
    return true;
  }
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "getsystem", &nu::GetSystemColor,
           "rgb", &CreateOnStack<nu::Color, unsigned, unsigned, unsigned>,
           "argb", &CreateOnStack<nu::Color, unsigned, unsigned, unsigned,
                                  unsigned>);
  }
};

template<>
struct Type<nu::Image> {
  static constexpr const char* name = "yue.Image";
  static void BuildMetaTable(State* state, int index) {
    RawSet(state, index,
           "createfromfile", &CreateOnHeap<nu::Image, const nu::FilePath&>,
           "getsize", &nu::Image::GetSize,
           "getscalefactor", &nu::Image::GetScaleFactor);
  }
};

template<>
struct Type<nu::TextAlign> {
  static constexpr const char* name = "yue.TextAlign";
  static inline bool To(State* state, int value, nu::TextAlign* out) {
    base::StringPiece align;
    if (!lua::To(state, value, &align))
      return false;
    if (align == "start")
      *out = nu::TextAlign::Start;
    else if (align == "center")
      *out = nu::TextAlign::Center;
    else if (align == "end")
      *out = nu::TextAlign::End;
    else
      return false;
    return true;
  }
};

template<>
struct Type<nu::TextAttributes> {
  static constexpr const char* name = "yue.TextAttributes";
  static inline bool To(State* state, int index, nu::TextAttributes* out) {
    if (GetType(state, index) != LuaType::Table)
      return false;
    RawGetAndPop(state, index, "font", &out->font);
    RawGetAndPop(state, index, "color", &out->color);
    RawGetAndPop(state, index, "align", &out->align);
    RawGetAndPop(state, index, "valign", &out->valign);
    return true;
  }
};

template<>
struct Type<nu::TextMetrics> {
  static constexpr const char* name = "yue.TextMetrics";
  static inline void Push(State* state, const nu::TextMetrics& metrics) {
    NewTable(state, 0, 1);
    RawSet(state, -1, "size", metrics.size);
  }
};

template<>
struct Type<nu::Painter> {
  static constexpr const char* name = "yue.Painter";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "save", &nu::Painter::Save,
           "restore", &nu::Painter::Restore,
           "beginpath", &nu::Painter::BeginPath,
           "closepath", &nu::Painter::ClosePath,
           "moveto", &nu::Painter::MoveTo,
           "lineto", &nu::Painter::LineTo,
           "beziercurveto", &nu::Painter::BezierCurveTo,
           "arcto", &nu::Painter::ArcTo,
           "arc", &nu::Painter::Arc,
           "rect", &nu::Painter::Rect,
           "clip", &nu::Painter::Clip,
           "cliprect", &nu::Painter::ClipRect,
           "translate", &nu::Painter::Translate,
           "rotate", &nu::Painter::Rotate,
           "scale", &nu::Painter::Scale,
           "setcolor", &nu::Painter::SetColor,
           "setstrokecolor", &nu::Painter::SetStrokeColor,
           "setfillcolor", &nu::Painter::SetFillColor,
           "setlinewidth", &nu::Painter::SetLineWidth,
           "stroke", &nu::Painter::Stroke,
           "fill", &nu::Painter::Fill,
           "strokerect", &nu::Painter::StrokeRect,
           "fillrect", &nu::Painter::FillRect,
           "drawimage", &nu::Painter::DrawImage,
           "drawimagefromrect", &nu::Painter::DrawImageFromRect,
           "drawcanvas", &nu::Painter::DrawCanvas,
           "drawcanvasfromrect", &nu::Painter::DrawCanvasFromRect,
           "measuretext", &nu::Painter::MeasureText,
           "drawtext", &nu::Painter::DrawText);
  }
};

template<>
struct Type<nu::EventType> {
  static constexpr const char* name = "yue.EventType";
  static inline void Push(State* state, nu::EventType type) {
    switch (type) {
      case nu::EventType::MouseDown:
        lua::Push(state, "mousedown");
      case nu::EventType::MouseUp:
        lua::Push(state, "mouseup");
      default:
        NOTREACHED();
        lua::Push(state, nullptr);
    }
  }
};

template<>
struct Type<nu::Event> {
  static constexpr const char* name = "yue.Event";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "maskshift", static_cast<int>(nu::MASK_SHIFT),
           "maskcontrol", static_cast<int>(nu::MASK_CONTROL),
           "maskalt", static_cast<int>(nu::MASK_ALT),
           "maskmeta", static_cast<int>(nu::MASK_META));
  }
  // Used by subclasses.
  static void SetEventProperties(State* state,
                                 int index,
                                 const nu::Event* event) {
    RawSet(state, index,
           "type", event->type,
           "modifiers", event->modifiers,
           "timestamp", event->timestamp);
  }
};

template<>
struct Type<nu::MouseEvent> {
  using base = nu::Event;
  static constexpr const char* name = "yue.MouseEvent";
  static inline void Push(State* state, const nu::MouseEvent& event) {
    NewTable(state);
    Type<nu::Event>::SetEventProperties(state, -1, &event);
    RawSet(state, -1,
           "button", event.button,
           "position", event.position);
  }
};

template<>
struct Type<nu::MenuBase> {
  static constexpr const char* name = "yue.MenuBase";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "append", &nu::MenuBase::Append,
           "insert", &Insert,
           "itemcount", &nu::MenuBase::ItemCount,
           "itemat", &ItemAt);
  }
  static inline void Insert(nu::MenuBase* menu, nu::MenuItem* item, int i) {
    menu->Insert(item, i - 1);
  }
  static inline nu::MenuItem* ItemAt(nu::MenuBase* menu, int i) {
    return menu->ItemAt(i - 1);
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
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "create", &Create);
  }
  static nu::MenuBar* Create(CallContext* context) {
    nu::MenuBar* menu = new nu::MenuBar;
    Type<nu::MenuBase>::ReadMembers(context->state, menu);
    return menu;
  }
};

template<>
struct Type<nu::Menu> {
  using base = nu::MenuBase;
  static constexpr const char* name = "yue.Menu";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "create", &Create,
           "popup", &nu::Menu::Popup);
  }
  static nu::Menu* Create(CallContext* context) {
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
    if (type == "label")
      *out = nu::MenuItem::Type::Label;
    else if (type == "checkbox")
      *out = nu::MenuItem::Type::CheckBox;
    else if (type == "radio")
      *out = nu::MenuItem::Type::Radio;
    else if (type == "separator")
      *out = nu::MenuItem::Type::Separator;
    else if (type == "submenu")
      *out = nu::MenuItem::Type::Submenu;
    else
      return false;
    return true;
  }
};

template<>
struct Type<nu::MenuItem> {
  static constexpr const char* name = "yue.MenuItem";
  static void BuildMetaTable(State* state, int index) {
    RawSet(state, index,
           "create", &Create,
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
    RawSetProperty(state, index,
                   "onclick", &nu::MenuItem::on_click);
  }
  static nu::MenuItem* Create(CallContext* context) {
    nu::MenuItem::Type type = nu::MenuItem::Type::Label;
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
      if (!item) item = new nu::MenuItem(nu::MenuItem::Type::CheckBox);
      item->SetChecked(b);
    }
    nu::Menu* submenu = nullptr;
    if (RawGetAndPop(context->state, 1, "submenu", &submenu)) {
      if (!item) item = new nu::MenuItem(nu::MenuItem::Type::Submenu);
      item->SetSubmenu(submenu);
    }
    if (!item)  // can not deduce type from property, assuming Label item.
      item = new nu::MenuItem(nu::MenuItem::Type::Label);
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
    RawGetAndPop(context->state, 1, "onclick", &item->on_click);
    return item;
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
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "create", &CreateOnHeap<nu::Window, const nu::Window::Options&>,
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
           "setmenu", &nu::Window::SetMenu,
           "getmenu", &nu::Window::GetMenu,
#endif
           "setbackgroundcolor", &nu::Window::SetBackgroundColor);
    RawSetProperty(state, metatable,
                   "onclose", &nu::Window::on_close,
                   "shouldclose", &nu::Window::should_close);
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
           "getparent", &nu::View::GetParent);
    RawSetProperty(state, index,
                   "onmousedown", &nu::View::on_mouse_down,
                   "onmouseup", &nu::View::on_mouse_up);
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
struct Type<nu::Container> {
  using base = nu::View;
  static constexpr const char* name = "yue.Container";
  static void BuildMetaTable(State* state, int index) {
    RawSet(state, index,
           "create", &CreateOnHeap<nu::Container>,
           "getpreferredsize", &nu::Container::GetPreferredSize,
           "getpreferredwidthforheight",
           &nu::Container::GetPreferredWidthForHeight,
           "getpreferredheightforwidth",
           &nu::Container::GetPreferredHeightForWidth,
           "addchildview", &nu::Container::AddChildView,
           "addchildviewat", &AddChildViewAt,
           "removechildview", &nu::Container::RemoveChildView,
           "childcount", &nu::Container::ChildCount,
           "childat", &ChildAt);
    RawSetProperty(state, index, "ondraw", &nu::Container::on_draw);
  }
  // Transalte 1-based index to 0-based.
  static inline void AddChildViewAt(nu::Container* c, nu::View* view, int i) {
    c->AddChildViewAt(view, i - 1);
  }
  static inline nu::View* ChildAt(nu::Container* container, int i) {
    return container->ChildAt(i - 1);
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
      *out = nu::Button::Type::Normal;
      return true;
    } else if (type == "checkbox") {
      *out = nu::Button::Type::CheckBox;
      return true;
    } else if (type == "radio") {
      *out = nu::Button::Type::Radio;
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
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "create", &Create,
           "settitle", &nu::Button::SetTitle,
           "gettitle", &nu::Button::GetTitle,
           "setchecked", &nu::Button::SetChecked,
           "ischecked", &nu::Button::IsChecked);
    RawSetProperty(state, metatable, "onclick", &nu::Button::on_click);
  }
  static nu::Button* Create(CallContext* context) {
    std::string title;
    if (To(context->state, 1, &title)) {
      return new nu::Button(title);
    } else if (GetType(context->state, 1) == LuaType::Table) {
      RawGetAndPop(context->state, 1, "title", &title);
      nu::Button::Type type = nu::Button::Type::Normal;
      RawGetAndPop(context->state, 1, "type", &type);
      return new nu::Button(title, type);
    } else {
      context->has_error = true;
      Push(context->state, "Button must be created with string or table");
      return nullptr;
    }
  }
};

template<>
struct Type<nu::Entry> {
  using base = nu::View;
  static constexpr const char* name = "yue.Entry";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "create", &CreateOnHeap<nu::Entry>,
           "settext", &nu::Entry::SetText,
           "gettext", &nu::Entry::GetText);
    RawSetProperty(state, metatable,
                   "onactivate", &nu::Entry::on_activate,
                   "ontextchange", &nu::Entry::on_text_change);
  }
};

template<>
struct Type<nu::Label> {
  using base = nu::View;
  static constexpr const char* name = "yue.Label";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "create", &CreateOnHeap<nu::Label, const std::string&>,
           "settext", &nu::Label::SetText,
           "gettext", &nu::Label::GetText);
  }
};

template<>
struct Type<nu::Progress> {
  using base = nu::View;
  static constexpr const char* name = "yue.Progress";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "create", &CreateOnHeap<nu::Progress>,
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
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "create", &CreateOnHeap<nu::Group, const std::string&>,
           "setcontentview", &nu::Group::SetContentView,
           "getcontentview", &nu::Group::GetContentView,
           "settitle", &nu::Group::SetTitle,
           "gettitle", &nu::Group::GetTitle);
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
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "create", &CreateOnHeap<nu::Scroll>,
           "setscrollbarpolicy", &nu::Scroll::SetScrollbarPolicy,
           "getscrollbarpolicy", &nu::Scroll::GetScrollbarPolicy,
           "setcontentsize", &nu::Scroll::SetContentSize,
           "getcontentsize", &nu::Scroll::GetContentSize,
           "setcontentview", &nu::Scroll::SetContentView,
           "getcontentview", &nu::Scroll::GetContentView);
  }
};

#if defined(OS_MACOSX)
template<>
struct Type<nu::Vibrant> {
  using base = nu::Container;
  static constexpr const char* name = "yue.Vibrant";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "create", &CreateOnHeap<nu::Vibrant>);
  }
};
#endif

}  // namespace lua

extern "C" int luaopen_yue_gui(lua::State* state) {
  lua::NewTable(state);
  lua::RawSet(state, -1,
              // Classes.
              "Lifetime",  lua::MetaTable<nu::Lifetime>(),
              "App",       lua::MetaTable<nu::App>(),
              "Font",      lua::MetaTable<nu::Font>(),
              "Canvas",    lua::MetaTable<nu::Canvas>(),
              "Color",     lua::MetaTable<nu::Color>(),
              "Image",     lua::MetaTable<nu::Image>(),
              "Painter",   lua::MetaTable<nu::Painter>(),
              "Event",     lua::MetaTable<nu::Event>(),
              "MenuBar",   lua::MetaTable<nu::MenuBar>(),
              "Menu",      lua::MetaTable<nu::Menu>(),
              "MenuItem",  lua::MetaTable<nu::MenuItem>(),
              "Window",    lua::MetaTable<nu::Window>(),
              "Container", lua::MetaTable<nu::Container>(),
              "Button",    lua::MetaTable<nu::Button>(),
              "Entry",     lua::MetaTable<nu::Entry>(),
              "Label",     lua::MetaTable<nu::Label>(),
              "Progress",  lua::MetaTable<nu::Progress>(),
              "Group",     lua::MetaTable<nu::Group>(),
              "Scroll",    lua::MetaTable<nu::Scroll>(),
#if defined(OS_MACOSX)
              "Vibrant",   lua::MetaTable<nu::Vibrant>(),
#endif
              // Properties.
              "lifetime", nu::Lifetime::GetCurrent(),
              "app",      nu::State::GetCurrent()->GetApp());
  return 1;
}
