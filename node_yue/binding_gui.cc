// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include <node.h>

#include "nativeui/nativeui.h"
#include "node_yue/binding_signal.h"
#include "node_yue/binding_values.h"
#include "node_yue/node_integration.h"

#if defined(OS_MACOSX)
#include "node_yue/chrome_view_mac.h"
#endif

#include "node_buffer.h"  // NOLINT(build/include)

namespace {

bool is_electron = false;
bool is_yode = false;

}  // namespace

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
struct Type<nu::Buffer> {
  static constexpr const char* name = "yue.Buffer";
  static v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                   const nu::Buffer& value) {
    return node::Buffer::Copy(context->GetIsolate(),
                              static_cast<char*>(value.content()),
                              value.size()).ToLocalChecked();
  }
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::Buffer* out) {
    if (!node::Buffer::HasInstance(value))
      return false;
    // We are assuming the Buffer is consumed immediately.
    *out = nu::Buffer::Wrap(node::Buffer::Data(value),
                            node::Buffer::Length(value));
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

template<>
struct Type<nu::Lifetime> {
  static constexpr const char* name = "yue.Lifetime";
  static void BuildConstructor(v8::Local<v8::Context>, v8::Local<v8::Object>) {
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
#if defined(OS_MACOSX)
    SetProperty(context, templ,
                "onReady", &nu::Lifetime::on_ready,
                "onOpen", &nu::Lifetime::on_open,
                "onActivate", &nu::Lifetime::on_activate);
#endif
  }
};

template<>
struct Type<nu::MessageLoop> {
  static constexpr const char* name = "yue.MessageLoop";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor,
        "quit", &nu::MessageLoop::Quit,
        "postTask", &nu::MessageLoop::PostTask,
        "postDelayedTask", &nu::MessageLoop::PostDelayedTask);
    // The "run" method should never be used in yode runtime.
    if (!is_yode) {
      Set(context, constructor, "run", &nu::MessageLoop::Run);
    }
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
  }
};

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
  static void BuildConstructor(v8::Local<v8::Context>, v8::Local<v8::Object>) {
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
#if defined(OS_MACOSX)
        "setApplicationMenu",
        RefMethod(&nu::App::SetApplicationMenu, RefType::Reset, "appMenu"),
        "setDockBadgeLabel", &nu::App::SetDockBadgeLabel,
        "getDockBadgeLabel", &nu::App::GetDockBadgeLabel,
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
        "derive", &nu::Font::Derive,
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
        "createFromPath", &CreateOnHeap<nu::Image, const base::FilePath&>,
        "createFromBuffer", &CreateOnHeap<nu::Image, const nu::Buffer&, float>);
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
        "drawText", &nu::Painter::DrawText,
        "drawCanvas", &nu::Painter::DrawCanvas,
        "drawCanvasFromRect", &nu::Painter::DrawCanvasFromRect,
        "drawImage", &nu::Painter::DrawImage);
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
        "maskMeta", static_cast<int>(nu::MASK_META),
        "isShiftPressed", &nu::Event::IsShiftPressed,
        "isControlPressed", &nu::Event::IsControlPressed,
        "isAltPressed", &nu::Event::IsAltPressed,
        "isMetaPressed", &nu::Event::IsMetaPressed);
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
struct Type<nu::FileDialog::Filter> {
  static constexpr const char* name = "yue.FileDialog.Filter";
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::FileDialog::Filter* out) {
    if (!value->IsObject())
      return false;
    return Get(context, value.As<v8::Object>(),
               "description", &std::get<0>(*out),
               "extensions", &std::get<1>(*out));
  }
  static v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                   const nu::FileDialog::Filter& filter) {
    auto obj = v8::Object::New(context->GetIsolate());
    Set(context, obj,
        "description", std::get<0>(filter),
        "extensions", std::get<1>(filter));
    return obj;
  }
};

template<>
struct Type<nu::FileDialog> {
  static constexpr const char* name = "yue.FileDialog";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor,
        "optionPickFolders",
        static_cast<int>(nu::FileDialog::OPTION_PICK_FOLDERS),
        "optionMultiSelect",
        static_cast<int>(nu::FileDialog::OPTION_MULTI_SELECT),
        "optionShowHidden",
        static_cast<int>(nu::FileDialog::OPTION_SHOW_HIDDEN));
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "getResult", &nu::FileDialog::GetResult,
        "run", &nu::FileDialog::Run,
        "runForWindow", &nu::FileDialog::RunForWindow,
        "setTitle", &nu::FileDialog::SetTitle,
        "setButtonLabel", &nu::FileDialog::SetButtonLabel,
        "setFilename", &nu::FileDialog::SetFilename,
        "setFolder", &nu::FileDialog::SetFolder,
        "setOptions", &nu::FileDialog::SetOptions,
        "setFilters", &nu::FileDialog::SetFilters);
  }
};

template<>
struct Type<nu::FileOpenDialog> {
  using base = nu::FileDialog;
  static constexpr const char* name = "yue.FileOpenDialog";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor, "create", &CreateOnHeap<nu::FileOpenDialog>);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "getResults", &nu::FileOpenDialog::GetResults);
  }
};

template<>
struct Type<nu::FileSaveDialog> {
  using base = nu::FileDialog;
  static constexpr const char* name = "yue.FileSaveDialog";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor, "create", &CreateOnHeap<nu::FileSaveDialog>);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
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
        "append", RefMethod(&nu::MenuBase::Append, RefType::Ref),
        "insert", RefMethod(&nu::MenuBase::Insert, RefType::Ref),
        "remove", RefMethod(&nu::MenuBase::Remove, RefType::Deref),
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
  static nu::MenuBar* CreateRaw(v8::Local<v8::Context> context,
                                v8::Local<v8::Array> options) {
    nu::MenuBar* menu = new nu::MenuBar;
    ReadMenuItems(context, options, menu);
    return menu;
  }
  static v8::Local<v8::Value> Create(v8::Local<v8::Context> context,
                                     v8::Local<v8::Array> options) {
    nu::MenuBar* menu = CreateRaw(context, options);
    // Remember the options.
    v8::Local<v8::Value> ret = vb::ToV8(context, menu);
    auto key = v8::Private::ForApi(context->GetIsolate(),
                                   ToV8(context, "options").As<v8::String>());
    ret.As<v8::Object>()->SetPrivate(context, key, options);
    return ret;
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
  static nu::Menu* CreateRaw(v8::Local<v8::Context> context,
                             v8::Local<v8::Array> options) {
    nu::Menu* menu = new nu::Menu;
    ReadMenuItems(context, options, menu);
    return menu;
  }
  static v8::Local<v8::Value> Create(v8::Local<v8::Context> context,
                                     v8::Local<v8::Array> options) {
    nu::Menu* menu = CreateRaw(context, options);
    // Remember the options.
    v8::Local<v8::Value> ret = vb::ToV8(context, menu);
    auto key = v8::Private::ForApi(context->GetIsolate(),
                                   ToV8(context, "options").As<v8::String>());
    ret.As<v8::Object>()->SetPrivate(context, key, options);
    return ret;
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
struct Type<nu::MenuItem::Role> {
  static constexpr const char* name = "yue.MenuItem.Role";
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::MenuItem::Role* out) {
    std::string role;
    if (!vb::FromV8(context, value, &role))
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
  static nu::MenuItem* CreateRaw(v8::Local<v8::Context> context,
                                 v8::Local<v8::Value> value) {
    nu::MenuItem::Type type = nu::MenuItem::Type::Label;
    if (FromV8(context, value, &type) || !value->IsObject())
      return new nu::MenuItem(type);
    v8::Local<v8::Object> obj = value.As<v8::Object>();
    nu::MenuItem* item = nullptr;
    // First read role.
    nu::MenuItem::Role role;
    if (Get(context, obj, "role", &role))
      item = new nu::MenuItem(role);
    // Use label if "type" is not specified.
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
      if (!submenu) submenu = Type<nu::Menu>::CreateRaw(context, options);
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
    // The signal handler must not be referenced by C++.
    v8::Local<v8::Value> on_click_val;
    std::function<void(nu::MenuItem*)> on_click;
    if (Get(context, obj, "onClick", &on_click_val) &&
        WeakFunctionFromV8(context, on_click_val, &on_click))
      item->on_click.Connect(on_click);
    return item;
  }
  static v8::Local<v8::Value> Create(v8::Local<v8::Context> context,
                                     v8::Local<v8::Value> value) {
    nu::MenuItem* item = CreateRaw(context, value);
    // Remember the options.
    v8::Local<v8::Value> ret = vb::ToV8(context, item);
    auto key = v8::Private::ForApi(context->GetIsolate(),
                                   ToV8(context, "options").As<v8::String>());
    ret.As<v8::Object>()->SetPrivate(context, key, value);
    return ret;
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
        item = Type<nu::MenuItem>::CreateRaw(context, obj);
      menu->Append(item);
    }
  }
}

template<>
struct Type<nu::Tray> {
  static constexpr const char* name = "yue.Tray";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor,
#if defined(OS_MACOSX)
        "createWithTitle", &CreateOnHeap<nu::Tray, const std::string&>,
#endif
        "createWithImage", &CreateOnHeap<nu::Tray, nu::Image*>);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "setTitle", &nu::Tray::SetTitle,
        "setImage", &nu::Tray::SetImage,
        "setMenu", RefMethod(&nu::Tray::SetMenu, RefType::Reset, "menu"));
    SetProperty(context, templ, "onClick", &nu::Tray::on_click);
  }
};

#if defined(OS_MACOSX)
template<>
struct Type<nu::Toolbar::Item> {
  static constexpr const char* name = "yue.Toolbar.Item";
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::Toolbar::Item* out) {
    if (!value->IsObject())
      return false;
    v8::Local<v8::Object> obj = value.As<v8::Object>();
    nu::Image* image;
    if (Get(context, obj, "image", &image))
      out->image = image;
    nu::View* view;
    if (Get(context, obj, "view", &view))
      out->view = view;
    Get(context, obj, "label", &out->label);
    Get(context, obj, "minSize", &out->min_size);
    Get(context, obj, "maxSize", &out->max_size);
    Get(context, obj, "subitems", &out->subitems);
    Get(context, obj, "onClick", &out->on_click);
    return true;
  }
};

template<>
struct Type<nu::Toolbar::DisplayMode> {
  static constexpr const char* name = "yue.Toolbar.DisplayMode";
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::Toolbar::DisplayMode* out) {
    std::string mode;
    if (!vb::FromV8(context, value, &mode))
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
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor,
        "create", &CreateOnHeap<nu::Toolbar, const std::string&>);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "setDefaultItemIdentifiers", &nu::Toolbar::SetDefaultItemIdentifiers,
        "setAllowedItemIdentifiers", &nu::Toolbar::SetAllowedItemIdentifiers,
        "setAllowCustomization", &nu::Toolbar::SetAllowCustomization,
        "setDisplayMode", &nu::Toolbar::SetDisplayMode,
        "setVisible", &nu::Toolbar::SetVisible,
        "isVisible", &nu::Toolbar::IsVisible,
        "getIdentifier", &nu::Toolbar::GetIdentifier);
    SetProperty(context, templ,
                "getItem", &nu::Toolbar::get_item);
  }
};
#endif

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
        "setContentView",
        RefMethod(&nu::Window::SetContentView, RefType::Reset, "contentView"),
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
        "isActive", &nu::Window::IsActive,
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
        "setMinimizable", &nu::Window::SetMinimizable,
        "isMinimizable", &nu::Window::IsMinimizable,
        "setMovable", &nu::Window::SetMovable,
        "isMovable", &nu::Window::IsMovable,
        "setTitle", &nu::Window::SetTitle,
        "getTitle", &nu::Window::GetTitle,
        "setBackgroundColor", &nu::Window::SetBackgroundColor,
#if defined(OS_MACOSX)
        "setToolbar", &nu::Window::SetToolbar,
        "getToolbar", &nu::Window::GetToolbar,
        "setTitleVisible", &nu::Window::SetTitleVisible,
        "isTitleVisible", &nu::Window::IsTitleVisible,
        "setFullSizeContentView", &nu::Window::SetFullSizeContentView,
        "isFullSizeContentView", &nu::Window::IsFullSizeContentView,
#endif
#if defined(OS_WIN) || defined(OS_LINUX)
        "setMenuBar",
        RefMethod(&nu::Window::SetMenuBar, RefType::Reset, "menuBar"),
        "getMenuBar", &nu::Window::GetMenuBar,
#endif
        "addChildWindow",
        RefMethod(&nu::Window::AddChildWindow, RefType::Ref),
        "removeChildView",
        RefMethod(&nu::Window::RemoveChildWindow, RefType::Deref),
        "getChildWindows", &nu::Window::GetChildWindows);
    SetProperty(context, templ,
                "onClose", &nu::Window::on_close,
                "onFocus", &nu::Window::on_focus,
                "onBlur", &nu::Window::on_blur,
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
        "schedulePaintRect", &nu::View::SchedulePaintRect,
        "setVisible", &nu::View::SetVisible,
        "isVisible", &nu::View::IsVisible,
        "setEnabled", &nu::View::SetEnabled,
        "isEnabled", &nu::View::IsEnabled,
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
        "getMinimumSize", &nu::View::GetMinimumSize,
#if defined(OS_MACOSX)
        "setWantsLayer", &nu::View::SetWantsLayer,
        "wantsLayer", &nu::View::WantsLayer,
#endif
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
        "addChildView",
        RefMethod(&nu::Container::AddChildView, RefType::Ref),
        "addChildViewAt",
        RefMethod(&nu::Container::AddChildViewAt, RefType::Ref),
        "removeChildView",
        RefMethod(&nu::Container::RemoveChildView, RefType::Deref),
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

#if defined(OS_MACOSX)
template<>
struct Type<nu::Button::Style> {
  static constexpr const char* name = "yue.Button.Style";
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::Button::Style* out) {
    std::string style;
    if (!vb::FromV8(context, value, &style))
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
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor, "create", &Create);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "setTitle", &nu::Button::SetTitle,
        "getTitle", &nu::Button::GetTitle,
#if defined(OS_MACOSX)
        "setButtonStyle", &nu::Button::SetButtonStyle,
        "setHasBorder", &nu::Button::SetHasBorder,
        "hasBorder", &nu::Button::HasBorder,
#endif
        "setChecked", &nu::Button::SetChecked,
        "isChecked", &nu::Button::IsChecked,
        "setImage", &nu::Button::SetImage,
        "getImage", &nu::Button::GetImage);
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
struct Type<nu::ProtocolJob> {
  static constexpr const char* name = "yue.ProtocolJob";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
  }
};

template<>
struct Type<nu::ProtocolStringJob> {
  using base = nu::ProtocolJob;
  static constexpr const char* name = "yue.ProtocolStringJob";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor,
        "create", &CreateOnHeap<nu::ProtocolStringJob,
                                const std::string&,
                                const std::string&>);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
  }
};

template<>
struct Type<nu::ProtocolFileJob> {
  using base = nu::ProtocolJob;
  static constexpr const char* name = "yue.ProtocolFileJob";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor,
        "create", &CreateOnHeap<nu::ProtocolFileJob,
                                const ::base::FilePath&>);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
  }
};

template<>
struct Type<nu::ProtocolAsarJob> {
  using base = nu::ProtocolFileJob;
  static constexpr const char* name = "yue.ProtocolAsarJob";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor,
        "create", &CreateOnHeap<nu::ProtocolAsarJob,
                                const ::base::FilePath&,
                                const std::string&>);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "setDecipher", &nu::ProtocolAsarJob::SetDecipher);
  }
};

template<>
struct Type<nu::Browser::Options> {
  static constexpr const char* name = "yue.Browser.Options";
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::Browser::Options* out) {
    auto obj = value.As<v8::Object>();
    if (obj.IsEmpty())
      return false;
    Get(context, obj, "devtools", &out->devtools);
    Get(context, obj, "contextMenu", &out->context_menu);
    return true;
  }
};

template<>
struct Type<nu::Browser> {
  using base = nu::View;
  static constexpr const char* name = "yue.Browser";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor,
        "create", &CreateOnHeap<nu::Browser, const nu::Browser::Options&>,
        "registerProtocol", &nu::Browser::RegisterProtocol,
        "unregisterProtocol", &nu::Browser::UnregisterProtocol);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "loadURL", &nu::Browser::LoadURL,
        "loadHTML", &nu::Browser::LoadHTML,
        "getURL", &nu::Browser::GetURL,
        "getTitle", &nu::Browser::GetTitle,
        "setUserAgent", &nu::Browser::SetUserAgent,
        "executeJavaScript", &nu::Browser::ExecuteJavaScript,
        "goBack", &nu::Browser::GoBack,
        "canGoBack", &nu::Browser::CanGoBack,
        "goForward", &nu::Browser::GoForward,
        "canGoForward", &nu::Browser::CanGoForward,
        "reload", &nu::Browser::Reload,
        "stop", &nu::Browser::Stop,
        "isLoading", &nu::Browser::IsLoading,
        "setBindingName", &nu::Browser::SetBindingName,
        "addBinding", &AddBinding,
        "addRawBinding", &AddRawBinding,
        "removeBinding", &RemoveBinding);
    SetProperty(context, templ,
                "onClose", &nu::Browser::on_close,
                "onUpdateCommand", &nu::Browser::on_update_command,
                "onChangeLoading", &nu::Browser::on_change_loading,
                "onUpdateTitle", &nu::Browser::on_update_title,
                "onStartNavigation", &nu::Browser::on_start_navigation,
                "onCommitNavigation", &nu::Browser::on_commit_navigation,
                "onFinishNavigation", &nu::Browser::on_finish_navigation,
                "onFailNavigation", &nu::Browser::on_fail_navigation);
  }
  static void AddBinding(Arguments* args,
                         const std::string& name,
                         v8::Local<v8::Function> func) {
    nu::Browser* browser;
    if (!args->GetHolder(&browser))
      return;
    // this[bindings][name] = func.
    v8::Local<v8::Context> context = args->GetContext();
    v8::Local<v8::Map> refs = vb::GetAttachedTable(
        context, args->This(), "bindings");
    ignore_result(refs->Set(context, ToV8(context, name), func));
    // The func must be stored as weak reference.
    v8::Isolate* isolate = args->isolate();
    std::shared_ptr<internal::V8FunctionWrapper> func_ref(
        new internal::V8FunctionWrapper(isolate, func));
    func_ref->SetWeak();
    // Parse base::Value and call func with v8 args.
    browser->AddRawBinding(name, [isolate, func_ref](nu::Browser* browser,
                                                     ::base::Value value) {
      Locker locker(isolate);
      v8::HandleScope handle_scope(isolate);
      v8::MicrotasksScope script_scope(isolate,
                                       v8::MicrotasksScope::kRunMicrotasks);
      auto func = func_ref->Get(isolate);
      DCHECK(!func.IsEmpty());
      auto context = func->CreationContext();
      std::vector<v8::Local<v8::Value>> args;
      args.reserve(value.GetList().size());
      for (const auto& it : value.GetList())
        args.push_back(ToV8(context, it));
      node::MakeCallback(isolate, func, func, static_cast<int>(args.size()),
                         &args.front());
    });
  }
  static void AddRawBinding(Arguments* args,
                            const std::string& name,
                            v8::Local<v8::Function> func) {
    nu::Browser* browser;
    if (!args->GetHolder(&browser))
      return;
    // this[bindings][name] = func
    v8::Local<v8::Context> context = args->GetContext();
    v8::Local<v8::Map> refs = vb::GetAttachedTable(
        context, args->This(), "bindings");
    ignore_result(refs->Set(context, ToV8(context, name), func));
    // The func must be stored as weak reference.
    nu::Browser::BindingFunc callback;
    WeakFunctionFromV8(context, func, &callback);
    browser->AddRawBinding(name, callback);
  }
  static void RemoveBinding(Arguments* args, const std::string& name) {
    nu::Browser* browser;
    if (!args->GetHolder(&browser))
      return;
    // delete this[bindings][name]
    v8::Local<v8::Context> context = args->GetContext();
    v8::Local<v8::Map> refs = vb::GetAttachedTable(
        context, args->This(), "bindings");
    ignore_result(refs->Delete(context, ToV8(context, name)));
    // Pass down.
    browser->RemoveBinding(name);
  }
};

template<>
struct Type<nu::Entry::Type> {
  static constexpr const char* name = "yue.Entry.Type";
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::Entry::Type* out) {
    std::string type;
    if (!vb::FromV8(context, value, &type))
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
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor,
        "create", &CreateOnHeap<nu::Entry>,
        "createType", &CreateOnHeap<nu::Entry, nu::Entry::Type>);
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
        "getText", &nu::Label::GetText,
        "setAlign", &nu::Label::SetAlign,
        "setVAlign", &nu::Label::SetVAlign);
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
struct Type<nu::GifPlayer> {
  using base = nu::View;
  static constexpr const char* name = "yue.GifPlayer";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor,
        "create", &CreateOnHeap<nu::GifPlayer>);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "setImage", &nu::GifPlayer::SetImage,
        "getImage", &nu::GifPlayer::GetImage,
        "setAnimating", &nu::GifPlayer::SetAnimating,
        "isAnimating", &nu::GifPlayer::IsAnimating);
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
        "setContentView",
        RefMethod(&nu::Group::SetContentView, RefType::Reset, "contentView"),
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
        "setContentSize", &nu::Scroll::SetContentSize,
        "getContentSize", &nu::Scroll::GetContentSize,
        "setContentView",
        RefMethod(&nu::Scroll::SetContentView, RefType::Reset, "contentView"),
        "getContentView", &nu::Scroll::GetContentView,
#if !defined(OS_WIN)
        "setOverlayScrollbar", &nu::Scroll::SetOverlayScrollbar,
        "isOverlayScrollbar", &nu::Scroll::IsOverlayScrollbar,
#endif
        "setScrollbarPolicy", &nu::Scroll::SetScrollbarPolicy,
        "getScrollbarPolicy", &nu::Scroll::GetScrollbarPolicy);
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
        "redo", &nu::TextEdit::Redo,
        "canRedo", &nu::TextEdit::CanRedo,
        "undo", &nu::TextEdit::Undo,
        "canUndo", &nu::TextEdit::CanUndo,
        "cut", &nu::TextEdit::Cut,
        "copy", &nu::TextEdit::Copy,
        "paste", &nu::TextEdit::Paste,
        "selectAll", &nu::TextEdit::SelectAll,
        "getSelectionRange", &nu::TextEdit::GetSelectionRange,
        "selectRange", &nu::TextEdit::SelectRange,
        "getTextInRange", &nu::TextEdit::GetTextInRange,
        "insertText", &nu::TextEdit::InsertText,
        "insertTextAt", &nu::TextEdit::InsertTextAt,
        "delete", &nu::TextEdit::Delete,
        "deleteRange", &nu::TextEdit::DeleteRange,
#if !defined(OS_WIN)
        "setOverlayScrollbar", &nu::TextEdit::SetOverlayScrollbar,
#endif
        "setScrollbarPolicy", &nu::TextEdit::SetScrollbarPolicy,
        "getTextBounds", &nu::TextEdit::GetTextBounds);
    SetProperty(context, templ,
                "onTextChange", &nu::TextEdit::on_text_change,
                "shouldInsertNewLine", &nu::TextEdit::should_insert_new_line);
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

template<>
struct Type<node_yue::ChromeView> {
  using base = nu::View;
  static constexpr const char* name = "yue.ChromeView";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor,
        "create", &CreateOnHeap<node_yue::ChromeView, v8::Local<v8::Value>>);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
  }
};
#endif

}  // namespace vb

namespace node_yue {

bool GetRuntime(
    v8::Local<v8::Context> context, bool* is_electron, bool* is_yode) {
  v8::Local<v8::Object> process;
  if (!vb::Get(context, context->Global(), "process", &process) ||
      !process->IsObject())
    return false;
  v8::Local<v8::Object> versions;
  if (!vb::Get(context, process, "versions", &versions) ||
      !versions->IsObject())
    return false;
  std::string tmp;
  *is_electron =  vb::Get(context, versions, "electron", &tmp) && !tmp.empty();
  *is_yode =  vb::Get(context, versions, "yode", &tmp) && !tmp.empty();
  return true;
}

void MemoryPressureNotification(v8::Local<v8::Context> context, int level) {
  level = std::max(0, std::min(level, 2));
  context->GetIsolate()->MemoryPressureNotification(
      static_cast<v8::MemoryPressureLevel>(level));
}

void Initialize(v8::Local<v8::Object> exports) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  CHECK(GetRuntime(context, &is_electron, &is_yode));

#if defined(OS_WIN)
  if (!is_electron) {
    // Show system dialog on crash.
    SetErrorMode(GetErrorMode() & ~SEM_NOGPFAULTERRORBOX);
  }
#endif
  // Non-Electron platforms needs the lifetime API.
  if (!is_electron) {
    new nu::Lifetime;
  }
  // Initialize the nativeui and leak it.
  new nu::State;
  // Official node platform needs node integration.
  if (!is_electron && !is_yode) {
    // Initialize node integration and leak it.
    NodeIntegration* node_integration = NodeIntegration::Create();
    node_integration->PrepareMessageLoop();
    node_integration->RunMessageLoop();
  }

  vb::Set(context, exports,
          // Classes.
          "App",               vb::Constructor<nu::App>(),
          "Font",              vb::Constructor<nu::Font>(),
          "Canvas",            vb::Constructor<nu::Canvas>(),
          "Color",             vb::Constructor<nu::Color>(),
          "Image",             vb::Constructor<nu::Image>(),
          "Painter",           vb::Constructor<nu::Painter>(),
          "Event",             vb::Constructor<nu::Event>(),
          "FileDialog",        vb::Constructor<nu::FileDialog>(),
          "FileOpenDialog",    vb::Constructor<nu::FileOpenDialog>(),
          "FileSaveDialog",    vb::Constructor<nu::FileSaveDialog>(),
          "MenuBar",           vb::Constructor<nu::MenuBar>(),
          "Menu",              vb::Constructor<nu::Menu>(),
          "MenuItem",          vb::Constructor<nu::MenuItem>(),
          "Window",            vb::Constructor<nu::Window>(),
          "View",              vb::Constructor<nu::View>(),
          "Container",         vb::Constructor<nu::Container>(),
          "Button",            vb::Constructor<nu::Button>(),
          "ProtocolStringJob", vb::Constructor<nu::ProtocolStringJob>(),
          "ProtocolFileJob",   vb::Constructor<nu::ProtocolFileJob>(),
          "ProtocolAsarJob",   vb::Constructor<nu::ProtocolAsarJob>(),
          "Browser",           vb::Constructor<nu::Browser>(),
          "Entry",             vb::Constructor<nu::Entry>(),
          "Label",             vb::Constructor<nu::Label>(),
          "ProgressBar",       vb::Constructor<nu::ProgressBar>(),
          "GifPlayer",         vb::Constructor<nu::GifPlayer>(),
          "Group",             vb::Constructor<nu::Group>(),
          "Scroll",            vb::Constructor<nu::Scroll>(),
          "TextEdit",          vb::Constructor<nu::TextEdit>(),
          "Tray",              vb::Constructor<nu::Tray>(),
#if defined(OS_MACOSX)
          "Toolbar",           vb::Constructor<nu::Toolbar>(),
          "Vibrant",           vb::Constructor<nu::Vibrant>(),
#endif
          // Properties.
          "app",      nu::State::GetCurrent()->GetApp(),
          // Functions.
          "memoryPressureNotification", &MemoryPressureNotification);
  if (is_electron) {
#if defined(OS_MACOSX)
    vb::Set(context, exports,
            "ChromeView", vb::Constructor<node_yue::ChromeView>());
#endif
  } else {
    vb::Set(context, exports,
            "Lifetime", vb::Constructor<nu::Lifetime>(),
            "lifetime", nu::Lifetime::GetCurrent(),
            "MessageLoop", vb::Constructor<nu::MessageLoop>());
  }
}

}  // namespace node_yue

NODE_MODULE(gui, node_yue::Initialize)
