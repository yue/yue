// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "lua_yue/binding_gui.h"

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "base/command_line.h"
#include "lua_yue/binding_signal.h"
#include "lua_yue/binding_values.h"
#include "nativeui/nativeui.h"

namespace lua {

template<>
struct Type<base::FilePath> {
  static constexpr const char* name = "yue.FilePath";
  static inline void Push(State* state, const base::FilePath& value) {
    return lua::Push(state, value.value());;
  }
  static inline bool To(State* state, int index, base::FilePath* out) {
    base::FilePath::StringType str;
    if (!lua::To(state, index, &str))
      return false;
    *out = base::FilePath(str);
    return true;
  }
};

template<>
struct Type<nu::Buffer> {
  static constexpr const char* name = "yue.Buffer";
  static inline void Push(State* state, const nu::Buffer& value) {
    lua_pushlstring(state, static_cast<char*>(value.content()), value.size());
  }
  static inline bool To(State* state, int index, nu::Buffer* out) {
    if (GetType(state, index) != LuaType::String)
      return false;
    size_t size = 0;
    const char* str = lua_tolstring(state, index, &size);
    if (!str)
      return false;
    // We are assuming the Buffer is consumed immediately.
    *out = nu::Buffer::Wrap(str, size);
    return true;
  }
};

template<>
struct Type<nu::Size> {
  static constexpr const char* name = "yue.Size";
  static inline void Push(State* state, const nu::Size& size) {
    lua::NewTable(state);
    lua::RawSet(state, -1, "width", size.width(), "height", size.height());
  }
  static inline bool To(State* state, int index, nu::Size* out) {
    float width = 0, height = 0;
    if (GetTop(state) - index == 1 && lua::To(state, index, &width, &height)) {
      *out = nu::Size(width, height);
      return true;
    }
    if (GetType(state, index) != LuaType::Table)
      return false;
    RawGetAndPop(state, index, "width", &width, "height", &height);
    *out = nu::Size(width, height);
    return true;
  }
};

template<>
struct Type<nu::SizeF> {
  static constexpr const char* name = "yue.SizeF";
  static inline void Push(State* state, const nu::SizeF& size) {
    lua::NewTable(state);
    lua::RawSet(state, -1, "width", size.width(), "height", size.height());
  }
  static inline bool To(State* state, int index, nu::SizeF* out) {
    float width = 0, height = 0;
    if (GetTop(state) - index == 1 && lua::To(state, index, &width, &height)) {
      *out = nu::SizeF(width, height);
      return true;
    }
    if (GetType(state, index) != LuaType::Table)
      return false;
    RawGetAndPop(state, index, "width", &width, "height", &height);
    *out = nu::SizeF(width, height);
    return true;
  }
};

template<>
struct Type<nu::RectF> {
  static constexpr const char* name = "yue.RectF";
  static inline void Push(State* state, const nu::RectF& rect) {
    lua::NewTable(state);
    lua::RawSet(state, -1,
                "x", rect.x(), "y", rect.y(),
                "width", rect.width(), "height", rect.height());
  }
  static inline bool To(State* state, int index, nu::RectF* out) {
    float x = 0, y = 0, width = 0, height = 0;
    if (GetTop(state) - index == 3 &&
        lua::To(state, index, &x, &y, &width, &height)) {
      *out = nu::RectF(x, y, width, height);
      return true;
    }
    if (GetType(state, index) != LuaType::Table)
      return false;
    RawGetAndPop(state, index, "x", &x, "y", &y);
    RawGetAndPop(state, index, "width", &width, "height", &height);
    *out = nu::RectF(x, y, width, height);
    return true;
  }
};

template<>
struct Type<nu::Vector2dF> {
  static constexpr const char* name = "yue.Vector2dF";
  static inline void Push(State* state, const nu::Vector2dF& vec) {
    lua::NewTable(state);
    lua::RawSet(state, -1, "x", vec.x(), "y", vec.y());
  }
  static inline bool To(State* state, int index, nu::Vector2dF* out) {
    float x = 0, y = 0;
    if (GetTop(state) - index == 1 && lua::To(state, index, &x, &y)) {
      *out = nu::Vector2dF(x, y);
      return true;
    }
    if (GetType(state, index) != LuaType::Table)
      return false;
    RawGetAndPop(state, index, "x", &x, "y", &y);
    *out = nu::Vector2dF(x, y);
    return true;
  }
};

template<>
struct Type<nu::PointF> {
  static constexpr const char* name = "yue.PointF";
  static inline void Push(State* state, const nu::PointF& p) {
    lua::NewTable(state);
    lua::RawSet(state, -1, "x", p.x(), "y", p.y());
  }
  static inline bool To(State* state, int index, nu::PointF* out) {
    float x = 0, y = 0;
    if (GetTop(state) - index == 1 && lua::To(state, index, &x, &y)) {
      *out = nu::PointF(x, y);
      return true;
    }
    if (GetType(state, index) != LuaType::Table)
      return false;
    RawGetAndPop(state, index, "x", &x, "y", &y);
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
#if defined(OS_MACOSX)
    RawSetProperty(state, index,
                   "onready", &nu::Lifetime::on_ready,
                   "onopen", &nu::Lifetime::on_open,
                   "onactivate", &nu::Lifetime::on_activate);
#endif
  }
};

template<>
struct Type<nu::MessageLoop> {
  static constexpr const char* name = "yue.MessageLoop";
  static void BuildMetaTable(State* state, int index) {
    RawSet(state, index,
           "run", &nu::MessageLoop::Run,
           "quit", &nu::MessageLoop::Quit,
           "posttask", &nu::MessageLoop::PostTask,
           "postdelayedtask", &nu::MessageLoop::PostDelayedTask);
  }
};

template<>
struct Type<nu::App::ThemeColor> {
  static constexpr const char* name = "yue.ThemeColor";
  static inline bool To(State* state, int index, nu::App::ThemeColor* out) {
    std::string id;
    if (!lua::To(state, index, &id))
      return false;
    if (id == "text")
      *out = nu::App::ThemeColor::Text;
    else if (id == "disabled-text")
      *out = nu::App::ThemeColor::DisabledText;
    else
      return false;
    return true;
  }
};

template<>
struct Type<nu::App> {
  static constexpr const char* name = "yue.App";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
#if defined(OS_MACOSX)
           "setapplicationmenu",
           RefMethod(&nu::App::SetApplicationMenu, RefType::Reset, "appmenu"),
           "setdockbadgelabel", &nu::App::SetDockBadgeLabel,
           "getdockbadgelabel", &nu::App::GetDockBadgeLabel,
#endif
           "getcolor", &nu::App::GetColor,
           "getdefaultfont", &nu::App::GetDefaultFont);
  }
};

template<>
struct Type<nu::Font::Weight> {
  static constexpr const char* name = "yue.Font.Weight";
  static inline bool To(State* state, int index, nu::Font::Weight* out) {
    std::string weight;
    if (!lua::To(state, index, &weight))
      return false;
    if (weight == "thin") {
      *out = nu::Font::Weight::Thin;
      return true;
    } else if (weight == "extra-light") {
      *out = nu::Font::Weight::ExtraLight;
      return true;
    } else if (weight == "light") {
      *out = nu::Font::Weight::Light;
      return true;
    } else if (weight == "normal") {
      *out = nu::Font::Weight::Normal;
      return true;
    } else if (weight == "medium") {
      *out = nu::Font::Weight::Medium;
      return true;
    } else if (weight == "semi-bold") {
      *out = nu::Font::Weight::SemiBold;
      return true;
    } else if (weight == "bold") {
      *out = nu::Font::Weight::Bold;
      return true;
    } else if (weight == "extra-bold") {
      *out = nu::Font::Weight::ExtraBold;
      return true;
    } else if (weight == "black") {
      *out = nu::Font::Weight::Black;
      return true;
    } else {
      return false;
    }
  }
  static inline void Push(State* state, nu::Font::Weight weight) {
    switch (weight) {
      case nu::Font::Weight::Thin:
        lua::Push(state, "thin");
        break;
      case nu::Font::Weight::ExtraLight:
        lua::Push(state, "extra-light");
        break;
      case nu::Font::Weight::Light:
        lua::Push(state, "light");
        break;
      case nu::Font::Weight::Normal:
        lua::Push(state, "normal");
        break;
      case nu::Font::Weight::Medium:
        lua::Push(state, "medium");
        break;
      case nu::Font::Weight::SemiBold:
        lua::Push(state, "semi-bold");
        break;
      case nu::Font::Weight::Bold:
        lua::Push(state, "bold");
        break;
      case nu::Font::Weight::ExtraBold:
        lua::Push(state, "extra-bold");
        break;
      case nu::Font::Weight::Black:
        lua::Push(state, "black");
        break;
    }
  }
};

template<>
struct Type<nu::Font::Style> {
  static constexpr const char* name = "yue.Font.Style";
  static inline bool To(State* state, int index, nu::Font::Style* out) {
    std::string style;
    if (!lua::To(state, index, &style))
      return false;
    if (style == "normal") {
      *out = nu::Font::Style::Normal;
      return true;
    } else if (style == "italic") {
      *out = nu::Font::Style::Italic;
      return true;
    } else {
      return false;
    }
  }
  static inline void Push(State* state, nu::Font::Style style) {
    switch (style) {
      case nu::Font::Style::Normal:
        lua::Push(state, "normal");
        break;
      case nu::Font::Style::Italic:
        lua::Push(state, "italic");
        break;
    }
  }
};

template<>
struct Type<nu::Font> {
  static constexpr const char* name = "yue.Font";
  static void BuildMetaTable(State* state, int index) {
    RawSet(state, index,
           "create", &CreateOnHeap<nu::Font, const std::string&, float,
                                   nu::Font::Weight, nu::Font::Style>,
           "default", &GetDefault,
           "derive", &nu::Font::Derive,
           "getname", &nu::Font::GetName,
           "getsize", &nu::Font::GetSize,
           "getweight", &nu::Font::GetWeight,
           "getstyle", &nu::Font::GetStyle);
  }
  static nu::Font* GetDefault() {
    return nu::App::GetCurrent()->GetDefaultFont();
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
           "createfrompath", &CreateOnHeap<nu::Image, const base::FilePath&>,
           "createfrombuffer", &CreateOnHeap<nu::Image,
                                             const nu::Buffer&,
                                             float>,
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
    nu::Font* font;
    if (RawGetAndPop(state, index, "font", &font))
      out->font = font;
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
        break;
      case nu::EventType::MouseUp:
        lua::Push(state, "mouseup");
        break;
      case nu::EventType::MouseMove:
        lua::Push(state, "mousemove");
        break;
      case nu::EventType::MouseEnter:
        lua::Push(state, "mouseenter");
        break;
      case nu::EventType::MouseLeave:
        lua::Push(state, "mouseleave");
        break;
      case nu::EventType::KeyDown:
        lua::Push(state, "keydown");
        break;
      case nu::EventType::KeyUp:
        lua::Push(state, "keyup");
        break;
      default:
        NOTREACHED();
        lua::Push(state, "unknown");
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
           "maskmeta", static_cast<int>(nu::MASK_META),
           "isshiftpressed", &nu::Event::IsShiftPressed,
           "iscontrolpressed", &nu::Event::IsControlPressed,
           "isaltpressed", &nu::Event::IsAltPressed,
           "ismetapressed", &nu::Event::IsMetaPressed);
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
           "positioninview", event.position_in_view,
           "positioninwindow", event.position_in_window);
  }
};

template<>
struct Type<nu::KeyboardCode> {
  static constexpr const char* name = "yue.KeyboardCode";
  static inline void Push(State* state, nu::KeyboardCode code) {
    lua::Push(state, nu::KeyboardCodeToStr(code));
  }
};

template<>
struct Type<nu::KeyEvent> {
  using base = nu::Event;
  static constexpr const char* name = "yue.KeyEvent";
  static inline void Push(State* state, const nu::KeyEvent& event) {
    NewTable(state);
    Type<nu::Event>::SetEventProperties(state, -1, &event);
    RawSet(state, -1,
           "key", event.key);
  }
};

template<>
struct Type<nu::FileDialog::Filter> {
  static constexpr const char* name = "yue.FileDialog.Filter";
  static inline void Push(State* state,
                          const nu::FileDialog::Filter& filter) {
    NewTable(state, 0, 2);
    RawSet(state, -1,
           "description", std::get<0>(filter),
           "extensions", std::get<1>(filter));
  }
  static inline bool To(State* state, int index,
                        nu::FileDialog::Filter* out) {
    if (GetType(state, index) == LuaType::Table)
      return false;
    return RawGetAndPop(state, index,
                        "description", &std::get<0>(*out),
                        "extensions", &std::get<1>(*out));
  }
};

template<>
struct Type<nu::FileDialog> {
  static constexpr const char* name = "yue.FileDialog";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "optionpickfolders",
           static_cast<int>(nu::FileDialog::OPTION_PICK_FOLDERS),
           "optionmultiselect",
           static_cast<int>(nu::FileDialog::OPTION_MULTI_SELECT),
           "optionshowhidden",
           static_cast<int>(nu::FileDialog::OPTION_SHOW_HIDDEN),
           "getresult", &nu::FileDialog::GetResult,
           "run", &nu::FileDialog::Run,
           "runforwindow", &nu::FileDialog::RunForWindow,
           "settitle", &nu::FileDialog::SetTitle,
           "setbuttonlabel", &nu::FileDialog::SetButtonLabel,
           "setfilename", &nu::FileDialog::SetFilename,
           "setfolder", &nu::FileDialog::SetFolder,
           "setoptions", &nu::FileDialog::SetOptions,
           "setfilters", &nu::FileDialog::SetFilters);
  }
};

template<>
struct Type<nu::FileOpenDialog> {
  using base = nu::FileOpenDialog;
  static constexpr const char* name = "yue.FileOpenDialog";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "create", &CreateOnHeap<nu::FileOpenDialog>,
           "getresults", &nu::FileOpenDialog::GetResults);
  }
};

template<>
struct Type<nu::FileSaveDialog> {
  using base = nu::FileSaveDialog;
  static constexpr const char* name = "yue.FileSaveDialog";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "create", &CreateOnHeap<nu::FileSaveDialog>);
  }
};

template<>
struct Type<nu::MenuBase> {
  static constexpr const char* name = "yue.MenuBase";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "append", RefMethod(&nu::MenuBase::Append, RefType::Ref),
           "insert", RefMethod(&Insert, RefType::Ref),
           "remove", RefMethod(&nu::MenuBase::Remove, RefType::Deref),
           "itemcount", &nu::MenuBase::ItemCount,
           "itemat", &ItemAt);
  }
  static inline void Insert(nu::MenuBase* menu, nu::MenuItem* item, int i) {
    menu->Insert(item, i - 1);
  }
  static inline nu::MenuItem* ItemAt(nu::MenuBase* menu, int i) {
    return menu->ItemAt(i - 1);
  }
};

void ReadMenuItems(State* state, int metatable, nu::MenuBase* menu);

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
    ReadMenuItems(context->state, context->current_arg, menu);
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
    ReadMenuItems(context->state, context->current_arg, menu);
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
      *out = nu::MenuItem::Type::Checkbox;
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
struct Type<nu::MenuItem::Role> {
  static constexpr const char* name = "yue.MenuItem.Role";
  static bool To(State* state, int index, nu::MenuItem::Role* out) {
    std::string role;
    if (!lua::To(state, index, &role))
      return false;
    if (role == "copy")
      *out = nu::MenuItem::Role::Copy;
    else if (role == "cut")
      *out = nu::MenuItem::Role::Cut;
    else if (role == "paste")
      *out = nu::MenuItem::Role::Paste;
    else if (role == "select-all")
      *out = nu::MenuItem::Role::SelectAll;
    else if (role == "undo")
      *out = nu::MenuItem::Role::Undo;
    else if (role == "redo")
      *out = nu::MenuItem::Role::Redo;
#if defined(OS_MACOSX)
    else if (role == "about")
      *out = nu::MenuItem::Role::About;
    else if (role == "hide")
      *out = nu::MenuItem::Role::Hide;
    else if (role == "hide-others")
      *out = nu::MenuItem::Role::HideOthers;
    else if (role == "unhide")
      *out = nu::MenuItem::Role::Unhide;
    else if (role == "help")
      *out = nu::MenuItem::Role::Help;
    else if (role == "window")
      *out = nu::MenuItem::Role::Window;
    else if (role == "services")
      *out = nu::MenuItem::Role::Services;
#endif
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
           "click", &nu::MenuItem::Click,
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
    State* state = context->state;
    int options = context->current_arg;
    nu::MenuItem::Type type = nu::MenuItem::Type::Label;
    if (lua::To(state, options, &type) ||  // 'type'
        GetType(state, options) != LuaType::Table)  // {type='type'}
      return new nu::MenuItem(type);
    nu::MenuItem* item = nullptr;
    // First read role.
    nu::MenuItem::Role role;
    if (RawGetAndPop(state, options, "role", &role))
      item = new nu::MenuItem(role);
    // Use label unless "type" is specified.
    if (RawGetAndPop(state, options, "type", &type))
      item = new nu::MenuItem(type);
    // Read table fields and set attributes.
    bool b = false;
    if (RawGetAndPop(state, options, "checked", &b)) {
      if (!item) item = new nu::MenuItem(nu::MenuItem::Type::Checkbox);
      item->SetChecked(b);
    }
    nu::Menu* submenu = nullptr;
    RawGet(state, options, "submenu");
    if (To(state, -1, &submenu) || GetType(state, -1) == LuaType::Table) {
      if (!item) item = new nu::MenuItem(nu::MenuItem::Type::Submenu);
      if (!submenu) {
        CallContext context(state);
        context.current_arg = AbsIndex(state, -1);
        submenu = Type<nu::Menu>::Create(&context);
      }
      item->SetSubmenu(submenu);
    }
    PopAndIgnore(state, 1);
    if (!item)  // can not deduce type from property, assuming Label item.
      item = new nu::MenuItem(nu::MenuItem::Type::Label);
    if (RawGetAndPop(state, options, "visible", &b))
      item->SetVisible(b);
    if (RawGetAndPop(state, options, "enabled", &b))
      item->SetEnabled(b);
    std::string label;
    if (RawGetAndPop(state, options, "label", &label))
      item->SetLabel(label);
    nu::Accelerator accelerator;
    if (RawGetAndPop(state, options, "accelerator", &accelerator))
      item->SetAccelerator(accelerator);
    std::function<void(nu::MenuItem*)> onclick;
    if (RawGetAndPop(state, options, "onclick", &onclick))
      item->on_click.Connect(onclick);
    return item;
  }
};

void ReadMenuItems(State* state, int options, nu::MenuBase* menu) {
  if (GetType(state, options) != LuaType::Table)
    return;
  StackAutoReset reset(state);
  PushNil(state);
  while (lua_next(state, options) != 0) {
    // Create the item if a table is passed.
    nu::MenuItem* item;
    if (!To(state, -1, &item)) {
      CallContext context(state);
      context.current_arg = AbsIndex(state, -1);
      item = Type<nu::MenuItem>::Create(&context);
    }
    PopAndIgnore(state, 1);
    menu->Append(item);
  }
}

template<>
struct Type<nu::Tray> {
  static constexpr const char* name = "yue.Tray";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
#if defined(OS_MACOSX)
           "createwithtitle", &CreateOnHeap<nu::Tray, const std::string&>,
#endif
           "createwithimage", &CreateOnHeap<nu::Tray, nu::Image*>,
           "settitle", &nu::Tray::SetTitle,
           "setimage", &nu::Tray::SetImage,
           "setmenu", RefMethod(&nu::Tray::SetMenu, RefType::Reset, "menu"));
    RawSetProperty(state, metatable, "onclick", &nu::Tray::on_click);
  }
};

#if defined(OS_MACOSX)
template<>
struct Type<nu::Toolbar::Item> {
  static constexpr const char* name = "yue.Toolbar::Item";
  static inline bool To(State* state, int index, nu::Toolbar::Item* out) {
    if (GetType(state, index) != LuaType::Table)
      return false;
    nu::Image* image;
    if (RawGetAndPop(state, index, "image", &image))
      out->image = image;
    nu::View* view;
    if (RawGetAndPop(state, index, "view", &view))
      out->view = view;
    RawGetAndPop(state, index, "label", &out->label);
    RawGetAndPop(state, index, "minsize", &out->min_size);
    RawGetAndPop(state, index, "maxsize", &out->max_size);
    RawGetAndPop(state, index, "subitems", &out->subitems);
    RawGetAndPop(state, index, "onclick", &out->on_click);
    return true;
  }
};

template<>
struct Type<nu::Toolbar::DisplayMode> {
  static constexpr const char* name = "yue.Toolbar.DisplayMode";
  static inline bool To(State* state, int index,
                        nu::Toolbar::DisplayMode* out) {
    std::string mode;
    if (!lua::To(state, index, &mode))
      return false;
    if (mode == "default") {
      *out = nu::Toolbar::DisplayMode::Default;
      return true;
    } else if (mode == "icon-and-label") {
      *out = nu::Toolbar::DisplayMode::IconAndLabel;
      return true;
    } else if (mode == "icon") {
      *out = nu::Toolbar::DisplayMode::Icon;
      return true;
    } else if (mode == "label") {
      *out = nu::Toolbar::DisplayMode::Label;
      return true;
    } else {
      return false;
    }
  }
};

template<>
struct Type<nu::Toolbar> {
  static constexpr const char* name = "yue.Toolbar";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "create", &CreateOnHeap<nu::Toolbar, const std::string&>,
           "setdefaultitemidentifiers", &nu::Toolbar::SetDefaultItemIdentifiers,
           "setalloweditemidentifiers", &nu::Toolbar::SetAllowedItemIdentifiers,
           "setallowcustomization", &nu::Toolbar::SetAllowCustomization,
           "setdisplaymode", &nu::Toolbar::SetDisplayMode,
           "setvisible", &nu::Toolbar::SetVisible,
           "isvisible", &nu::Toolbar::IsVisible,
           "getidentifier", &nu::Toolbar::GetIdentifier);
    RawSetProperty(state, metatable,
                   "getitem", &nu::Toolbar::get_item);
  }
};
#endif

template<>
struct Type<nu::Window::Options> {
  static constexpr const char* name = "yue.Window.Options";
  static inline bool To(State* state, int index, nu::Window::Options* out) {
    if (GetType(state, index) == LuaType::Table) {
      RawGetAndPop(state, index, "frame", &out->frame);
      RawGetAndPop(state, index, "transparent", &out->transparent);
#if defined(OS_MACOSX)
      RawGetAndPop(state, index,
                   "showtrafficlights", &out->show_traffic_lights);
#endif
    }
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
           "sethasshadow", &nu::Window::SetHasShadow,
           "hasshadow", &nu::Window::HasShadow,
           "center", &nu::Window::Center,
           "setcontentview",
           RefMethod(&nu::Window::SetContentView, RefType::Reset, "content"),
           "getcontentview", &nu::Window::GetContentView,
           "setcontentsize", &nu::Window::SetContentSize,
           "getcontentsize", &nu::Window::GetContentSize,
           "setbounds", &nu::Window::SetBounds,
           "getbounds", &nu::Window::GetBounds,
           "setsizeconstraints", &nu::Window::SetSizeConstraints,
           "getsizeconstraints", &nu::Window::GetSizeConstraints,
           "setcontentsizeconstraints", &nu::Window::SetContentSizeConstraints,
           "getcontentsizeconstraints", &nu::Window::GetContentSizeConstraints,
           "activate", &nu::Window::Activate,
           "deactivate", &nu::Window::Deactivate,
           "isActive", &nu::Window::IsActive,
           "setvisible", &nu::Window::SetVisible,
           "isvisible", &nu::Window::IsVisible,
           "setalwaysontop", &nu::Window::SetAlwaysOnTop,
           "isalwaysontop", &nu::Window::IsAlwaysOnTop,
           "setfullscreen", &nu::Window::SetFullscreen,
           "isfullscreen", &nu::Window::IsFullscreen,
           "maximize", &nu::Window::Maximize,
           "unmaximize", &nu::Window::Unmaximize,
           "ismaximized", &nu::Window::IsMaximized,
           "minimize", &nu::Window::Minimize,
           "restore", &nu::Window::Restore,
           "isminimized", &nu::Window::IsMinimized,
           "setresizable", &nu::Window::SetResizable,
           "isresizable", &nu::Window::IsResizable,
           "setmaximizable", &nu::Window::SetMaximizable,
           "ismaximizable", &nu::Window::IsMaximizable,
           "setminimizable", &nu::Window::SetMinimizable,
           "isminimizable", &nu::Window::IsMinimizable,
           "setmovable", &nu::Window::SetMovable,
           "ismovable", &nu::Window::IsMovable,
           "settitle", &nu::Window::SetTitle,
           "gettitle", &nu::Window::GetTitle,
           "setbackgroundcolor", &nu::Window::SetBackgroundColor,
#if defined(OS_MACOSX)
           "settoolbar", &nu::Window::SetToolbar,
           "gettoolbar", &nu::Window::GetToolbar,
           "settitlevisible", &nu::Window::SetTitleVisible,
           "istitlevisible", &nu::Window::IsTitleVisible,
           "setfullsizecontentview", &nu::Window::SetFullSizeContentView,
           "isfullsizecontentview", &nu::Window::IsFullSizeContentView,
#endif
#if defined(OS_WIN) || defined(OS_LINUX)
           "setmenubar",
           RefMethod(&nu::Window::SetMenuBar, RefType::Reset, "menubar"),
           "getmenubar", &nu::Window::GetMenuBar,
#endif
           "addchildwindow",
           RefMethod(&nu::Window::AddChildWindow, RefType::Ref),
           "removechildview",
           RefMethod(&nu::Window::RemoveChildWindow, RefType::Deref),
           "getchildwindows", &nu::Window::GetChildWindows);
    RawSetProperty(state, metatable,
                   "onclose", &nu::Window::on_close,
                   "onfocus", &nu::Window::on_focus,
                   "onblur", &nu::Window::on_blur,
                   "shouldclose", &nu::Window::should_close);
  }
};

template<>
struct Type<nu::View> {
  static constexpr const char* name = "yue.View";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "offsetfromview", &nu::View::OffsetFromView,
           "offsetfromwindow", &nu::View::OffsetFromWindow,
           "setbounds", &nu::View::SetBounds,
           "getbounds", &nu::View::GetBounds,
           "layout", &nu::View::Layout,
           "schedulepaint", &nu::View::SchedulePaint,
           "schedulepaintrect", &nu::View::SchedulePaintRect,
           "setvisible", &nu::View::SetVisible,
           "isvisible", &nu::View::IsVisible,
           "setenabled", &nu::View::SetEnabled,
           "isenabled", &nu::View::IsEnabled,
           "focus", &nu::View::Focus,
           "hasfocus", &nu::View::HasFocus,
           "setfocusable", &nu::View::SetFocusable,
           "isfocusable", &nu::View::IsFocusable,
           "setcapture", &nu::View::SetCapture,
           "releasecapture", &nu::View::ReleaseCapture,
           "hascapture", &nu::View::HasCapture,
           "setmousedowncanmovewindow", &nu::View::SetMouseDownCanMoveWindow,
           "ismousedowncanmovewindow", &nu::View::IsMouseDownCanMoveWindow,
           "setfont", &nu::View::SetFont,
           "setcolor", &nu::View::SetColor,
           "setbackgroundcolor", &nu::View::SetBackgroundColor,
           "setstyle", &SetStyle,
           "printstyle", &nu::View::PrintStyle,
           "getminimumsize", &nu::View::GetMinimumSize,
#if defined(OS_MACOSX)
           "setwantslayer", &nu::View::SetWantsLayer,
           "wantslayer", &nu::View::WantsLayer,
#endif
           "getparent", &nu::View::GetParent,
           "getwindow", &nu::View::GetWindow);
    RawSetProperty(state, metatable,
                   "onmousedown", &nu::View::on_mouse_down,
                   "onmouseup", &nu::View::on_mouse_up,
                   "onmousemove", &nu::View::on_mouse_move,
                   "onmouseenter", &nu::View::on_mouse_enter,
                   "onmouseleave", &nu::View::on_mouse_leave,
                   "onkeydown", &nu::View::on_key_down,
                   "onkeyup", &nu::View::on_key_up,
                   "onsizechanged", &nu::View::on_size_changed,
                   "oncapturelost", &nu::View::on_capture_lost);
  }
  static void SetStyle(nu::View* view,
                       const std::map<std::string, std::string>& styles) {
    for (const auto& it : styles)
      view->SetStyleProperty(it.first, it.second);
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
           "addchildview",
           RefMethod(&nu::Container::AddChildView, RefType::Ref),
           "addchildviewat",
           RefMethod(&AddChildViewAt, RefType::Ref),
           "removechildview",
           RefMethod(&nu::Container::RemoveChildView, RefType::Deref),
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
      *out = nu::Button::Type::Checkbox;
      return true;
    } else if (type == "radio") {
      *out = nu::Button::Type::Radio;
      return true;
    } else {
      return false;
    }
  }
};

#if defined(OS_MACOSX)
template<>
struct Type<nu::Button::Style> {
  static constexpr const char* name = "yue.Button.Style";
  static bool To(State* state, int index, nu::Button::Style* out) {
    std::string style;
    if (!lua::To(state, index, &style))
      return false;
    if (style == "rounded") {
      *out = nu::Button::Style::Rounded;
      return true;
    } else if (style == "regular-square") {
      *out = nu::Button::Style::RegularSquare;
      return true;
    } else if (style == "thick-square") {
      *out = nu::Button::Style::ThickSquare;
      return true;
    } else if (style == "thicker-square") {
      *out = nu::Button::Style::ThickerSquare;
      return true;
    } else if (style == "disclosure") {
      *out = nu::Button::Style::Disclosure;
      return true;
    } else if (style == "shadowless-square") {
      *out = nu::Button::Style::ShadowlessSquare;
      return true;
    } else if (style == "circular") {
      *out = nu::Button::Style::Circular;
      return true;
    } else if (style == "textured-square") {
      *out = nu::Button::Style::TexturedSquare;
      return true;
    } else if (style == "help-button") {
      *out = nu::Button::Style::HelpButton;
      return true;
    } else if (style == "small-square") {
      *out = nu::Button::Style::SmallSquare;
      return true;
    } else if (style == "textured-rounded") {
      *out = nu::Button::Style::TexturedRounded;
      return true;
    } else if (style == "round-rect") {
      *out = nu::Button::Style::RoundRect;
      return true;
    } else if (style == "recessed") {
      *out = nu::Button::Style::Recessed;
      return true;
    } else if (style == "rounded-disclosure") {
      *out = nu::Button::Style::Recessed;
      return true;
    } else if (style == "inline") {
      *out = nu::Button::Style::Inline;
      return true;
    } else {
      return false;
    }
  }
};
#endif

template<>
struct Type<nu::Button> {
  using base = nu::View;
  static constexpr const char* name = "yue.Button";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "create", &Create,
           "settitle", &nu::Button::SetTitle,
           "gettitle", &nu::Button::GetTitle,
#if defined(OS_MACOSX)
           "setbuttonstyle", &nu::Button::SetButtonStyle,
           "sethasborder", &nu::Button::SetHasBorder,
           "hasborder", &nu::Button::HasBorder,
#endif
           "setchecked", &nu::Button::SetChecked,
           "ischecked", &nu::Button::IsChecked,
           "setimage", &nu::Button::SetImage,
           "getimage", &nu::Button::GetImage);
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
struct Type<nu::ProtocolJob> {
  static constexpr const char* name = "yue.ProtocolJob";
  static void BuildMetaTable(State* state, int metatable) {
  }
};

template<>
struct Type<nu::ProtocolStringJob> {
  using base = nu::ProtocolJob;
  static constexpr const char* name = "yue.ProtocolStringJob";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "create", &CreateOnHeap<nu::ProtocolStringJob,
                                   const std::string&,
                                   const std::string&>);
  }
};

template<>
struct Type<nu::ProtocolFileJob> {
  using base = nu::ProtocolJob;
  static constexpr const char* name = "yue.ProtocolFileJob";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "create", &CreateOnHeap<nu::ProtocolFileJob,
                                   const ::base::FilePath&>);
  }
};

template<>
struct Type<nu::ProtocolAsarJob> {
  using base = nu::ProtocolFileJob;
  static constexpr const char* name = "yue.ProtocolAsarJob";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "create", &CreateOnHeap<nu::ProtocolAsarJob,
                                   const ::base::FilePath&,
                                   const std::string&>,
           "setdecipher", &nu::ProtocolAsarJob::SetDecipher);
  }
};

template<>
struct Type<nu::Browser::Options> {
  static constexpr const char* name = "yue.Browser.Options";
  static inline bool To(State* state, int index, nu::Browser::Options* out) {
    if (GetType(state, index) == LuaType::Table) {
      RawGetAndPop(state, index, "devtools", &out->devtools);
      RawGetAndPop(state, index, "contextmenu", &out->context_menu);
    }
    return true;
  }
};

template<>
struct Type<nu::Browser> {
  using base = nu::View;
  static constexpr const char* name = "yue.Browser";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "create", &CreateOnHeap<nu::Browser, const nu::Browser::Options&>,
           "registerprotocol", &nu::Browser::RegisterProtocol,
           "unregisterprotocol", &nu::Browser::UnregisterProtocol,
           "loadurl", &nu::Browser::LoadURL,
           "loadhtml", &nu::Browser::LoadHTML,
           "geturl", &nu::Browser::GetURL,
           "gettitle", &nu::Browser::GetTitle,
           "setuseragent", &nu::Browser::SetUserAgent,
           "executejavascript", &nu::Browser::ExecuteJavaScript,
           "goback", &nu::Browser::GoBack,
           "cangoback", &nu::Browser::CanGoBack,
           "goforward", &nu::Browser::GoForward,
           "cangoforward", &nu::Browser::CanGoForward,
           "reload", &nu::Browser::Reload,
           "stop", &nu::Browser::Stop,
           "isloading", &nu::Browser::IsLoading,
           "setbindingname", &nu::Browser::SetBindingName,
           "addbinding", &AddBinding,
           "addrawbinding", &nu::Browser::AddRawBinding,
           "removebinding", &nu::Browser::RemoveBinding);
    RawSetProperty(state, metatable,
                   "onclose", &nu::Browser::on_close,
                   "onupdatecommand", &nu::Browser::on_update_command,
                   "onchangeloading", &nu::Browser::on_change_loading,
                   "onupdatetitle", &nu::Browser::on_update_title,
                   "onstartnavigation", &nu::Browser::on_start_navigation,
                   "oncommitnavigation", &nu::Browser::on_commit_navigation,
                   "onfinishnavigation", &nu::Browser::on_finish_navigation,
                   "onfailnavigation", &nu::Browser::on_fail_navigation);
  }
  static void AddBinding(CallContext* context,
                         nu::Browser* browser,
                         const std::string& name) {
    State* state = context->state;
    if (GetType(state, 3) != LuaType::Function) {
      Push(state, "The arg 3 should be function");
      context->has_error = true;
      return;
    }
    // Persistent the function and pass it to lambda.
    std::shared_ptr<Persistent> func_ref = Persistent::New(state, 3);
    browser->AddRawBinding(name, [state, func_ref](nu::Browser* browser,
                                                   ::base::Value value) {
      func_ref->Push();
      for (const auto& it : value.GetList())
        Push(state, it);
      lua_pcall(state, static_cast<int>(value.GetList().size()), 0, 0);
    });
  }
};

template<>
struct Type<nu::Entry::Type> {
  static constexpr const char* name = "yue.Entry.Type";
  static bool To(State* state, int index, nu::Entry::Type* out) {
    std::string type;
    if (!lua::To(state, index, &type))
      return false;
    if (type == "normal")
      *out = nu::Entry::Type::Normal;
    else if (type == "password")
      *out = nu::Entry::Type::Password;
    else
      return false;
    return true;
  }
};

template<>
struct Type<nu::Entry> {
  using base = nu::View;
  static constexpr const char* name = "yue.Entry";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "create", &CreateOnHeap<nu::Entry>,
           "createtype", &CreateOnHeap<nu::Entry, nu::Entry::Type>,
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
           "gettext", &nu::Label::GetText,
           "setalign", &nu::Label::SetAlign,
           "setvalign", &nu::Label::SetVAlign);
  }
};

template<>
struct Type<nu::ProgressBar> {
  using base = nu::View;
  static constexpr const char* name = "yue.ProgressBar";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "create", &CreateOnHeap<nu::ProgressBar>,
           "setvalue", &nu::ProgressBar::SetValue,
           "getvalue", &nu::ProgressBar::GetValue,
           "setindeterminate", &nu::ProgressBar::SetIndeterminate,
           "isindeterminate", &nu::ProgressBar::IsIndeterminate);
  }
};

template<>
struct Type<nu::GifPlayer> {
  using base = nu::View;
  static constexpr const char* name = "yue.GifPlayer";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "create", &CreateOnHeap<nu::GifPlayer>,
           "setimage", &nu::GifPlayer::SetImage,
           "getimage", &nu::GifPlayer::GetImage,
           "setanimating", &nu::GifPlayer::SetAnimating,
           "isanimating", &nu::GifPlayer::IsAnimating);
  }
};

template<>
struct Type<nu::Group> {
  using base = nu::View;
  static constexpr const char* name = "yue.Group";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "create", &CreateOnHeap<nu::Group, const std::string&>,
           "setcontentview",
           RefMethod(&nu::Group::SetContentView, RefType::Reset, "content"),
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
           "setcontentsize", &nu::Scroll::SetContentSize,
           "getcontentsize", &nu::Scroll::GetContentSize,
           "setcontentview",
           RefMethod(&nu::Scroll::SetContentView, RefType::Reset, "content"),
           "getcontentview", &nu::Scroll::GetContentView,
#if !defined(OS_WIN)
           "setOverlayScrollbar", &nu::Scroll::SetOverlayScrollbar,
           "isOverlayScrollbar", &nu::Scroll::IsOverlayScrollbar,
#endif
           "setscrollbarpolicy", &nu::Scroll::SetScrollbarPolicy,
           "getscrollbarpolicy", &nu::Scroll::GetScrollbarPolicy);
  }
};

template<>
struct Type<nu::TextEdit> {
  using base = nu::View;
  static constexpr const char* name = "yue.TextEdit";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "create", &CreateOnHeap<nu::TextEdit>,
           "settext", &nu::TextEdit::SetText,
           "gettext", &nu::TextEdit::GetText,
           "redo", &nu::TextEdit::Redo,
           "canredo", &nu::TextEdit::CanRedo,
           "undo", &nu::TextEdit::Undo,
           "canundo", &nu::TextEdit::CanUndo,
           "cut", &nu::TextEdit::Cut,
           "copy", &nu::TextEdit::Copy,
           "paste", &nu::TextEdit::Paste,
           "selectall", &nu::TextEdit::SelectAll,
           "getselectionrange", &nu::TextEdit::GetSelectionRange,
           "selectrange", &nu::TextEdit::SelectRange,
           "gettextinrange", &nu::TextEdit::GetTextInRange,
           "inserttext", &nu::TextEdit::InsertText,
           "inserttextat", &nu::TextEdit::InsertTextAt,
           "delete", &nu::TextEdit::Delete,
           "deleterange", &nu::TextEdit::DeleteRange,
#if !defined(OS_WIN)
           "setoverlayscrollbar", &nu::TextEdit::SetOverlayScrollbar,
#endif
           "setscrollbarpolicy", &nu::TextEdit::SetScrollbarPolicy,
           "gettextbounds", &nu::TextEdit::GetTextBounds);
    RawSetProperty(state, metatable,
                   "ontextchange", &nu::TextEdit::on_text_change,
                   "shouldinsertnewline",
                   &nu::TextEdit::should_insert_new_line);
  }
};

#if defined(OS_MACOSX)
template<>
struct Type<nu::Vibrant::Material> {
  static constexpr const char* name = "yue.Vibrant.Material";
  static inline bool To(State* state, int index, nu::Vibrant::Material* out) {
    std::string material;
    if (!lua::To(state, index, &material))
      return false;
    if (material == "appearance-based") {
      *out = nu::Vibrant::Material::AppearanceBased;
      return true;
    } else if (material == "light") {
      *out = nu::Vibrant::Material::Light;
      return true;
    } else if (material == "dark") {
      *out = nu::Vibrant::Material::Dark;
      return true;
    } else if (material == "titlebar") {
      *out = nu::Vibrant::Material::Titlebar;
      return true;
    } else {
      return false;
    }
  }
  static inline void Push(State* state, nu::Vibrant::Material material) {
    if (material == nu::Vibrant::Material::Light)
      lua::Push(state, "light");
    else if (material == nu::Vibrant::Material::Dark)
      lua::Push(state, "dark");
    else if (material == nu::Vibrant::Material::Titlebar)
      lua::Push(state, "titlebar");
    else
      lua::Push(state, "appearance-based");
  }
};

template<>
struct Type<nu::Vibrant::BlendingMode> {
  static constexpr const char* name = "yue.Vibrant.BlendingMode";
  static inline bool To(State* state, int index,
                        nu::Vibrant::BlendingMode* out) {
    std::string mode;
    if (!lua::To(state, index, &mode))
      return false;
    if (mode == "within-window") {
      *out = nu::Vibrant::BlendingMode::WithinWindow;
      return true;
    } else if (mode == "behind-window") {
      *out = nu::Vibrant::BlendingMode::BehindWindow;
      return true;
    } else {
      return false;
    }
  }
  static inline void Push(State* state, nu::Vibrant::BlendingMode mode) {
    if (mode == nu::Vibrant::BlendingMode::WithinWindow)
      lua::Push(state, "within-window");
    else
      lua::Push(state, "behind-window");
  }
};

template<>
struct Type<nu::Vibrant> {
  using base = nu::Container;
  static constexpr const char* name = "yue.Vibrant";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "create", &CreateOnHeap<nu::Vibrant>,
           "setmaterial", &nu::Vibrant::SetMaterial,
           "getmaterial", &nu::Vibrant::GetMaterial,
           "setblendingmode", &nu::Vibrant::SetBlendingMode,
           "getblendingmode", &nu::Vibrant::GetBlendingMode);
  }
};
#endif

}  // namespace lua

template<typename T>
inline void BindType(lua::State* state, const char* name) {
  int top = lua::GetTop(state);
  lua::Push(state, name);
  lua::Push(state, lua::MetaTable<T>());
  // Reference the nu::State object so it is freed at last.
  lua::RawSet(state, -1, "__state", lua::ValueOnStack(state, top - 1));
  // Assign to exports table.
  lua_rawset(state, top);
}

extern "C" int luaopen_yue_gui(lua::State* state) {
  // Initialize nativeui.
  lua::NewUserData<nu::Lifetime>(state);
  lua::NewUserData<nu::State>(state);
  // The exports table.
  lua::NewTable(state);

  // Classes.
  BindType<nu::Lifetime>(state, "Lifetime");
  BindType<nu::MessageLoop>(state, "MessageLoop");
  BindType<nu::App>(state, "App");
  BindType<nu::Font>(state, "Font");
  BindType<nu::Canvas>(state, "Canvas");
  BindType<nu::Color>(state, "Color");
  BindType<nu::Image>(state, "Image");
  BindType<nu::Painter>(state, "Painter");
  BindType<nu::Event>(state, "Event");
  BindType<nu::FileDialog>(state, "FileDialog");
  BindType<nu::FileOpenDialog>(state, "FileOpenDialog");
  BindType<nu::FileSaveDialog>(state, "FileSaveDialog");
  BindType<nu::MenuBar>(state, "MenuBar");
  BindType<nu::Menu>(state, "Menu");
  BindType<nu::MenuItem>(state, "MenuItem");
  BindType<nu::Window>(state, "Window");
  BindType<nu::Container>(state, "Container");
  BindType<nu::Button>(state, "Button");
  BindType<nu::ProtocolStringJob>(state, "ProtocolStringJob");
  BindType<nu::ProtocolFileJob>(state, "ProtocolFileJob");
  BindType<nu::ProtocolAsarJob>(state, "ProtocolAsarJob");
  BindType<nu::Browser>(state, "Browser");
  BindType<nu::Entry>(state, "Entry");
  BindType<nu::Label>(state, "Label");
  BindType<nu::ProgressBar>(state, "ProgressBar");
  BindType<nu::GifPlayer>(state, "GifPlayer");
  BindType<nu::Group>(state, "Group");
  BindType<nu::Scroll>(state, "Scroll");
  BindType<nu::TextEdit>(state, "TextEdit");
  BindType<nu::Tray>(state, "Tray");
#if defined(OS_MACOSX)
  BindType<nu::Toolbar>(state, "Toolbar");
  BindType<nu::Vibrant>(state, "Vibrant");
#endif
  // Properties.
  lua::RawSet(state, -1,
              "lifetime", nu::Lifetime::GetCurrent(),
              "app",      nu::State::GetCurrent()->GetApp());
  return 1;
}
