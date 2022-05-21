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
  static constexpr const char* name = "FilePath";
  static inline void Push(State* state, const base::FilePath& value) {
    return lua::Push(state, value.value());
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
struct Type<base::Time> {
  static constexpr const char* name = "Time";
  static inline void Push(State* state, const base::Time& value) {
    lua_pushinteger(state, value.ToTimeT());
  }
  static inline bool To(State* state, int index, base::Time* out) {
    lua_Integer integer;
    if (!Type<lua_Integer>::To(state, index, &integer))
      return false;
    *out = base::Time::FromTimeT(integer);
    return true;
  }
};

template<>
struct Type<nu::Buffer> {
  static constexpr const char* name = "Buffer";
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
struct Type<nu::Display> {
  static constexpr const char* name = "Display";
  static inline void Push(State* state, const nu::Display& display) {
    lua::NewTable(state);
    lua::RawSet(state, -1,
                "id", display.id,
                "scalefactor", display.scale_factor,
#if defined(OS_MAC)
                "internal", display.internal,
#endif
                "bounds", display.bounds,
                "workarea", display.work_area);
  }
};

template<>
struct Type<nu::Size> {
  static constexpr const char* name = "Size";
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
    if (!ReadOptions(state, index, "width", &width, "height", &height))
      return false;
    *out = nu::Size(width, height);
    return true;
  }
};

template<>
struct Type<nu::SizeF> {
  static constexpr const char* name = "SizeF";
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
    if (!ReadOptions(state, index, "width", &width, "height", &height))
      return false;
    *out = nu::SizeF(width, height);
    return true;
  }
};

template<>
struct Type<nu::RectF> {
  static constexpr const char* name = "RectF";
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
    if (!ReadOptions(state, index,
                     "x", &x, "y", &y,
                     "width", &width, "height", &height))
      return false;
    *out = nu::RectF(x, y, width, height);
    return true;
  }
};

template<>
struct Type<nu::Vector2dF> {
  static constexpr const char* name = "Vector2dF";
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
    if (!ReadOptions(state, index, "x", &x, "y", &y))
      return false;
    *out = nu::Vector2dF(x, y);
    return true;
  }
};

template<>
struct Type<nu::PointF> {
  static constexpr const char* name = "PointF";
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
    if (!ReadOptions(state, index, "x", &x, "y", &y))
      return false;
    *out = nu::PointF(x, y);
    return true;
  }
};

template<>
struct Type<nu::ImageScale> {
  static constexpr const char* name = "ImageScale";
  static inline bool To(State* state, int index, nu::ImageScale* out) {
    std::string scale;
    if (!lua::To(state, index, &scale))
      return false;
    if (scale == "none") {
      *out = nu::ImageScale::None;
      return true;
    } else if (scale == "fill") {
      *out = nu::ImageScale::Fill;
      return true;
    } else if (scale == "down") {
      *out = nu::ImageScale::Down;
      return true;
    } else if (scale == "up-or-down") {
      *out = nu::ImageScale::UpOrDown;
      return true;
    } else {
      return false;
    }
  }
  static inline void Push(State* state, nu::ImageScale scale) {
    switch (scale) {
      case nu::ImageScale::None:
        return lua::Push(state, "none");
      case nu::ImageScale::Fill:
        return lua::Push(state, "fill");
      case nu::ImageScale::Down:
        return lua::Push(state, "down");
      case nu::ImageScale::UpOrDown:
        return lua::Push(state, "up-or-down");
    }
    NOTREACHED();
    return lua::Push(state, nullptr);
  }
};

template<>
struct Type<nu::Orientation> {
  static constexpr const char* name = "Orientation";
  static inline bool To(State* state, int index, nu::Orientation* out) {
    std::string orientation;
    if (!lua::To(state, index, &orientation))
      return false;
    if (orientation == "horizontal") {
      *out = nu::Orientation::Horizontal;
      return true;
    } else if (orientation == "vertical") {
      *out = nu::Orientation::Vertical;
      return true;
    } else {
      return false;
    }
  }
  static inline void Push(State* state, nu::Orientation orientation) {
    switch (orientation) {
      case nu::Orientation::Horizontal:
        return lua::Push(state, "horizontal");
      case nu::Orientation::Vertical:
        return lua::Push(state, "vertical");
    }
    NOTREACHED();
    return lua::Push(state, nullptr);
  }
};

template<>
struct Type<nu::Accelerator> {
  static constexpr const char* name = "Accelerator";
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

#if defined(OS_MAC)
template<>
struct Type<nu::Lifetime::Reply> {
  static constexpr const char* name = "Lifetime::Reply";
  static inline bool To(State* state, int index, nu::Lifetime::Reply* out) {
    std::string reply;
    if (!lua::To(state, index, &reply))
      return false;
    if (reply == "success") {
      *out = nu::Lifetime::Reply::Success;
      return true;
    } else if (reply == "cancel") {
      *out = nu::Lifetime::Reply::Cancel;
      return true;
    } else if (reply == "failure") {
      *out = nu::Lifetime::Reply::Failure;
      return true;
    } else {
      return false;
    }
  }
};
#endif

template<>
struct Type<nu::Lifetime> {
  static constexpr const char* name = "Lifetime";
  static void BuildMetaTable(State* state, int index) {
#if defined(OS_MAC)
    RawSetProperty(state, index,
                   "onready", &nu::Lifetime::on_ready,
                   "onactivate", &nu::Lifetime::on_activate,
                   "openfiles", &nu::Lifetime::open_files);
#endif
  }
};

template<>
struct Type<nu::MessageLoop> {
  static constexpr const char* name = "MessageLoop";
  static void BuildMetaTable(State* state, int index) {
    RawSet(state, index,
           "run", &nu::MessageLoop::Run,
           "quit", &nu::MessageLoop::Quit,
           "posttask", &nu::MessageLoop::PostTask,
           "postdelayedtask", &nu::MessageLoop::PostDelayedTask);
  }
};

#if defined(OS_MAC)
template<>
struct Type<nu::App::ActivationPolicy> {
  static constexpr const char* name = "AppActivationPolicy";
  static inline bool To(State* state, int index,
                        nu::App::ActivationPolicy* out) {
    std::string policy;
    if (!lua::To(state, index, &policy))
      return false;
    if (policy == "regular") {
      *out = nu::App::ActivationPolicy::Regular;
      return true;
    } else if (policy == "accessory") {
      *out = nu::App::ActivationPolicy::Accessory;
      return true;
    } else if (policy == "prohibited") {
      *out = nu::App::ActivationPolicy::Prohibited;
      return true;
    } else {
      return false;
    }
  }
  static inline void Push(State* state, nu::App::ActivationPolicy policy) {
    switch (policy) {
      case nu::App::ActivationPolicy::Regular:
        lua::Push(state, "regular");
        break;
      case nu::App::ActivationPolicy::Accessory:
        lua::Push(state, "accessory");
        break;
      case nu::App::ActivationPolicy::Prohibited:
        lua::Push(state, "prohibited");
        break;
    }
  }
};
#endif

#if defined(OS_WIN)
template<>
struct Type<nu::App::ShortcutOptions> {
  static constexpr const char* name = "AppShortcutOptions";
  static inline bool To(State* state, int index,
                        nu::App::ShortcutOptions* out) {
    if (GetType(state, index) != LuaType::Table)
      return false;
    return ReadOptions(state, index,
                       "arguments", &out->arguments,
                       "description", &out->description,
                       "workingdir", &out->working_dir);
  }
};
#endif

template<>
struct Type<nu::App> {
  static constexpr const char* name = "App";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "setname", &nu::App::SetName,
           "getname", &nu::App::GetName,
#if defined(OS_LINUX) || defined(OS_WIN)
           "setid", &nu::App::SetID,
#endif
           "getid", &nu::App::GetID);
#if defined(OS_MAC)
    RawSet(state, metatable,
           "setapplicationmenu",
           RefMethod(state, &nu::App::SetApplicationMenu,
                     RefType::Reset, "appmenu"),
           "setdockbadgelabel", &nu::App::SetDockBadgeLabel,
           "getdockbadgelabel", &nu::App::GetDockBadgeLabel,
           "activate", &nu::App::Activate,
           "deactivate", &nu::App::Deactivate,
           "isactive", &nu::App::IsActive,
           "setactivationpolicy", &nu::App::SetActivationPolicy,
           "getactivationpolicy", &nu::App::GetActivationPolicy);
#elif defined(OS_WIN)
    RawSet(state, metatable,
           "createstartmenushortcut", &nu::App::CreateStartMenuShortcut,
           "getstartmenushortcutpath", &nu::App::GetStartMenuShortcutPath);
#endif
  }
};

template<>
struct Type<nu::Appearance> {
  static constexpr const char* name = "Appearance";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
#if defined(OS_WIN)
           "setdarkmodeenabled", &nu::Appearance::SetDarkModeEnabled,
#endif
           "isdarkscheme", &nu::Appearance::IsDarkScheme);
    RawSetProperty(
        state, metatable,
        "oncolorschemechange", &nu::Appearance::on_color_scheme_change);
  }
};

template<>
struct Type<nu::AttributedText> {
  static constexpr const char* name = "AttributedText";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "create", &CreateOnHeap<nu::AttributedText,
                                   const std::string&,
                                   nu::TextAttributes>,
           "setformat", &nu::AttributedText::SetFormat,
           "getformat", &nu::AttributedText::GetFormat,
           "setfont", &nu::AttributedText::SetFont,
#if !defined(OS_WIN)
           "setfontfor", &SetFontFor,
#endif
           "setcolor", &nu::AttributedText::SetColor,
#if !defined(OS_WIN)
           "setcolorfor", &SetColorFor,
#endif
           "clear", &nu::AttributedText::Clear,
           "getboundsfor", &nu::AttributedText::GetBoundsFor,
           "settext", &nu::AttributedText::SetText,
           "gettext", &nu::AttributedText::GetText);
  }
#if !defined(OS_WIN)
  static void SetFontFor(nu::AttributedText* text, nu::Font* font,
                         int start, int end) {
    text->SetFontFor(font, start - 1, end <= 0 ? end : end - 1);
  }
  static void SetColorFor(nu::AttributedText* text, nu::Color color,
                          int start, int end) {
    text->SetColorFor(color, start - 1, end <= 0 ? end : end - 1);
  }
#endif
};

template<>
struct Type<nu::Font::Weight> {
  static constexpr const char* name = "FontWeight";
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
  static constexpr const char* name = "FontStyle";
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
  static constexpr const char* name = "Font";
  static void BuildMetaTable(State* state, int index) {
    RawSet(state, index,
           "create", &CreateOnHeap<nu::Font, const std::string&, float,
                                   nu::Font::Weight, nu::Font::Style>,
           "createfrompath", &CreateOnHeap<nu::Font, const base::FilePath&,
                                           float>,
           "default", &nu::Font::Default,
           "derive", &nu::Font::Derive,
           "getname", &nu::Font::GetName,
           "getsize", &nu::Font::GetSize,
           "getweight", &nu::Font::GetWeight,
           "getstyle", &nu::Font::GetStyle);
  }
};

template<>
struct Type<nu::Canvas> {
  static constexpr const char* name = "Canvas";
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
struct Type<nu::Clipboard::Data::Type> {
  static constexpr const char* name = "ClipboardDataType";
  static inline void Push(State* state, nu::Clipboard::Data::Type type) {
    switch (type) {
      case nu::Clipboard::Data::Type::Text:
        return lua::Push(state, "text");
      case nu::Clipboard::Data::Type::HTML:
        return lua::Push(state, "html");
      case nu::Clipboard::Data::Type::Image:
        return lua::Push(state, "image");
      case nu::Clipboard::Data::Type::FilePaths:
        return lua::Push(state, "file-paths");
      default:
        return lua::Push(state, "none");
    }
  }
  static inline bool To(State* state, int index,
                        nu::Clipboard::Data::Type* out) {
    std::string type;
    if (!lua::To(state, index, &type))
      return false;
    if (type == "text") {
      *out = nu::Clipboard::Data::Type::Text;
      return true;
    } else if (type == "html") {
      *out = nu::Clipboard::Data::Type::HTML;
      return true;
    } else if (type == "image") {
      *out = nu::Clipboard::Data::Type::Image;
      return true;
    } else if (type == "file-paths") {
      *out = nu::Clipboard::Data::Type::FilePaths;
      return true;
    } else {
      return false;
    }
  }
};

template<>
struct Type<nu::Clipboard::Data> {
  static constexpr const char* name = "ClipboardData";
  static inline void Push(State* state, const nu::Clipboard::Data& data) {
    NewTable(state);
    RawSet(state, -1, "type", data.type());
    switch (data.type()) {
      case nu::Clipboard::Data::Type::Text:
      case nu::Clipboard::Data::Type::HTML:
        RawSet(state, -1, "value", data.str());
        break;
      case nu::Clipboard::Data::Type::Image:
        RawSet(state, -1, "value", data.image());
        break;
      case nu::Clipboard::Data::Type::FilePaths:
        RawSet(state, -1, "value", data.file_paths());
        break;
      default:
        RawSet(state, -1, "value", nullptr);
        break;
    }
  }
  static inline bool To(State* state, int index, nu::Clipboard::Data* out) {
    if (GetType(state, index) != LuaType::Table)
      return false;
    nu::Clipboard::Data::Type type;
    if (!RawGetAndPop(state, index, "type", &type))
      return false;
    switch (type) {
      case nu::Clipboard::Data::Type::Text:
      case nu::Clipboard::Data::Type::HTML: {
        std::string str;
        if (!RawGetAndPop(state, index, "value", &str))
          return false;
        *out = nu::Clipboard::Data(type, std::move(str));
        break;
      }
      case nu::Clipboard::Data::Type::Image: {
        nu::Image* image;
        if (!RawGetAndPop(state, index, "value", &image))
          return false;
        *out = nu::Clipboard::Data(image);
        break;
      }
      case nu::Clipboard::Data::Type::FilePaths: {
        std::vector<base::FilePath> file_paths;
        if (!RawGetAndPop(state, index, "value", &file_paths))
          return false;
        *out = nu::Clipboard::Data(std::move(file_paths));
        break;
      }
      default:
        return false;
    }
    return true;
  }
};

template<>
struct Type<nu::Clipboard::Type> {
  static constexpr const char* name = "ClipboardType";
  static inline bool To(State* state, int index, nu::Clipboard::Type* out) {
    std::string type;
    if (!lua::To(state, index, &type))
      return false;
    if (type == "copy-paste") {
      *out = nu::Clipboard::Type::CopyPaste;
      return true;
#if defined(OS_MAC)
    } else if (type == "drag") {
      *out = nu::Clipboard::Type::Drag;
      return true;
    } else if (type == "find") {
      *out = nu::Clipboard::Type::Find;
      return true;
    } else if (type == "font") {
      *out = nu::Clipboard::Type::Font;
      return true;
#elif defined(OS_LINUX)
    } else if (type == "selection") {
      *out = nu::Clipboard::Type::Selection;
      return true;
#endif
    } else {
      return false;
    }
  }
};

template<>
struct Type<nu::Clipboard> {
  static constexpr const char* name = "Clipboard";
  static void BuildMetaTable(State* state, int index) {
    RawSet(state, index,
           "get", &nu::Clipboard::Get,
           "fromtype", &nu::Clipboard::FromType,
           "clear", &nu::Clipboard::Clear,
           "settext", &nu::Clipboard::SetText,
           "gettext", &nu::Clipboard::GetText,
           "isdataavailable", &nu::Clipboard::IsDataAvailable,
           "getdata", &nu::Clipboard::GetData,
           "setdata", &nu::Clipboard::SetData,
           "startwatching", &nu::Clipboard::StartWatching,
           "stopwatching", &nu::Clipboard::StopWatching);
    RawSetProperty(state, index,
                   "onchange", &nu::Clipboard::on_change);
  }
};

template<>
struct Type<nu::Color::Name> {
  static constexpr const char* name = "ColorName";
  static inline bool To(State* state, int index, nu::Color::Name* out) {
    std::string id;
    if (!lua::To(state, index, &id))
      return false;
    if (id == "text")
      *out = nu::Color::Name::Text;
    else if (id == "disabled-text")
      *out = nu::Color::Name::DisabledText;
    else if (id == "control")
      *out = nu::Color::Name::Control;
    else if (id == "window-background")
      *out = nu::Color::Name::WindowBackground;
    else
      return false;
    return true;
  }
};

template<>
struct Type<nu::Color> {
  static constexpr const char* name = "Color";
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
           "get", &nu::Color::Get,
           "rgb", &CreateOnStack<nu::Color, unsigned, unsigned, unsigned>,
           "argb", &CreateOnStack<nu::Color, unsigned, unsigned, unsigned,
                                  unsigned>);
  }
};

template<>
struct Type<nu::Cursor::Type> {
  static constexpr const char* name = "CursorType";
  static inline bool To(State* state, int index, nu::Cursor::Type* out) {
    std::string type;
    if (!lua::To(state, index, &type))
      return false;
    if (type == "default") {
      *out = nu::Cursor::Type::Default;
      return true;
    } else if (type == "hand") {
      *out = nu::Cursor::Type::Hand;
      return true;
    } else if (type == "crosshair") {
      *out = nu::Cursor::Type::Crosshair;
      return true;
    } else if (type == "progress") {
      *out = nu::Cursor::Type::Progress;
      return true;
    } else if (type == "text") {
      *out = nu::Cursor::Type::Text;
      return true;
    } else if (type == "not-allowed") {
      *out = nu::Cursor::Type::NotAllowed;
      return true;
    } else if (type == "help") {
      *out = nu::Cursor::Type::Help;
      return true;
    } else if (type == "move") {
      *out = nu::Cursor::Type::Move;
      return true;
    } else if (type == "resize-ew") {
      *out = nu::Cursor::Type::ResizeEW;
      return true;
    } else if (type == "resize-ns") {
      *out = nu::Cursor::Type::ResizeNS;
      return true;
    } else if (type == "resize-nesw") {
      *out = nu::Cursor::Type::ResizeNESW;
      return true;
    } else if (type == "resize-nwse") {
      *out = nu::Cursor::Type::ResizeNWSE;
      return true;
    } else {
      return false;
    }
  }
};

template<>
struct Type<nu::Cursor> {
  static constexpr const char* name = "Cursor";
  static void BuildMetaTable(State* state, int index) {
    RawSet(state, index,
           "createwithtype", &CreateOnHeap<nu::Cursor, nu::Cursor::Type>);
  }
};

template<>
struct Type<nu::DraggingInfo> {
  static constexpr const char* name = "DraggingInfo";
  static void BuildMetaTable(State* state, int index) {
    RawSet(state, index,
        "dragoperationnone", static_cast<int>(nu::DRAG_OPERATION_NONE),
        "dragoperationcopy", static_cast<int>(nu::DRAG_OPERATION_COPY),
        "dragoperationmove", static_cast<int>(nu::DRAG_OPERATION_MOVE),
        "dragoperationlink", static_cast<int>(nu::DRAG_OPERATION_LINK),
        "isdataavailable", &nu::DraggingInfo::IsDataAvailable,
        "getdata", &nu::DraggingInfo::GetData,
        "getdragoperations", &nu::DraggingInfo::GetDragOperations);
  }
};

template<>
struct Type<nu::DragOptions> {
  static constexpr const char* name = "DragOptions";
  static inline bool To(State* state, int index, nu::DragOptions* out) {
    if (GetType(state, index) != LuaType::Table)
      return false;
    return ReadOptions(state, index, "image", &out->image);
  }
};

template<>
struct Type<nu::Image> {
  static constexpr const char* name = "Image";
  static void BuildMetaTable(State* state, int index) {
    RawSet(state, index,
           "createempty", &CreateOnHeap<nu::Image>,
           "createfrompath", &CreateOnHeap<nu::Image, const base::FilePath&>,
           "createfrombuffer", &CreateOnHeap<nu::Image,
                                             const nu::Buffer&,
                                             float>,
           "isempty", &nu::Image::IsEmpty,
           "getsize", &nu::Image::GetSize,
           "getscalefactor", &nu::Image::GetScaleFactor);
  }
};

template<>
struct Type<nu::TextAlign> {
  static constexpr const char* name = "TextAlign";
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
  static inline void Push(State* state, nu::TextAlign align) {
    switch (align) {
      case nu::TextAlign::Center:
        lua::Push(state, "center");
        break;
      case nu::TextAlign::End:
        lua::Push(state, "end");
        break;
      default:
        lua::Push(state, "start");
        break;
    }
  }
};

template<>
struct Type<nu::TextFormat> {
  static constexpr const char* name = "TextFormat";
  static inline bool To(State* state, int index, nu::TextFormat* out) {
    if (GetType(state, index) != LuaType::Table)
      return false;
    return ReadOptions(state, index,
                       "align", &out->align, "valign", &out->valign,
                       "wrap", &out->wrap, "ellipsis", &out->ellipsis);
  }
  static inline void Push(State* state, const nu::TextFormat& options) {
    NewTable(state, 0, 4);
    RawSet(state, -1, "align", options.align, "valign", options.valign,
                      "wrap", options.wrap, "ellipsis", options.ellipsis);
  }
};

template<>
struct Type<nu::TextAttributes> {
  static constexpr const char* name = "TextAttributes";
  static inline bool To(State* state, int index, nu::TextAttributes* out) {
    if (!Type<nu::TextFormat>::To(state, index, out))
      return false;
    return ReadOptions(state, index,
                       "font", &out->font, "color", &out->color);
  }
};

template<>
struct Type<nu::Painter> {
  static constexpr const char* name = "Painter";
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
           "clear", &nu::Painter::Clear,
           "strokerect", &nu::Painter::StrokeRect,
           "fillrect", &nu::Painter::FillRect,
           "drawimage", &nu::Painter::DrawImage,
           "drawimagefromrect", &nu::Painter::DrawImageFromRect,
           "drawcanvas", &nu::Painter::DrawCanvas,
           "drawcanvasfromrect", &nu::Painter::DrawCanvasFromRect,
           "drawattributedtext", &nu::Painter::DrawAttributedText,
           "drawtext", &nu::Painter::DrawText);
  }
};

template<>
struct Type<nu::EventType> {
  static constexpr const char* name = "EventType";
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
  static constexpr const char* name = "Event";
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
  static constexpr const char* name = "MouseEvent";
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
  static constexpr const char* name = "KeyboardCode";
  static inline void Push(State* state, nu::KeyboardCode code) {
    lua::Push(state, nu::KeyboardCodeToStr(code));
  }
};

template<>
struct Type<nu::KeyEvent> {
  using base = nu::Event;
  static constexpr const char* name = "KeyEvent";
  static inline void Push(State* state, const nu::KeyEvent& event) {
    NewTable(state);
    Type<nu::Event>::SetEventProperties(state, -1, &event);
    RawSet(state, -1,
           "key", event.key);
  }
};

template<>
struct Type<nu::FileDialog::Filter> {
  static constexpr const char* name = "FileDialogFilter";
  static inline void Push(State* state,
                          const nu::FileDialog::Filter& filter) {
    NewTable(state, 0, 2);
    RawSet(state, -1,
           "description", std::get<0>(filter),
           "extensions", std::get<1>(filter));
  }
  static inline bool To(State* state, int index,
                        nu::FileDialog::Filter* out) {
    if (GetType(state, index) != LuaType::Table)
      return false;
    return RawGetAndPop(state, index,
                        "description", &std::get<0>(*out),
                        "extensions", &std::get<1>(*out));
  }
};

template<>
struct Type<nu::FileDialog> {
  static constexpr const char* name = "FileDialog";
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
  using base = nu::FileDialog;
  static constexpr const char* name = "FileOpenDialog";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "create", &CreateOnHeap<nu::FileOpenDialog>,
           "getresults", &nu::FileOpenDialog::GetResults);
  }
};

template<>
struct Type<nu::FileSaveDialog> {
  using base = nu::FileDialog;
  static constexpr const char* name = "FileSaveDialog";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "create", &CreateOnHeap<nu::FileSaveDialog>);
  }
};

template<>
struct Type<nu::MenuBase> {
  static constexpr const char* name = "MenuBase";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "append", RefMethod(state, &nu::MenuBase::Append, RefType::Ref),
           "insert", RefMethod(state, &Insert, RefType::Ref),
           "remove", RefMethod(state, &nu::MenuBase::Remove, RefType::Deref),
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
  static constexpr const char* name = "MenuBar";
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
  static constexpr const char* name = "Menu";
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
  static constexpr const char* name = "MenuItemType";
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
  static constexpr const char* name = "MenuItemRole";
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
    else if (role == "minimize")
      *out = nu::MenuItem::Role::Minimize;
    else if (role == "maximize")
      *out = nu::MenuItem::Role::Maximize;
    else if (role == "close-window")
      *out = nu::MenuItem::Role::CloseWindow;
#if defined(OS_MAC)
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
  static constexpr const char* name = "MenuItem";
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
#if defined(OS_MAC) || defined(OS_WIN)
           "setimage", &nu::MenuItem::SetImage,
           "getimage", &nu::MenuItem::GetImage,
#endif
           "setaccelerator", &nu::MenuItem::SetAccelerator);
    RawSetProperty(state, index,
                   "onclick", &nu::MenuItem::on_click,
                   "validate", &nu::MenuItem::validate);
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
        CallContext sub_context(state);
        sub_context.current_arg = AbsIndex(state, -1);
        submenu = Type<nu::Menu>::Create(&sub_context);
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
#if defined(OS_MAC) || defined(OS_WIN)
    nu::Image* image;
    if (RawGetAndPop(state, options, "image", &image))
      item->SetImage(image);
#endif
    std::function<void(nu::MenuItem*)> onclick;
    if (RawGetAndPop(state, options, "onclick", &onclick))
      item->on_click.Connect(onclick);
    std::function<bool(nu::MenuItem*)> validate;
    if (RawGetAndPop(state, options, "validate", &validate))
      item->validate = validate;
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
struct Type<nu::MessageBox::Type> {
  static constexpr const char* name = "MessageBoxType";
  static bool To(State* state, int index, nu::MessageBox::Type* out) {
    std::string type;
    if (!lua::To(state, index, &type))
      return false;
    if (type == "none")
      *out = nu::MessageBox::Type::None;
    else if (type == "information")
      *out = nu::MessageBox::Type::Information;
    else if (type == "warning")
      *out = nu::MessageBox::Type::Warning;
    else if (type == "error")
      *out = nu::MessageBox::Type::Error;
    else
      return false;
    return true;
  }
};

template<>
struct Type<nu::MessageBox> {
  static constexpr const char* name = "MessageBox";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "create", &CreateOnHeap<nu::MessageBox>,
           "run", &nu::MessageBox::Run,
           "runforwindow", &nu::MessageBox::RunForWindow,
#if defined(OS_LINUX) || defined(OS_WIN)
           "show", &nu::MessageBox::Show,
#endif
           "showforwindow", &nu::MessageBox::ShowForWindow,
           "close", &nu::MessageBox::Close,
           "settype", &nu::MessageBox::SetType,
#if defined(OS_LINUX) || defined(OS_WIN)
           "settitle", &nu::MessageBox::SetTitle,
#endif
           "addbutton", &nu::MessageBox::AddButton,
           "setdefaultresponse", &nu::MessageBox::SetDefaultResponse,
           "setcancelresponse", &nu::MessageBox::SetCancelResponse,
           "settext", &nu::MessageBox::SetText,
           "setinformativetext", &nu::MessageBox::SetInformativeText,
#if defined(OS_LINUX) || defined(OS_MAC)
           "setaccessoryview",
           RefMethod(state, &nu::MessageBox::SetAccessoryView,
                     RefType::Reset, "accv"),
           "getaccessoryview", &nu::MessageBox::GetAccessoryView,
#endif
           "setimage", &nu::MessageBox::SetImage,
           "getimage", &nu::MessageBox::GetImage);
    RawSetProperty(state, metatable,
                   "onresponse", &nu::MessageBox::on_response);
  }
};

template<>
struct Type<nu::Notification::Action> {
  static constexpr const char* name = "NotificationAction";
  static inline bool To(State* state, int index,
                        nu::Notification::Action* out) {
    if (GetType(state, index) != LuaType::Table)
      return false;
    return RawGetAndPop(state, index,
                        "info", &out->info,
                        "title", &out->title);
  }
};

template<>
struct Type<nu::Notification> {
  static constexpr const char* name = "Notification";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "create", &CreateOnHeap<nu::Notification>,
           "show", &nu::Notification::Show,
           "close", &nu::Notification::Close,
           "settitle", &nu::Notification::SetTitle,
           "setbody", &nu::Notification::SetBody,
           "setinfo", &nu::Notification::SetInfo,
           "getinfo", &nu::Notification::GetInfo,
           "setsilent", &nu::Notification::SetSilent,
           "setimage", &nu::Notification::SetImage,
           "setimagepath", &nu::Notification::SetImagePath,
#if defined(OS_MAC) || defined(OS_WIN)
           "sethasreplybutton", &nu::Notification::SetHasReplyButton,
           "setresponseplaceholder", &nu::Notification::SetResponsePlaceholder,
           "setidentifier", &nu::Notification::SetIdentifier,
           "getidentifier", &nu::Notification::GetIdentifier,
#endif
#if defined(OS_WIN)
           "setimageplacement", &nu::Notification::SetImagePlacement,
           "setxml", &nu::Notification::SetXML,
           "getxml", &nu::Notification::GetXML,
#endif
           "setactions", &nu::Notification::SetActions);
  }
};

#if defined(OS_WIN)
template<>
struct Type<nu::NotificationCenter::COMServerOptions> {
  static constexpr const char* name = "NotificationCenterCOMServerOptions";
  static inline bool To(State* state, int index,
                        nu::NotificationCenter::COMServerOptions* out) {
    if (GetType(state, index) != LuaType::Table)
      return false;
    return ReadOptions(state, index,
                       "writeregistry", &out->write_registry,
                       "arguments", &out->arguments,
                       "toastactivatorclsid", &out->toast_activator_clsid);
  }
};

template<>
struct Type<nu::NotificationCenter::InputData> {
  static constexpr const char* name = "NotificationCenterInputData";
  static inline void Push(State* state,
                          const nu::NotificationCenter::InputData& data) {
    lua::NewTable(state);
    lua::RawSet(state, -1, "key", data.key, "value", data.value);
  }
};
#endif

template<>
struct Type<nu::NotificationCenter> {
  static constexpr const char* name = "NotificationCenter";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "clear", &nu::NotificationCenter::Clear);
#if defined(OS_WIN)
    RawSet(state, metatable,
           "setcomserveroptions",
           &nu::NotificationCenter::SetCOMServerOptions,
           "registercomserver",
           &nu::NotificationCenter::RegisterCOMServer,
           "removecomserverfromregistry",
           &nu::NotificationCenter::RemoveCOMServerFromRegistry,
           "gettoastactivatorclsid",
           &nu::NotificationCenter::GetToastActivatorCLSID);
#endif
    RawSetProperty(state, metatable,
                   "onnotificationshow",
                   &nu::NotificationCenter::on_notification_show,
                   "onnotificationclose",
                   &nu::NotificationCenter::on_notification_close,
                   "onnotificationclick",
                   &nu::NotificationCenter::on_notification_click,
                   "onnotificationaction",
                   &nu::NotificationCenter::on_notification_action);
#if defined(OS_MAC) || defined(OS_WIN)
    RawSetProperty(state, metatable,
                   "onnotificationreply",
                   &nu::NotificationCenter::on_notification_reply);
#endif
#if defined(OS_WIN)
    RawSetProperty(state, metatable,
                   "ontoastactivate",
                   &nu::NotificationCenter::on_toast_activate);
#endif
  }
};

template<>
struct Type<nu::Tray> {
  static constexpr const char* name = "Tray";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
#if defined(OS_MAC)
           "createwithtitle", &CreateOnHeap<nu::Tray, const std::string&>,
#endif
           "createwithimage", &CreateOnHeap<nu::Tray, scoped_refptr<nu::Image>>,
           "remove", &nu::Tray::Remove,
#if defined(OS_MAC) || defined(OS_WIN)
           "getbounds", &nu::Tray::GetBounds,
#endif
#if defined(OS_MAC) || defined(OS_LINUX)
           "settitle", &nu::Tray::SetTitle,
#endif
           "setimage", &nu::Tray::SetImage,
#if defined(OS_MAC)
           "setpressedimage", &nu::Tray::SetPressedImage,
#endif
           "setmenu",
           RefMethod(state, &nu::Tray::SetMenu, RefType::Reset, "menu"));
    RawSetProperty(state, metatable, "onclick", &nu::Tray::on_click);
  }
};

#if defined(OS_MAC)
template<>
struct Type<nu::Toolbar::Item> {
  static constexpr const char* name = "Toolbar::Item";
  static inline bool To(State* state, int index, nu::Toolbar::Item* out) {
    if (GetType(state, index) != LuaType::Table)
      return false;
    return ReadOptions(state, index,
                       "image", &out->image,
                       "view", &out->view,
                       "label", &out->label,
                       "minsize", &out->min_size,
                       "maxsize", &out->max_size,
                       "subitems", &out->subitems,
                       "onclick", &out->on_click);
  }
};

template<>
struct Type<nu::Toolbar::DisplayMode> {
  static constexpr const char* name = "ToolbarDisplayMode";
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
  static constexpr const char* name = "Toolbar";
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
struct Type<nu::Responder> {
  static constexpr const char* name = "Responder";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "setcapture", &nu::Responder::SetCapture,
           "releasecapture", &nu::Responder::ReleaseCapture,
           "hascapture", &nu::Responder::HasCapture);
    RawSetProperty(state, metatable,
                   "onmousedown", &nu::Responder::on_mouse_down,
                   "onmouseup", &nu::Responder::on_mouse_up,
                   "onmousemove", &nu::Responder::on_mouse_move,
                   "onmouseenter", &nu::Responder::on_mouse_enter,
                   "onmouseleave", &nu::Responder::on_mouse_leave,
                   "onkeydown", &nu::Responder::on_key_down,
                   "onkeyup", &nu::Responder::on_key_up,
                   "oncapturelost", &nu::View::on_capture_lost);
  }
};

template<>
struct Type<nu::Window::Options> {
  static constexpr const char* name = "WindowOptions";
  static inline bool To(State* state, int index, nu::Window::Options* out) {
    if (GetType(state, index) != LuaType::Table)
      return false;
    return ReadOptions(state, index,
                       "frame", &out->frame,
#if defined(OS_MAC)
                       "showtrafficlights", &out->show_traffic_lights,
#endif
                       "transparent", &out->transparent);
  }
};

template<>
struct Type<nu::Window> {
  using base = nu::Responder;
  static constexpr const char* name = "Window";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "create", &CreateOnHeap<nu::Window, const nu::Window::Options&>,
           "close", &nu::Window::Close,
           "sethasshadow", &nu::Window::SetHasShadow,
           "hasshadow", &nu::Window::HasShadow,
           "center", &nu::Window::Center,
           "setcontentview",
           RefMethod(state, &nu::Window::SetContentView,
                     RefType::Reset, "content"),
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
#if defined(OS_MAC)
           "settoolbar", &nu::Window::SetToolbar,
           "gettoolbar", &nu::Window::GetToolbar,
           "settitlevisible", &nu::Window::SetTitleVisible,
           "istitlevisible", &nu::Window::IsTitleVisible,
           "setfullsizecontentview", &nu::Window::SetFullSizeContentView,
           "isfullsizecontentview", &nu::Window::IsFullSizeContentView,
#endif
#if defined(OS_WIN) || defined(OS_LINUX)
           "setskiptaskbar", &nu::Window::SetSkipTaskbar,
           "seticon", &nu::Window::SetIcon,
           "setmenubar",
           RefMethod(state, &nu::Window::SetMenuBar, RefType::Reset, "menubar"),
           "getmenubar", &nu::Window::GetMenuBar,
#endif
           "addchildwindow",
           RefMethod(state, &nu::Window::AddChildWindow, RefType::Ref),
           "removechildview",
           RefMethod(state, &nu::Window::RemoveChildWindow, RefType::Deref),
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
  using base = nu::Responder;
  static constexpr const char* name = "View";
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
           "setmousedowncanmovewindow", &nu::View::SetMouseDownCanMoveWindow,
           "ismousedowncanmovewindow", &nu::View::IsMouseDownCanMoveWindow,
           "dodrag", &nu::View::DoDrag,
           "dodragwithoptions", &nu::View::DoDragWithOptions,
           "canceldrag", &nu::View::CancelDrag,
           "isdragging", &nu::View::IsDragging,
           "registerdraggedtypes", &nu::View::RegisterDraggedTypes,
           "setcursor", &nu::View::SetCursor,
           "setfont", &nu::View::SetFont,
           "setcolor", &nu::View::SetColor,
           "setbackgroundcolor", &nu::View::SetBackgroundColor,
           "setstyle", &SetStyle,
           "getcomputedlayout", &nu::View::GetComputedLayout,
           "getminimumsize", &nu::View::GetMinimumSize,
#if defined(OS_MAC)
           "setwantslayer", &nu::View::SetWantsLayer,
           "wantslayer", &nu::View::WantsLayer,
#endif
           "getparent", &nu::View::GetParent,
           "getwindow", &nu::View::GetWindow);
    RawSetProperty(state, metatable,
                   "ondragleave", &nu::View::on_drag_leave,
                   "onsizechanged", &nu::View::on_size_changed,
                   "handledragenter", &nu::View::handle_drag_enter,
                   "handledragupdate", &nu::View::handle_drag_update,
                   "handledrop", &nu::View::handle_drop);
  }
  static void SetStyle(nu::View* view,
                       const std::map<std::string, std::string>& styles) {
    for (const auto& it : styles)
      view->SetStyleProperty(it.first, it.second);
    view->Layout();
  }
};
template<>
struct Type<nu::ComboBox> {
  using base = nu::Picker;
  static constexpr const char* name = "ComboBox";
  static void BuildMetaTable(State* state, int index) {
    RawSet(state, index,
           "create", &CreateOnHeap<nu::ComboBox>,
           "settext", &nu::ComboBox::SetText,
           "gettext", &nu::ComboBox::GetText);
    RawSetProperty(state, index,
                   "ontextchange", &nu::ComboBox::on_text_change);
  }
};

template<>
struct Type<nu::Container> {
  using base = nu::View;
  static constexpr const char* name = "Container";
  static void BuildMetaTable(State* state, int index) {
    RawSet(state, index,
           "create", &CreateOnHeap<nu::Container>,
           "getpreferredsize", &nu::Container::GetPreferredSize,
           "getpreferredwidthforheight",
           &nu::Container::GetPreferredWidthForHeight,
           "getpreferredheightforwidth",
           &nu::Container::GetPreferredHeightForWidth,
           "addchildview",
           RefMethod(state, &nu::Container::AddChildView, RefType::Ref),
           "addchildviewat",
           RefMethod(state, &AddChildViewAt, RefType::Ref),
           "removechildview",
           RefMethod(state, &nu::Container::RemoveChildView, RefType::Deref),
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
  static constexpr const char* name = "ButtonType";
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

#if defined(OS_MAC)
template<>
struct Type<nu::Button::Style> {
  static constexpr const char* name = "ButtonStyle";
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
  static constexpr const char* name = "Button";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "create", &Create,
           "makedefault", &nu::Button::MakeDefault,
           "settitle", &nu::Button::SetTitle,
           "gettitle", &nu::Button::GetTitle,
#if defined(OS_MAC)
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
      nu::Button::Type type = nu::Button::Type::Normal;
      ReadOptions(context->state, 1, "title", &title, "type", &type);
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
  static constexpr const char* name = "ProtocolJob";
  static void BuildMetaTable(State* state, int metatable) {
  }
};

template<>
struct Type<nu::ProtocolStringJob> {
  using base = nu::ProtocolJob;
  static constexpr const char* name = "ProtocolStringJob";
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
  static constexpr const char* name = "ProtocolFileJob";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "create", &CreateOnHeap<nu::ProtocolFileJob,
                                   const ::base::FilePath&>);
  }
};

template<>
struct Type<nu::ProtocolAsarJob> {
  using base = nu::ProtocolFileJob;
  static constexpr const char* name = "ProtocolAsarJob";
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
  static constexpr const char* name = "BrowserOptions";
  static inline bool To(State* state, int index, nu::Browser::Options* out) {
    if (GetType(state, index) != LuaType::Table)
      return false;
    return ReadOptions(
        state, index,
#if defined(OS_MAC) || defined(OS_LINUX)
        "allowfileaccessfromfiles", &out->allow_file_access_from_files,
#endif
#if defined(OS_LINUX)
        "hardwareacceleration", &out->hardware_acceleration,
#endif
#if defined(OS_WIN) && defined(WEBVIEW2_SUPPORT)
        "webview2support", &out->webview2_support,
#endif
        "devtools", &out->devtools,
        "contextmenu", &out->context_menu);
  }
};

template<>
struct Type<nu::Browser> {
  using base = nu::View;
  static constexpr const char* name = "Browser";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "create", &CreateOnHeap<nu::Browser, nu::Browser::Options>,
           "registerprotocol", &nu::Browser::RegisterProtocol,
           "unregisterprotocol", &nu::Browser::UnregisterProtocol,
           "loadurl", &nu::Browser::LoadURL,
           "loadhtml", &nu::Browser::LoadHTML,
           "geturl", &nu::Browser::GetURL,
           "gettitle", &nu::Browser::GetTitle,
           "setuseragent", &nu::Browser::SetUserAgent,
#if defined(OS_MAC)
           "ismagnifiable", &nu::Browser::IsMagnifiable,
           "setmagnifiable", &nu::Browser::SetMagnifiable,
#endif
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
                         const std::string& bname) {
    State* state = context->state;
    if (GetType(state, 3) != LuaType::Function) {
      Push(state, "The arg 3 should be function");
      context->has_error = true;
      return;
    }
    // Persistent the function and pass it to lambda.
    auto ref = std::make_shared<Persistent>(state, 3);
    browser->AddRawBinding(bname, [state, ref](nu::Browser* browser,
                                               ::base::Value value) {
      ref->Push();
      for (const auto& it : value.GetList())
        Push(state, it);
      lua_pcall(state, static_cast<int>(value.GetList().size()), 0, 0);
    });
  }
};

template<>
struct Type<nu::DatePicker::Options> {
  static constexpr const char* name = "DatePickerOptions";
  static inline bool To(State* state, int index, nu::DatePicker::Options* out) {
    if (GetType(state, index) != LuaType::Table)
      return false;
    return ReadOptions(state, index,
                       "elements", &out->elements,
                       "hasstepper", &out->has_stepper);
  }
};

template<>
struct Type<nu::DatePicker> {
  using base = nu::View;
  static constexpr const char* name = "DatePicker";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "elementyearmonth",
           static_cast<int>(nu::DatePicker::ELEMENT_YEAR_MONTH),
           "elementyearmonthday",
           static_cast<int>(nu::DatePicker::ELEMENT_YEAR_MONTH_DAY),
           "elementhourminute",
           static_cast<int>(nu::DatePicker::ELEMENT_HOUR_MINUTE),
           "elementhourminutesecond",
           static_cast<int>(nu::DatePicker::ELEMENT_HOUR_MINUTE_SECOND),
           "create",
           &CreateOnHeap<nu::DatePicker, const nu::DatePicker::Options&>,
           "setdate", &nu::DatePicker::SetDate,
           "getdate", &nu::DatePicker::GetDate,
           "setrange", &nu::DatePicker::SetRange,
           "getrange", &nu::DatePicker::GetRange,
           "hasstepper", &nu::DatePicker::HasStepper);
    RawSetProperty(state, metatable,
                   "ondatechange", &nu::DatePicker::on_date_change);
  }
};

template<>
struct Type<nu::Entry::Type> {
  static constexpr const char* name = "EntryType";
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
  static constexpr const char* name = "Entry";
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
  static constexpr const char* name = "Label";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "create", &CreateOnHeap<nu::Label, const std::string&>,
           "createwithattributedtext",
           &CreateOnHeap<nu::Label, nu::AttributedText*>,
           "settext", &nu::Label::SetText,
           "gettext", &nu::Label::GetText,
           "setalign", &nu::Label::SetAlign,
           "setvalign", &nu::Label::SetVAlign,
           "setattributedtext",
           RefMethod(state, &nu::Label::SetAttributedText,
                     RefType::Reset, "atext"),
           "getattributedtext", &nu::Label::GetAttributedText);
  }
};

template<>
struct Type<nu::ProgressBar> {
  using base = nu::View;
  static constexpr const char* name = "ProgressBar";
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
struct Type<nu::Picker> {
  using base = nu::View;
  static constexpr const char* name = "Picker";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "create", &CreateOnHeap<nu::Picker>,
           "additem", &nu::Picker::AddItem,
           "removeitemat", &RemoveItemAt,
           "getitems", &nu::Picker::GetItems,
           "selectitemat", &SelectItemAt,
           "getselecteditem", &nu::Picker::GetSelectedItem,
           "getselecteditemindex", &GetSelectedItemIndex);
    RawSetProperty(state, metatable,
                   "onselectionchange", &nu::Picker::on_selection_change);
  }
  static void RemoveItemAt(nu::Picker* picker, int i) {
    return picker->RemoveItemAt(i - 1);
  }
  static void SelectItemAt(nu::Picker* picker, int i) {
    return picker->SelectItemAt(i - 1);
  }
  static int GetSelectedItemIndex(nu::Picker* picker) {
    int index = picker->GetSelectedItemIndex();
    return index == -1 ? -1 : index + 1;
  }
};

template<>
struct Type<nu::GifPlayer> {
  using base = nu::View;
  static constexpr const char* name = "GifPlayer";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "create", &CreateOnHeap<nu::GifPlayer>,
           "setimage", &nu::GifPlayer::SetImage,
           "getimage", &nu::GifPlayer::GetImage,
           "setanimating", &nu::GifPlayer::SetAnimating,
           "isanimating", &nu::GifPlayer::IsAnimating,
           "setscale", &nu::GifPlayer::SetScale,
           "getscale", &nu::GifPlayer::GetScale);
  }
};

template<>
struct Type<nu::Group> {
  using base = nu::View;
  static constexpr const char* name = "Group";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "create", &CreateOnHeap<nu::Group, const std::string&>,
           "setcontentview",
           RefMethod(state, &nu::Group::SetContentView,
                     RefType::Reset, "content"),
           "getcontentview", &nu::Group::GetContentView,
           "settitle", &nu::Group::SetTitle,
           "gettitle", &nu::Group::GetTitle);
  }
};

template<>
struct Type<nu::Screen> {
  static constexpr const char* name = "Screen";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "getprimarydisplay", &nu::Screen::GetPrimaryDisplay,
           "getalldisplays", &nu::Screen::GetAllDisplays,
           "getdisplaynearestwindow", &nu::Screen::GetDisplayNearestWindow,
           "getdisplaynearestpoint", &nu::Screen::GetDisplayNearestPoint,
           "getcursorscreenpoint", &nu::Screen::GetCursorScreenPoint);
    RawSetProperty(state, metatable,
                   "onadddisplay", &nu::Screen::on_add_display,
                   "onremovedisplay", &nu::Screen::on_remove_display,
                   "onupdatedisplay", &nu::Screen::on_update_display);
  }
};

template<>
struct Type<nu::Scroll::Policy> {
  static constexpr const char* name = "ScrollPolicy";
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

#if defined(OS_MAC)
template<>
struct Type<nu::Scroll::Elasticity> {
  static constexpr const char* name = "ScrollElasticity";
  static inline bool To(State* state, int index, nu::Scroll::Elasticity* out) {
    std::string elasticity;
    if (!lua::To(state, index, &elasticity))
      return false;
    if (elasticity == "automatic") {
      *out = nu::Scroll::Elasticity::Automatic;
      return true;
    } else if (elasticity == "none") {
      *out = nu::Scroll::Elasticity::None;
      return true;
    } else if (elasticity == "allowed") {
      *out = nu::Scroll::Elasticity::Allowed;
      return true;
    } else {
      return false;
    }
  }
  static inline void Push(State* state, nu::Scroll::Elasticity elasticity) {
    if (elasticity == nu::Scroll::Elasticity::Automatic)
      lua::Push(state, "automatic");
    else if (elasticity == nu::Scroll::Elasticity::None)
      lua::Push(state, "none");
    else
      lua::Push(state, "allowed");
  }
};
#endif

template<>
struct Type<nu::Scroll> {
  using base = nu::View;
  static constexpr const char* name = "Scroll";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "create", &CreateOnHeap<nu::Scroll>,
           "setcontentsize", &nu::Scroll::SetContentSize,
           "getcontentsize", &nu::Scroll::GetContentSize,
           "setscrollposition", &nu::Scroll::SetScrollPosition,
           "getscrollposition", &nu::Scroll::GetScrollPosition,
           "getmaximumscrollposition", &nu::Scroll::GetMaximumScrollPosition,
           "setcontentview",
           RefMethod(state, &nu::Scroll::SetContentView,
                     RefType::Reset, "content"),
           "getcontentview", &nu::Scroll::GetContentView,
#if !defined(OS_WIN)
           "setoverlayscrollbar", &nu::Scroll::SetOverlayScrollbar,
           "isoverlayscrollbar", &nu::Scroll::IsOverlayScrollbar,
#endif
#if defined(OS_MAC)
           "setscrollelasticity", &nu::Scroll::SetScrollElasticity,
           "getscrollelasticity", &nu::Scroll::GetScrollElasticity,
#endif
           "setscrollbarpolicy", &nu::Scroll::SetScrollbarPolicy,
           "getscrollbarpolicy", &nu::Scroll::GetScrollbarPolicy);
    RawSetProperty(state, metatable,
                   "onscroll", &nu::Scroll::on_scroll);
  }
};

template<>
struct Type<nu::Separator> {
  using base = nu::View;
  static constexpr const char* name = "Separator";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "create", &CreateOnHeap<nu::Separator, nu::Orientation>);
  }
};

template<>
struct Type<nu::Slider> {
  using base = nu::View;
  static constexpr const char* name = "Slider";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "create", &CreateOnHeap<nu::Slider>,
           "setvalue", &nu::Slider::SetValue,
           "getvalue", &nu::Slider::GetValue,
           "setstep", &nu::Slider::SetStep,
           "getstep", &nu::Slider::GetStep,
           "setrange", &nu::Slider::SetRange,
           "getrange", &nu::Slider::GetRange);
    RawSetProperty(state, metatable,
                   "onvaluechange", &nu::Slider::on_value_change,
                   "onslidingcomplete", &nu::Slider::on_sliding_complete);
  }
};

template<>
struct Type<nu::Tab> {
  using base = nu::View;
  static constexpr const char* name = "Tab";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "create", &CreateOnHeap<nu::Tab>,
           "addpage", RefMethod(state, &nu::Tab::AddPage, RefType::Ref),
           "removePage", RefMethod(state, &nu::Tab::RemovePage, RefType::Deref),
           "pagecount", &nu::Tab::PageCount,
           "pageat", &PageAt,
           "selectpageat", &SelectPageAt,
           "getselectedpage", &nu::Tab::GetSelectedPage,
           "getselectedpageindex", &GetSelectedPageIndex);
    RawSetProperty(state, metatable,
                   "onselectedpagechange", &nu::Tab::on_selected_page_change);
  }
  static nu::View* PageAt(nu::Tab* tab, int index) {
    return tab->PageAt(index - 1);
  }
  static void SelectPageAt(nu::Tab* tab, int index) {
    tab->SelectPageAt(index - 1);
  }
  static int GetSelectedPageIndex(nu::Tab* tab) {
    int index = tab->GetSelectedPageIndex();
    return index == -1 ? -1 : index + 1;
  }
};

template<>
struct Type<nu::TableModel> {
  static constexpr const char* name = "TableModel";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "getrowcount", &nu::TableModel::GetRowCount,
           "setvalue", &SetValue,
           "getvalue", &GetValue,
           "notifyrowinsertion", &NotifyRowInsertion,
           "notifyrowdeletion", &NotifyRowDeletion,
           "notifyvaluechange", &NotifyValueChange);
  }
  static void SetValue(nu::TableModel* model,
                       uint32_t column,
                       uint32_t row,
                       ::base::Value value) {
    model->SetValue(column - 1, row - 1, std::move(value));
  }
  static const base::Value* GetValue(
      nu::TableModel* model, uint32_t column, uint32_t row) {
    return model->GetValue(column - 1, row - 1);
  }
  static void NotifyRowInsertion(nu::TableModel* model, uint32_t row) {
    model->NotifyRowInsertion(row - 1);
  }
  static void NotifyRowDeletion(nu::TableModel* model, uint32_t row) {
    model->NotifyRowDeletion(row - 1);
  }
  static void NotifyValueChange(nu::TableModel* model,
                              uint32_t module, uint32_t row) {
    model->NotifyValueChange(module - 1, row - 1);
  }
};

template<>
struct Type<nu::AbstractTableModel> {
  using base = nu::TableModel;
  static constexpr const char* name = "AbstractTableModel";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable, "create", &Create);
    RawSetProperty(state, metatable,
                   "getrowcount", &nu::AbstractTableModel::get_row_count,
                   "setvalue", &nu::AbstractTableModel::set_value,
                   "getvalue", &nu::AbstractTableModel::get_value);
  }
  static nu::AbstractTableModel* Create() {
    return new nu::AbstractTableModel(false /* index_starts_from_0 */);
  }
};

template<>
struct Type<nu::SimpleTableModel> {
  using base = nu::TableModel;
  static constexpr const char* name = "SimpleTableModel";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "create", &CreateOnHeap<nu::SimpleTableModel, uint32_t>,
           "addrow", &nu::SimpleTableModel::AddRow,
           "removerowat", &RemoveRowAt);
  }
  static void RemoveRowAt(nu::SimpleTableModel* model, uint32_t row) {
    model->RemoveRowAt(row - 1);
  }
};

template<>
struct Type<nu::Table::ColumnType> {
  static constexpr const char* name = "TableColumnType";
  static inline bool To(State* state, int index, nu::Table::ColumnType* out) {
    std::string type;
    if (!lua::To(state, index, &type))
      return false;
    if (type == "text") {
      *out = nu::Table::ColumnType::Text;
      return true;
    } else if (type == "edit") {
      *out = nu::Table::ColumnType::Edit;
      return true;
    } else if (type == "custom") {
      *out = nu::Table::ColumnType::Custom;
      return true;
    } else {
      return false;
    }
  }
};

template<>
struct Type<nu::Table::ColumnOptions> {
  static constexpr const char* name = "TableColumnOptions";
  static inline bool To(State* state, int index,
                        nu::Table::ColumnOptions* out) {
    if (GetType(state, index) != LuaType::Table)
      return false;
    int column;
    if (RawGetAndPop(state, index, "column", &column))
      out->column = column - 1;
    return ReadOptions(state, index,
                       "type", &out->type,
                       "ondraw", &out->on_draw,
                       "width", &out->width);
  }
};

template<>
struct Type<nu::Table> {
  using base = nu::View;
  static constexpr const char* name = "Table";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "create", &CreateOnHeap<nu::Table>,
           "setmodel",
           RefMethod(state, &nu::Table::SetModel, RefType::Reset, "model"),
           "getmodel", &nu::Table::GetModel,
           "addcolumn", &nu::Table::AddColumn,
           "addcolumnwithoptions",
           RefMethod(state, &nu::Table::AddColumnWithOptions,
                     RefType::Ref, nullptr, 2),
           "getcolumncount", &nu::Table::GetColumnCount,
           "setcolumnsvisible", &nu::Table::SetColumnsVisible,
           "iscolumnsvisible", &nu::Table::IsColumnsVisible,
           "setrowheight", &nu::Table::SetRowHeight,
           "getrowheight", &nu::Table::GetRowHeight,
           "selectrow", &SelectRow,
           "getselectedrow", &GetSelectedRow);
  }
  static void SelectRow(nu::Table* table, int row) {
    table->SelectRow(row - 1);
  }
  static int GetSelectedRow(nu::Table* table) {
    int index = table->GetSelectedRow();
    return index == -1 ? -1 : index + 1;
  }
};

template<>
struct Type<nu::TextEdit> {
  using base = nu::View;
  static constexpr const char* name = "TextEdit";
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
#if defined(OS_MAC)
           "setscrollelasticity", &nu::TextEdit::SetScrollElasticity,
           "getscrollelasticity", &nu::TextEdit::GetScrollElasticity,
#endif
           "gettextbounds", &nu::TextEdit::GetTextBounds);
    RawSetProperty(state, metatable,
                   "ontextchange", &nu::TextEdit::on_text_change,
                   "shouldinsertnewline",
                   &nu::TextEdit::should_insert_new_line);
  }
};

#if defined(OS_MAC)
template<>
struct Type<nu::Vibrant::Material> {
  static constexpr const char* name = "VibrantMaterial";
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
  static constexpr const char* name = "VibrantBlendingMode";
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
  static constexpr const char* name = "Vibrant";
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
  // The exports table.
  lua::NewTable(state);

  // Initialize nativeui.
  lua::Push(state, "__lifetime_ptr");
  lua::NewUserData<nu::Lifetime>(state);
  lua_rawset(state, -3);
  lua::Push(state, "__state_ptr");
  lua::NewUserData<nu::State>(state);
  lua_rawset(state, -3);

  // Classes.
  BindType<nu::Lifetime>(state, "Lifetime");
  BindType<nu::MessageLoop>(state, "MessageLoop");
  BindType<nu::App>(state, "App");
  BindType<nu::Appearance>(state, "Appearance");
  BindType<nu::AttributedText>(state, "AttributedText");
  BindType<nu::Font>(state, "Font");
  BindType<nu::Canvas>(state, "Canvas");
  BindType<nu::Clipboard>(state, "Clipboard");
  BindType<nu::Color>(state, "Color");
  BindType<nu::Cursor>(state, "Cursor");
  BindType<nu::DraggingInfo>(state, "DraggingInfo");
  BindType<nu::Image>(state, "Image");
  BindType<nu::Painter>(state, "Painter");
  BindType<nu::Event>(state, "Event");
  BindType<nu::FileDialog>(state, "FileDialog");
  BindType<nu::FileOpenDialog>(state, "FileOpenDialog");
  BindType<nu::FileSaveDialog>(state, "FileSaveDialog");
  BindType<nu::MenuBar>(state, "MenuBar");
  BindType<nu::Menu>(state, "Menu");
  BindType<nu::MenuItem>(state, "MenuItem");
  BindType<nu::MessageBox>(state, "MessageBox");
  BindType<nu::Notification>(state, "Notification");
  BindType<nu::NotificationCenter>(state, "NotificationCenter");
  BindType<nu::Responder>(state, "Responder");
  BindType<nu::Window>(state, "Window");
  BindType<nu::View>(state, "View");
  BindType<nu::ComboBox>(state, "ComboBox");
  BindType<nu::Container>(state, "Container");
  BindType<nu::Button>(state, "Button");
  BindType<nu::ProtocolStringJob>(state, "ProtocolStringJob");
  BindType<nu::ProtocolFileJob>(state, "ProtocolFileJob");
  BindType<nu::ProtocolAsarJob>(state, "ProtocolAsarJob");
  BindType<nu::Browser>(state, "Browser");
  BindType<nu::DatePicker>(state, "DatePicker");
  BindType<nu::Entry>(state, "Entry");
  BindType<nu::Label>(state, "Label");
  BindType<nu::Picker>(state, "Picker");
  BindType<nu::ProgressBar>(state, "ProgressBar");
  BindType<nu::GifPlayer>(state, "GifPlayer");
  BindType<nu::Group>(state, "Group");
  BindType<nu::Screen>(state, "Screen");
  BindType<nu::Scroll>(state, "Scroll");
  BindType<nu::Separator>(state, "Separator");
  BindType<nu::Slider>(state, "Slider");
  BindType<nu::Tab>(state, "Tab");
  BindType<nu::TableModel>(state, "TableModel");
  BindType<nu::AbstractTableModel>(state, "AbstractTableModel");
  BindType<nu::SimpleTableModel>(state, "SimpleTableModel");
  BindType<nu::Table>(state, "Table");
  BindType<nu::TextEdit>(state, "TextEdit");
  BindType<nu::Tray>(state, "Tray");
#if defined(OS_MAC)
  BindType<nu::Toolbar>(state, "Toolbar");
  BindType<nu::Vibrant>(state, "Vibrant");
#endif
  // Properties.
  lua::RawSet(state, -1,
              "lifetime",           nu::Lifetime::GetCurrent(),
              "app",                nu::App::GetCurrent(),
              "appearance",         nu::Appearance::GetCurrent(),
              "notificationcenter", nu::NotificationCenter::GetCurrent(),
              "screen",             nu::Screen::GetCurrent());
  return 1;
}
