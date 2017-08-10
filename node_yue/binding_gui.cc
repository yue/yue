// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include <node.h>

#include "nativeui/nativeui.h"
#include "node_yue/binding_signal.h"
#include "node_yue/node_integration.h"

namespace vb {

template<>
struct Type<base::FilePath> {
  static constexpr const char* name = "yue.FilePath";
  static v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                   const base::FilePath& value) {
    return vb::ToV8(context, value.value());;
  }
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     base::FilePath* out) {
    base::FilePath::StringType str;
    if (!vb::FromV8(context, value, &str))
      return false;
    *out = base::FilePath(str);
    return true;
  }
};

template<>
struct Type<nu::Size> {
  static constexpr const char* name = "yue.Size";
  static v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                   const nu::Size& value) {
    auto obj = v8::Object::New(context->GetIsolate());
    Set(context, obj, "width", value.width(), "height", value.height());
    return obj;
  }
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::Size* out) {
    if (!value->IsObject())
      return false;
    int width = 0, height = 0;
    Get(context, value.As<v8::Object>(), "width", &width, "height", &height);
    *out = nu::Size(width, height);
    return true;
  }
};

template<>
struct Type<nu::SizeF> {
  static constexpr const char* name = "yue.SizeF";
  static v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                   const nu::SizeF& value) {
    auto obj = v8::Object::New(context->GetIsolate());
    Set(context, obj, "width", value.width(), "height", value.height());
    return obj;
  }
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::SizeF* out) {
    if (!value->IsObject())
      return false;
    float width = 0, height = 0;
    Get(context, value.As<v8::Object>(), "width", &width, "height", &height);
    *out = nu::SizeF(width, height);
    return true;
  }
};

template<>
struct Type<nu::RectF> {
  static constexpr const char* name = "yue.RectF";
  static v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                   const nu::RectF& value) {
    auto obj = v8::Object::New(context->GetIsolate());
    Set(context, obj, "x", value.x(), "y", value.y(),
        "width", value.width(), "height", value.height());
    return obj;
  }
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::RectF* out) {
    if (!value->IsObject())
      return false;
    float x = 0, y = 0, width = 0, height = 0;
    Get(context, value.As<v8::Object>(), "x", &x, "y", &y);
    Get(context, value.As<v8::Object>(), "width", &width, "height", &height);
    *out = nu::RectF(x, y, width, height);
    return true;
  }
};

template<>
struct Type<nu::Vector2dF> {
  static constexpr const char* name = "yue.Vector2dF";
  static v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                   const nu::Vector2dF& value) {
    auto obj = v8::Object::New(context->GetIsolate());
    Set(context, obj, "x", value.x(), "y", value.y());
    return obj;
  }
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::Vector2dF* out) {
    if (!value->IsObject())
      return false;
    float x = 0, y = 0;
    Get(context, value.As<v8::Object>(), "x", &x, "y", &y);
    *out = nu::Vector2dF(x, y);
    return true;
  }
};

template<>
struct Type<nu::PointF> {
  static constexpr const char* name = "yue.PointF";
  static v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                   const nu::PointF& value) {
    auto obj = v8::Object::New(context->GetIsolate());
    Set(context, obj, "x", value.x(), "y", value.y());
    return obj;
  }
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::PointF* out) {
    if (!value->IsObject())
      return false;
    float x = 0, y = 0;
    Get(context, value.As<v8::Object>(), "x", &x, "y", &y);
    *out = nu::PointF(x, y);
    return true;
  }
};

template<>
struct Type<nu::Accelerator> {
  static constexpr const char* name = "yue.Accelerator";
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::Accelerator* out) {
    std::string description;
    if (!vb::FromV8(context, value, &description))
      return false;
    *out = nu::Accelerator(description);
    return true;
  }
};

#ifndef ELECTRON_BUILD
template<>
struct Type<nu::Lifetime> {
  static constexpr const char* name = "yue.Lifetime";
  static void BuildConstructor(v8::Local<v8::Context>, v8::Local<v8::Object>) {
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "run", &nu::Lifetime::Run,
        "quit", &nu::Lifetime::Quit,
        "postTask", &nu::Lifetime::PostTask,
        "postDelayedTask", &nu::Lifetime::PostDelayedTask);
    SetProperty(context, templ,
                "onReady", &nu::Lifetime::on_ready);
  }
};
#endif

template<>
struct Type<nu::App::ThemeColor> {
  static constexpr const char* name = "yue.ThemeColor";
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::App::ThemeColor* out) {
    std::string id;
    if (!vb::FromV8(context, value, &id))
      return false;
    if (id == "text")
      *out = nu::App::ThemeColor::Text;
    else
      return false;
    return true;
  }
};

template<>
struct Type<nu::App> {
  static constexpr const char* name = "yue.App";
  static void BuildConstructor(v8::Local<v8::Context>, v8::Local<v8::Object>) {
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
#if defined(OS_MACOSX)
        "setApplicationMenu", &nu::App::SetApplicationMenu,
#endif
        "getColor", &nu::App::GetColor,
        "getDefaultFont", &nu::App::GetDefaultFont);
  }
};

template<>
struct Type<nu::Font::Weight> {
  static constexpr const char* name = "yue.Font.Weight";
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::Font::Weight* out) {
    std::string weight;
    if (!vb::FromV8(context, value, &weight))
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
  static v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                   nu::Font::Weight weight) {
    switch (weight) {
      case nu::Font::Weight::Thin:
        return vb::ToV8(context, "thin");
      case nu::Font::Weight::ExtraLight:
        return vb::ToV8(context, "extra-light");
      case nu::Font::Weight::Light:
        return vb::ToV8(context, "light");
      case nu::Font::Weight::Normal:
        return vb::ToV8(context, "normal");
      case nu::Font::Weight::Medium:
        return vb::ToV8(context, "medium");
      case nu::Font::Weight::SemiBold:
        return vb::ToV8(context, "semi-bold");
      case nu::Font::Weight::Bold:
        return vb::ToV8(context, "bold");
      case nu::Font::Weight::ExtraBold:
        return vb::ToV8(context, "extra-bold");
      case nu::Font::Weight::Black:
        return vb::ToV8(context, "black");
    }
    NOTREACHED();
    return v8::Undefined(context->GetIsolate());
  }
};

template<>
struct Type<nu::Font::Style> {
  static constexpr const char* name = "yue.Font.Style";
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::Font::Style* out) {
    std::string style;
    if (!vb::FromV8(context, value, &style))
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
  static v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                   nu::Font::Style style) {
    switch (style) {
      case nu::Font::Style::Normal:
        return vb::ToV8(context, "normal");
      case nu::Font::Style::Italic:
        return vb::ToV8(context, "italic");
    }
    NOTREACHED();
    return v8::Undefined(context->GetIsolate());
  }
};

template<>
struct Type<nu::Font> {
  static constexpr const char* name = "yue.Font";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor,
        "create", &CreateOnHeap<nu::Font, const std::string&, float,
                                nu::Font::Weight, nu::Font::Style>,
        "default", &GetDefault);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "getName", &nu::Font::GetName,
        "getSize", &nu::Font::GetSize,
        "getWeight", &nu::Font::GetWeight,
        "getStyle", &nu::Font::GetStyle);
  }
  static nu::Font* GetDefault() {
    return nu::App::GetCurrent()->GetDefaultFont();
  }
};

template<>
struct Type<nu::Canvas> {
  static constexpr const char* name = "yue.Canvas";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor,
        "create", &CreateOnHeap<nu::Canvas, const nu::SizeF&, float>,
        "createForMainScreen", &CreateOnHeap<nu::Canvas, const nu::SizeF&>);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "getScaleFactor", &nu::Canvas::GetScaleFactor,
        "getPainter", &nu::Canvas::GetPainter,
        "getSize", &nu::Canvas::GetSize);
  }
};

template<>
struct Type<nu::Color> {
  static constexpr const char* name = "yue.Color";
  static v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                   nu::Color color) {
    return vb::ToV8(context, color.value());
  }
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::Color* out) {
    // Direct value.
    if (value->IsUint32()) {
      *out = nu::Color(value->Uint32Value(context).ToChecked());
      return true;
    }
    // String representation.
    std::string hex;
    if (!vb::FromV8(context, value, &hex))
      return false;
    *out = nu::Color(hex);
    return true;
  }
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor,
        "rgb", &CreateOnStack<nu::Color, unsigned, unsigned, unsigned>,
        "argb", &CreateOnStack<nu::Color, unsigned, unsigned, unsigned,
                               unsigned>);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
  }
};

template<>
struct Type<nu::Image> {
  static constexpr const char* name = "yue.Image";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor,
        "createFromPath", &CreateOnHeap<nu::Image, const base::FilePath&>);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "getSize", &nu::Image::GetSize,
        "getScaleFactor", &nu::Image::GetScaleFactor);
  }
};

template<>
struct Type<nu::TextAlign> {
  static constexpr const char* name = "yue.TextAlign";
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::TextAlign* out) {
    std::string align;
    if (!vb::FromV8(context, value, &align))
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
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::TextAttributes* out) {
    if (!value->IsObject())
      return false;
    v8::Local<v8::Object> obj = value.As<v8::Object>();
    nu::Font* font;
    if (Get(context, obj, "font", &font))
      out->font = font;
    Get(context, obj, "color", &out->color);
    Get(context, obj, "align", &out->align);
    Get(context, obj, "valign", &out->valign);
    return true;
  }
};

template<>
struct Type<nu::TextMetrics> {
  static constexpr const char* name = "yue.TextMetrics";
  static v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                   const nu::TextMetrics& metrics) {
    v8::Local<v8::Object> obj = v8::Object::New(context->GetIsolate());
    Set(context, obj, "size", metrics.size);
    return obj;
  }
};

template<>
struct Type<nu::Painter> {
  static constexpr const char* name = "yue.Painter";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "save", &nu::Painter::Save,
        "restore", &nu::Painter::Restore,
        "beginPath", &nu::Painter::BeginPath,
        "closePath", &nu::Painter::ClosePath,
        "moveTo", &nu::Painter::MoveTo,
        "lineTo", &nu::Painter::LineTo,
        "bezierCurveTo", &nu::Painter::BezierCurveTo,
        "arcTo", &nu::Painter::ArcTo,
        "arc", &nu::Painter::Arc,
        "rect", &nu::Painter::Rect,
        "clip", &nu::Painter::Clip,
        "clipRect", &nu::Painter::ClipRect,
        "translate", &nu::Painter::Translate,
        "rotate", &nu::Painter::Rotate,
        "scale", &nu::Painter::Scale,
        "setColor", &nu::Painter::SetColor,
        "setStrokeColor", &nu::Painter::SetStrokeColor,
        "setFillColor", &nu::Painter::SetFillColor,
        "setLineWidth", &nu::Painter::SetLineWidth,
        "stroke", &nu::Painter::Stroke,
        "fill", &nu::Painter::Fill,
        "strokeRect", &nu::Painter::StrokeRect,
        "fillRect", &nu::Painter::FillRect,
        "measureText", &nu::Painter::MeasureText,
        "drawText", &nu::Painter::DrawText);
  }
};

template<>
struct Type<nu::EventType> {
  static constexpr const char* name = "yue.EventType";
  static v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                   nu::EventType type) {
    switch (type) {
      case nu::EventType::MouseDown:
        return vb::ToV8(context, "mouseDown");
      case nu::EventType::MouseUp:
        return vb::ToV8(context, "mouseUp");
      case nu::EventType::MouseMove:
        return vb::ToV8(context, "mouseMove");
      case nu::EventType::MouseEnter:
        return vb::ToV8(context, "mouseEnter");
      case nu::EventType::MouseLeave:
        return vb::ToV8(context, "mouseLeave");
      case nu::EventType::KeyDown:
        return vb::ToV8(context, "keyDown");
      case nu::EventType::KeyUp:
        return vb::ToV8(context, "keyUp");
      default:
        NOTREACHED();
        return vb::ToV8(context, "unknown");
    }
  }
};

template<>
struct Type<nu::Event> {
  static constexpr const char* name = "yue.Event";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor,
        "maskShift", static_cast<int>(nu::MASK_SHIFT),
        "maskControl", static_cast<int>(nu::MASK_CONTROL),
        "maskAlt", static_cast<int>(nu::MASK_ALT),
        "maskMeta", static_cast<int>(nu::MASK_META));
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
  }
  // Used by subclasses.
  static void SetEventProperties(v8::Local<v8::Context> context,
                                 v8::Local<v8::Object> obj,
                                 const nu::Event* event) {
    Set(context, obj,
        "type", event->type,
        "modifiers", event->modifiers,
        "timestamp", event->timestamp);
  }
};

template<>
struct Type<nu::MouseEvent> {
  using base = nu::Event;
  static constexpr const char* name = "yue.MouseEvent";
  static v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                   const nu::MouseEvent& event) {
    v8::Local<v8::Object> obj = v8::Object::New(context->GetIsolate());
    Type<nu::Event>::SetEventProperties(context, obj, &event);
    Set(context, obj,
        "button", event.button,
        "positionInView", event.position_in_view,
        "positionInWindow", event.position_in_window);
    return obj;
  }
};

template<>
struct Type<nu::KeyboardCode> {
  static constexpr const char* name = "yue.KeyboardCode";
  static v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                   nu::KeyboardCode code) {
    return vb::ToV8(context, nu::KeyboardCodeToStr(code));
  }
};

template<>
struct Type<nu::KeyEvent> {
  using base = nu::Event;
  static constexpr const char* name = "yue.KeyEvent";
  static v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                   const nu::KeyEvent& event) {
    v8::Local<v8::Object> obj = v8::Object::New(context->GetIsolate());
    Type<nu::Event>::SetEventProperties(context, obj, &event);
    Set(context, obj,
        "key", event.key);
    return obj;
  }
};

template<>
struct Type<nu::MenuBase> {
  static constexpr const char* name = "yue.MenuBase";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "append", &nu::MenuBase::Append,
        "insert", &nu::MenuBase::Insert,
        "itemCount", &nu::MenuBase::ItemCount,
        "itemAt", &nu::MenuBase::ItemAt);
  }
};

void ReadMenuItems(v8::Local<v8::Context> context,
                   v8::Local<v8::Array> options,
                   nu::MenuBase* menu);

template<>
struct Type<nu::MenuBar> {
  using base = nu::MenuBase;
  static constexpr const char* name = "yue.MenuBar";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor, "create", &Create);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
  }
  static nu::MenuBar* Create(v8::Local<v8::Context> context,
                             v8::Local<v8::Array> options) {
    nu::MenuBar* menu = new nu::MenuBar;
    ReadMenuItems(context, options, menu);
    return menu;
  }
};

template<>
struct Type<nu::Menu> {
  using base = nu::MenuBase;
  static constexpr const char* name = "yue.Menu";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor, "create", &Create);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "popup", &nu::Menu::Popup);
  }
  static nu::Menu* Create(v8::Local<v8::Context> context,
                          v8::Local<v8::Array> options) {
    nu::Menu* menu = new nu::Menu;
    ReadMenuItems(context, options, menu);
    return menu;
  }
};

template<>
struct Type<nu::MenuItem::Type> {
  static constexpr const char* name = "yue.MenuItem.Type";
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::MenuItem::Type* out) {
    std::string type;
    if (!vb::FromV8(context, value, &type))
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
struct Type<nu::MenuItem> {
  static constexpr const char* name = "yue.MenuItem";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor, "create", &Create);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "click", &nu::MenuItem::Click,
        "setLabel", &nu::MenuItem::SetLabel,
        "getLabel", &nu::MenuItem::GetLabel,
        "setChecked", &nu::MenuItem::SetChecked,
        "isChecked", &nu::MenuItem::IsChecked,
        "setEnabled", &nu::MenuItem::SetEnabled,
        "isEnabled", &nu::MenuItem::IsEnabled,
        "setVisible", &nu::MenuItem::SetVisible,
        "isVisible", &nu::MenuItem::IsVisible,
        "setSubmenu", &nu::MenuItem::SetSubmenu,
        "getSubmenu", &nu::MenuItem::GetSubmenu,
        "setAccelerator", &nu::MenuItem::SetAccelerator);
    SetProperty(context, templ,
                "onClick", &nu::MenuItem::on_click);
  }
  static nu::MenuItem* Create(v8::Local<v8::Context> context,
                              v8::Local<v8::Value> value) {
    nu::MenuItem::Type type = nu::MenuItem::Type::Label;
    if (FromV8(context, value, &type) || !value->IsObject())
      return new nu::MenuItem(type);
    v8::Local<v8::Object> obj = value.As<v8::Object>();
    // Use label unless "type" is specified.
    nu::MenuItem* item = nullptr;
    if (Get(context, obj, "type", &type))
      item = new nu::MenuItem(type);
    // Read table fields and set attributes.
    bool b = false;
    if (Get(context, obj, "checked", &b)) {
      if (!item) item = new nu::MenuItem(nu::MenuItem::Type::Checkbox);
      item->SetChecked(b);
    }
    nu::Menu* submenu = nullptr;
    v8::Local<v8::Array> options;
    if (Get(context, obj, "submenu", &submenu) ||
        (Get(context, obj, "submenu", &options) && options->IsArray())) {
      if (!submenu) submenu = Type<nu::Menu>::Create(context, options);
      if (!item) item = new nu::MenuItem(nu::MenuItem::Type::Submenu);
      item->SetSubmenu(submenu);
    }
    if (!item)  // can not deduce type from property, assuming Label item.
      item = new nu::MenuItem(nu::MenuItem::Type::Label);
    if (Get(context, obj, "visible", &b))
      item->SetVisible(b);
    if (Get(context, obj, "enabled", &b))
      item->SetEnabled(b);
    std::string label;
    if (Get(context, obj, "label", &label))
      item->SetLabel(label);
    nu::Accelerator accelerator;
    if (Get(context, obj, "accelerator", &accelerator))
      item->SetAccelerator(accelerator);
    Get(context, obj, "onClick", &item->on_click);
    return item;
  }
};

void ReadMenuItems(v8::Local<v8::Context> context,
                   v8::Local<v8::Array> arr,
                   nu::MenuBase* menu) {
  std::vector<v8::Local<v8::Object>> items;
  if (vb::FromV8(context, arr, &items)) {
    for (v8::Local<v8::Object> obj : items) {
      // Create the item if an object is passed.
      nu::MenuItem* item;
      if (!vb::FromV8(context, obj, &item))
        item = Type<nu::MenuItem>::Create(context, obj);
      menu->Append(item);
    }
  }
}

template<>
struct Type<nu::Window::Options> {
  static constexpr const char* name = "yue.Window.Options";
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::Window::Options* out) {
    auto obj = value.As<v8::Object>();
    if (obj.IsEmpty())
      return false;
    Get(context, obj, "frame", &out->frame);
    Get(context, obj, "transparent", &out->transparent);
#if defined(OS_MACOSX)
    Get(context, obj, "showTrafficLights", &out->show_traffic_lights);
#endif
    return true;
  }
};

template<>
struct Type<nu::Window> {
  static constexpr const char* name = "yue.Window";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor,
        "create", &CreateOnHeap<nu::Window, nu::Window::Options>);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "close", &nu::Window::Close,
        "setHasShadow", &nu::Window::SetHasShadow,
        "hasShadow", &nu::Window::HasShadow,
        "center", &nu::Window::Center,
        "setContentView", &nu::Window::SetContentView,
        "getContentView", &nu::Window::GetContentView,
        "setContentSize", &nu::Window::SetContentSize,
        "getContentSize", &nu::Window::GetContentSize,
        "setBounds", &nu::Window::SetBounds,
        "getBounds", &nu::Window::GetBounds,
        "setSizeConstraints", &nu::Window::SetSizeConstraints,
        "getSizeConstraints", &nu::Window::GetSizeConstraints,
        "setContentSizeConstraints", &nu::Window::SetContentSizeConstraints,
        "getContentSizeConstraints", &nu::Window::GetContentSizeConstraints,
        "activate", &nu::Window::Activate,
        "deactivate", &nu::Window::Deactivate,
        "isactive", &nu::Window::IsActive,
        "setVisible", &nu::Window::SetVisible,
        "isVisible", &nu::Window::IsVisible,
        "setAlwaysOnTop", &nu::Window::SetAlwaysOnTop,
        "isAlwaysOnTop", &nu::Window::IsAlwaysOnTop,
        "setFullscreen", &nu::Window::SetFullscreen,
        "isFullscreen", &nu::Window::IsFullscreen,
        "maximize", &nu::Window::Maximize,
        "unmaximize", &nu::Window::Unmaximize,
        "isMaximized", &nu::Window::IsMaximized,
        "minimize", &nu::Window::Minimize,
        "restore", &nu::Window::Restore,
        "isMinimized", &nu::Window::IsMinimized,
        "setResizable", &nu::Window::SetResizable,
        "isResizable", &nu::Window::IsResizable,
        "setMaximizable", &nu::Window::SetMaximizable,
        "isMaximizable", &nu::Window::IsMaximizable,
        "setMinimizable", &nu::Window::SetMaximizable,
        "isMinimizable", &nu::Window::IsMinimizable,
        "setMovable", &nu::Window::SetMovable,
        "isMovable", &nu::Window::IsMovable,
#if defined(OS_WIN) || defined(OS_LINUX)
        "setMenuBar", &nu::Window::SetMenuBar,
        "getMenuBar", &nu::Window::GetMenuBar,
#endif
        "setBackgroundColor", &nu::Window::SetBackgroundColor);
    SetProperty(context, templ,
                "onClose", &nu::Window::on_close,
                "shouldClose", &nu::Window::should_close);
  }
};

template<>
struct Type<nu::View> {
  static constexpr const char* name = "yue.View";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "offsetFromView", &nu::View::OffsetFromView,
        "offsetFromWindow", &nu::View::OffsetFromWindow,
        "setBounds", &nu::View::SetBounds,
        "getBounds", &nu::View::GetBounds,
        "layout", &nu::View::Layout,
        "schedulePaint", &nu::View::SchedulePaint,
        "setVisible", &nu::View::SetVisible,
        "isVisible", &nu::View::IsVisible,
        "focus", &nu::View::Focus,
        "hasFocus", &nu::View::HasFocus,
        "setFocusable", &nu::View::SetFocusable,
        "isFocusable", &nu::View::IsFocusable,
        "setCapture", &nu::View::SetCapture,
        "releaseCapture", &nu::View::ReleaseCapture,
        "hasCapture", &nu::View::HasCapture,
        "setMouseDownCanMoveWindow", &nu::View::SetMouseDownCanMoveWindow,
        "isMouseDownCanMoveWindow", &nu::View::IsMouseDownCanMoveWindow,
        "setFont", &nu::View::SetFont,
        "setColor", &nu::View::SetColor,
        "setBackgroundColor", &nu::View::SetBackgroundColor,
        "setStyle", &SetStyle,
        "printStyle", &nu::View::PrintStyle,
        "getParent", &nu::View::GetParent,
        "getWindow", &nu::View::GetWindow);
    SetProperty(context, templ,
                "onMouseDown", &nu::View::on_mouse_down,
                "onMouseUp", &nu::View::on_mouse_up,
                "onMouseMove", &nu::View::on_mouse_move,
                "onMouseEnter", &nu::View::on_mouse_enter,
                "onMouseLeave", &nu::View::on_mouse_leave,
                "onKeyDown", &nu::View::on_key_down,
                "onKeyUp", &nu::View::on_key_up,
                "onSizeChanged", &nu::View::on_size_changed,
                "onCaptureLost", &nu::View::on_capture_lost);
  }
  static void SetStyle(
      Arguments* args,
      const std::map<std::string, v8::Local<v8::Value>>& styles) {
    nu::View* view;
    if (!args->GetHolder(&view))
      return;
    for (const auto& it : styles) {
      if (it.second->IsNumber())
        view->SetStyleProperty(it.first, it.second->NumberValue());
      else
        view->SetStyleProperty(it.first, *v8::String::Utf8Value(it.second));
    }
    view->Layout();
  }
};

template<>
struct Type<nu::Container> {
  using base = nu::View;
  static constexpr const char* name = "yue.Container";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor,
        "create", &CreateOnHeap<nu::Container>);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "getPreferredSize", &nu::Container::GetPreferredSize,
        "getPreferredWidthForHeight",
        &nu::Container::GetPreferredWidthForHeight,
        "getPreferredHeightForWidth",
        &nu::Container::GetPreferredHeightForWidth,
        "addChildView", &nu::Container::AddChildView,
        "addChildViewAt", &nu::Container::AddChildViewAt,
        "removeChildView", &nu::Container::RemoveChildView,
        "childCount", &nu::Container::ChildCount,
        "childAt", &nu::Container::ChildAt);
    SetProperty(context, templ,
                "onDraw", &nu::Container::on_draw);
  }
};

template<>
struct Type<nu::Button::Type> {
  static constexpr const char* name = "yue.Button.Type";
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::Button::Type* out) {
    std::string type;
    if (!vb::FromV8(context, value, &type))
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

template<>
struct Type<nu::Button> {
  using base = nu::View;
  static constexpr const char* name = "yue.Button";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor, "create", &Create);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "setTitle", &nu::Button::SetTitle,
        "getTitle", &nu::Button::GetTitle,
        "setChecked", &nu::Button::SetChecked,
        "isChecked", &nu::Button::IsChecked);
    SetProperty(context, templ,
                "onClick", &nu::Button::on_click);
  }
  static nu::Button* Create(Arguments* args, v8::Local<v8::Context> context,
                            v8::Local<v8::Value> value) {
    std::string title;
    if (FromV8(context, value, &title)) {
      return new nu::Button(title);
    } else if (value->IsObject()) {
      v8::Local<v8::Object> obj = value.As<v8::Object>();
      Get(context, obj, "title", &title);
      nu::Button::Type type = nu::Button::Type::Normal;
      Get(context, obj, "type", &type);
      return new nu::Button(title, type);
    } else {
      args->ThrowError("String or Object");
      return nullptr;
    }
  }
};

template<>
struct Type<nu::Browser> {
  using base = nu::View;
  static constexpr const char* name = "yue.Browser";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor, "create", &CreateOnHeap<nu::Browser>);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "loadURL", &nu::Browser::LoadURL);
    SetProperty(context, templ,
                "onClose", &nu::Browser::on_close);
  }
};

template<>
struct Type<nu::Entry> {
  using base = nu::View;
  static constexpr const char* name = "yue.Entry";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor, "create", &CreateOnHeap<nu::Entry>);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "setText", &nu::Entry::SetText,
        "getText", &nu::Entry::GetText);
    SetProperty(context, templ,
                "onActivate", &nu::Entry::on_activate,
                "onTextChange", &nu::Entry::on_text_change);
  }
};

template<>
struct Type<nu::Label> {
  using base = nu::View;
  static constexpr const char* name = "yue.Label";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor,
        "create", &CreateOnHeap<nu::Label, const std::string&>);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "setText", &nu::Label::SetText,
        "getText", &nu::Label::GetText);
  }
};

template<>
struct Type<nu::ProgressBar> {
  using base = nu::View;
  static constexpr const char* name = "yue.ProgressBar";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor, "create", &CreateOnHeap<nu::ProgressBar>);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "setValue", &nu::ProgressBar::SetValue,
        "getValue", &nu::ProgressBar::GetValue,
        "setIndeterminate", &nu::ProgressBar::SetIndeterminate,
        "isIndeterminate", &nu::ProgressBar::IsIndeterminate);
  }
};

template<>
struct Type<nu::Group> {
  using base = nu::View;
  static constexpr const char* name = "yue.Group";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor,
        "create", &CreateOnHeap<nu::Group, const std::string&>);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "setContentView", &nu::Group::SetContentView,
        "getContentView", &nu::Group::GetContentView,
        "setTitle", &nu::Group::SetTitle,
        "getTitle", &nu::Group::GetTitle);
  }
};

template<>
struct Type<nu::Scroll::Policy> {
  static constexpr const char* name = "yue.Scroll.Policy";
  static v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                   nu::Scroll::Policy policy) {
    if (policy == nu::Scroll::Policy::Always)
      return vb::ToV8(context, "always");
    else if (policy == nu::Scroll::Policy::Never)
      return vb::ToV8(context, "never");
    else
      return vb::ToV8(context, "automatic");
  }
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::Scroll::Policy* out) {
    std::string policy;
    if (!vb::FromV8(context, value, &policy))
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
};

template<>
struct Type<nu::Scroll> {
  using base = nu::View;
  static constexpr const char* name = "yue.Scroll";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor, "create", &CreateOnHeap<nu::Scroll>);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "setScrollbarPolicy", &nu::Scroll::SetScrollbarPolicy,
        "getScrollbarPolicy", &nu::Scroll::GetScrollbarPolicy,
        "setContentSize", &nu::Scroll::SetContentSize,
        "getContentSize", &nu::Scroll::GetContentSize,
        "setContentView", &nu::Scroll::SetContentView,
        "getContentView", &nu::Scroll::GetContentView);
  }
};

template<>
struct Type<nu::TextEdit> {
  using base = nu::View;
  static constexpr const char* name = "yue.TextEdit";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor, "create", &CreateOnHeap<nu::TextEdit>);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "setText", &nu::TextEdit::SetText,
        "getText", &nu::TextEdit::GetText,
        "cut", &nu::TextEdit::Cut,
        "copy", &nu::TextEdit::Copy,
        "paste", &nu::TextEdit::Paste,
        "clear", &nu::TextEdit::Clear,
        "getSelectionRange", &nu::TextEdit::GetSelectionRange,
        "selectRange", &nu::TextEdit::SelectRange,
        "getTextInRange", &nu::TextEdit::GetTextInRange,
        "insertText", &nu::TextEdit::InsertText,
        "insertTextAt", &nu::TextEdit::InsertTextAt,
        "delete", &nu::TextEdit::Delete,
        "deleteRange", &nu::TextEdit::DeleteRange);
    SetProperty(context, templ,
                "onTextChange", &nu::TextEdit::on_text_change);
  }
};

#if defined(OS_MACOSX)
template<>
struct Type<nu::Vibrant::Material> {
  static constexpr const char* name = "yue.Vibrant.Material";
  static v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                   nu::Vibrant::Material material) {
    if (material == nu::Vibrant::Material::Light)
      return vb::ToV8(context, "light");
    else if (material == nu::Vibrant::Material::Dark)
      return vb::ToV8(context, "dark");
    else if (material == nu::Vibrant::Material::Titlebar)
      return vb::ToV8(context, "titlebar");
    else
      return vb::ToV8(context, "appearance-based");
  }
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::Vibrant::Material* out) {
    std::string material;
    if (!vb::FromV8(context, value, &material))
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
};

template<>
struct Type<nu::Vibrant::BlendingMode> {
  static constexpr const char* name = "yue.Vibrant.BlendingMode";
  static v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                   nu::Vibrant::BlendingMode mode) {
    if (mode == nu::Vibrant::BlendingMode::WithinWindow)
      return vb::ToV8(context, "within-window");
    else
      return vb::ToV8(context, "behind-window");
  }
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::Vibrant::BlendingMode* out) {
    std::string mode;
    if (!vb::FromV8(context, value, &mode))
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
};

template<>
struct Type<nu::Vibrant> {
  using base = nu::Container;
  static constexpr const char* name = "yue.Vibrant";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor, "create", &CreateOnHeap<nu::Vibrant>);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "setMaterial", &nu::Vibrant::SetMaterial,
        "getMaterial", &nu::Vibrant::GetMaterial,
        "setBlendingMode", &nu::Vibrant::SetBlendingMode,
        "getBlendingMode", &nu::Vibrant::GetBlendingMode);
  }
};
#endif

}  // namespace vb

namespace node_yue {

void Initialize(v8::Local<v8::Object> exports) {
#if defined(OS_WIN) && !defined(ELECTRON_BUILD)
  // Show system dialog on crash.
  SetErrorMode(GetErrorMode() & ~SEM_NOGPFAULTERRORBOX);
#endif
  // Initialize the nativeui and leak it.
  new nu::State;
#if !defined(ELECTRON_BUILD)
  new nu::Lifetime;
  // Initialize node integration and leak it.
  NodeIntegration* node_integration = NodeIntegration::Create();
  node_integration->PrepareMessageLoop();
  node_integration->RunMessageLoop();
#endif

  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  vb::Set(context, exports,
          // Classes.
#if !defined(ELECTRON_BUILD)
          "Lifetime",    vb::Constructor<nu::Lifetime>(),
#endif
          "App",         vb::Constructor<nu::App>(),
          "Font",        vb::Constructor<nu::Font>(),
          "Color",       vb::Constructor<nu::Color>(),
          "Image",       vb::Constructor<nu::Image>(),
          "Painter",     vb::Constructor<nu::Painter>(),
          "Event",       vb::Constructor<nu::Event>(),
          "MenuBar",     vb::Constructor<nu::MenuBar>(),
          "Menu",        vb::Constructor<nu::Menu>(),
          "MenuItem",    vb::Constructor<nu::MenuItem>(),
          "Window",      vb::Constructor<nu::Window>(),
          "View",        vb::Constructor<nu::View>(),
          "Container",   vb::Constructor<nu::Container>(),
          "Button",      vb::Constructor<nu::Button>(),
          "Browser",     vb::Constructor<nu::Browser>(),
          "Entry",       vb::Constructor<nu::Entry>(),
          "Label",       vb::Constructor<nu::Label>(),
          "ProgressBar", vb::Constructor<nu::ProgressBar>(),
          "Group",       vb::Constructor<nu::Group>(),
          "Scroll",      vb::Constructor<nu::Scroll>(),
          "TextEdit",    vb::Constructor<nu::TextEdit>(),
#if defined(OS_MACOSX)
          "Vibrant",     vb::Constructor<nu::Vibrant>(),
#endif
          // Properties.
#if !defined(ELECTRON_BUILD)
          "lifetime", nu::Lifetime::GetCurrent(),
#endif
          "app",      nu::State::GetCurrent()->GetApp());
}

}  // namespace node_yue

NODE_MODULE(gui, node_yue::Initialize)
