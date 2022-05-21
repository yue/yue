// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include <node.h>
#include <node_buffer.h>

#include "base/notreached.h"
#include "base/time/time.h"
#include "nativeui/nativeui.h"
#include "node_yue/binding_signal.h"
#include "node_yue/binding_values.h"
#include "node_yue/node_integration.h"

#if defined(OS_MAC)
#include "node_yue/chrome_view_mac.h"
#endif

#if defined(OS_LINUX) || defined(OS_MAC)
#include "base/environment.h"
#include "base/strings/string_number_conversions.h"
#endif

namespace {

bool is_electron = false;
bool is_yode = false;

}  // namespace

namespace vb {

template<>
struct Type<base::FilePath> {
  static constexpr const char* name = "FilePath";
  static v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                   const base::FilePath& value) {
    return vb::ToV8(context, value.value());
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
struct Type<base::Time> {
  static constexpr const char* name = "Time";
  static v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                   const base::Time& value) {
    v8::Local<v8::Value> date;
    if (v8::Date::New(context, value.ToJsTimeIgnoringNull()).ToLocal(&date))
      return date;
    else
      return v8::Null(context->GetIsolate());
  }
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     base::Time* out) {
    if (!value->IsDate())
      return false;
    *out = base::Time::FromJsTime(value.As<v8::Date>()->ValueOf());
    return true;
  }
};

template<>
struct Type<nu::Buffer> {
  static constexpr const char* name = "Buffer";
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
struct Type<nu::Display> {
  static constexpr const char* name = "Display";
  static v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                   const nu::Display& display) {
    auto obj = v8::Object::New(context->GetIsolate());
    Set(context, obj,
        "id", display.id,
        "scaleFactor", display.scale_factor,
#if defined(OS_MAC)
        "internal", display.internal,
#endif
        "bounds", display.bounds,
        "workArea", display.work_area);
    return obj;
  }
};

template<>
struct Type<nu::Size> {
  static constexpr const char* name = "Size";
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
    if (!ReadOptions(context, value.As<v8::Object>(),
                     "width", &width, "height", &height))
      return false;
    *out = nu::Size(width, height);
    return true;
  }
};

template<>
struct Type<nu::SizeF> {
  static constexpr const char* name = "SizeF";
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
    if (!ReadOptions(context, value.As<v8::Object>(),
                     "width", &width, "height", &height))
      return false;
    *out = nu::SizeF(width, height);
    return true;
  }
};

template<>
struct Type<nu::RectF> {
  static constexpr const char* name = "RectF";
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
    if (!ReadOptions(context, value.As<v8::Object>(),
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
    if (!ReadOptions(context, value.As<v8::Object>(), "x", &x, "y", &y))
      return false;
    *out = nu::Vector2dF(x, y);
    return true;
  }
};

template<>
struct Type<nu::PointF> {
  static constexpr const char* name = "PointF";
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
    if (!ReadOptions(context, value.As<v8::Object>(), "x", &x, "y", &y))
      return false;
    *out = nu::PointF(x, y);
    return true;
  }
};

template<>
struct Type<nu::ImageScale> {
  static constexpr const char* name = "ImageScale";
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::ImageScale* out) {
    std::string scale;
    if (!vb::FromV8(context, value, &scale))
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
  static v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                   nu::ImageScale scale) {
    switch (scale) {
      case nu::ImageScale::None:
        return vb::ToV8(context, "none");
      case nu::ImageScale::Fill:
        return vb::ToV8(context, "fill");
      case nu::ImageScale::Down:
        return vb::ToV8(context, "down");
      case nu::ImageScale::UpOrDown:
        return vb::ToV8(context, "up-or-down");
    }
    NOTREACHED();
    return v8::Undefined(context->GetIsolate());
  }
};

template<>
struct Type<nu::Orientation> {
  static constexpr const char* name = "Orientation";
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::Orientation* out) {
    std::string orientation;
    if (!vb::FromV8(context, value, &orientation))
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
  static v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                   nu::Orientation orientation) {
    switch (orientation) {
      case nu::Orientation::Horizontal:
        return vb::ToV8(context, "horizontal");
      case nu::Orientation::Vertical:
        return vb::ToV8(context, "vertical");
    }
    NOTREACHED();
    return v8::Undefined(context->GetIsolate());
  }
};

template<>
struct Type<nu::Accelerator> {
  static constexpr const char* name = "Accelerator";
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

#if defined(OS_MAC)
template<>
struct Type<nu::Lifetime::Reply> {
  static constexpr const char* name = "LifetimeReply";
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::Lifetime::Reply* out) {
    std::string reply;
    if (!vb::FromV8(context, value, &reply))
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
  static void BuildConstructor(v8::Local<v8::Context>, v8::Local<v8::Object>) {
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
#if defined(OS_MAC)
    SetProperty(context, templ,
                "onReady", &nu::Lifetime::on_ready,
                "onActivate", &nu::Lifetime::on_activate,
                "openFiles", &nu::Lifetime::open_files);
#endif
  }
};

template<>
struct Type<nu::MessageLoop> {
  static constexpr const char* name = "MessageLoop";
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

#if defined(OS_MAC)
template<>
struct Type<nu::App::ActivationPolicy> {
  static constexpr const char* name = "AppActivationPolicy";
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::App::ActivationPolicy* out) {
    std::string policy;
    if (!vb::FromV8(context, value, &policy))
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
  static v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                   nu::App::ActivationPolicy policy) {
    switch (policy) {
      case nu::App::ActivationPolicy::Regular:
        return vb::ToV8(context, "regular");
      case nu::App::ActivationPolicy::Accessory:
        return vb::ToV8(context, "accessory");
      case nu::App::ActivationPolicy::Prohibited:
        return vb::ToV8(context, "prohibited");
    }
    NOTREACHED();
    return v8::Undefined(context->GetIsolate());
  }
};
#endif

#if defined(OS_WIN)
template<>
struct Type<nu::App::ShortcutOptions> {
  static constexpr const char* name = "AppShortcutOptions";
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::App::ShortcutOptions* out) {
    if (!value->IsObject())
      return false;
    return ReadOptions(context, value.As<v8::Object>(),
                       "arguments", &out->arguments,
                       "description", &out->description,
                       "workingDir", &out->working_dir);
  }
};
#endif

template<>
struct Type<nu::App> {
  static constexpr const char* name = "App";
  static void BuildConstructor(v8::Local<v8::Context>, v8::Local<v8::Object>) {
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "setName", &nu::App::SetName,
        "getName", &nu::App::GetName,
#if defined(OS_LINUX) || defined(OS_WIN)
        "setID", &nu::App::SetID,
#endif
        "getID", &nu::App::GetID);
#if defined(OS_MAC)
    Set(context, templ,
        "setApplicationMenu",
        RefMethod(&nu::App::SetApplicationMenu, RefType::Reset, "appMenu"),
        "setDockBadgeLabel", &nu::App::SetDockBadgeLabel,
        "getDockBadgeLabel", &nu::App::GetDockBadgeLabel,
        "activate", &nu::App::Activate,
        "deactivate", &nu::App::Deactivate,
        "isActive", &nu::App::IsActive,
        "setActivationPolicy", &nu::App::SetActivationPolicy,
        "getActivationPolicy", &nu::App::GetActivationPolicy);
#elif defined(OS_WIN)
    Set(context, templ,
        "createStartMenuShortcut", &nu::App::CreateStartMenuShortcut,
        "getStartMenuShortcutPath", &nu::App::GetStartMenuShortcutPath);
#endif
  }
};

template<>
struct Type<nu::Appearance> {
  static constexpr const char* name = "Appearance";
  static void BuildConstructor(v8::Local<v8::Context>, v8::Local<v8::Object>) {
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
#if defined(OS_WIN)
        "setDarkModeEnabled", &nu::Appearance::SetDarkModeEnabled,
#endif
        "isDarkScheme", &nu::Appearance::IsDarkScheme);
    SetProperty(context, templ,
                "onColorSchemeChange", &nu::Appearance::on_color_scheme_change);
  }
};

template<>
struct Type<nu::AttributedText> {
  static constexpr const char* name = "AttributedText";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor,
        "create", &CreateOnHeap<nu::AttributedText,
                                const std::string&,
                                nu::TextAttributes>);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "setFont", &nu::AttributedText::SetFont,
#if !defined(OS_WIN)
        "setFontFor", &nu::AttributedText::SetFontFor,
#endif
        "setColor", &nu::AttributedText::SetColor,
#if !defined(OS_WIN)
        "setColorFor", &nu::AttributedText::SetColorFor,
#endif
        "clear", &nu::AttributedText::Clear,
        "getBoundsFor", &nu::AttributedText::GetBoundsFor,
        "setFormat", &nu::AttributedText::SetFormat,
        "getFormat", &nu::AttributedText::GetFormat,
        "setText", &nu::AttributedText::SetText,
        "getText", &nu::AttributedText::GetText);
  }
};

template<>
struct Type<nu::Font::Weight> {
  static constexpr const char* name = "FontWeight";
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
  static constexpr const char* name = "FontStyle";
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
  static constexpr const char* name = "Font";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor,
        "create", &CreateOnHeap<nu::Font, const std::string&, float,
                                nu::Font::Weight, nu::Font::Style>,
        "createFromPath", &CreateOnHeap<nu::Font, const base::FilePath&, float>,
        "default", &nu::Font::Default);
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
};

template<>
struct Type<nu::Canvas> {
  static constexpr const char* name = "Canvas";
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
struct Type<nu::Clipboard::Data::Type> {
  static constexpr const char* name = "ClipboardDataType";
  static v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                   nu::Clipboard::Data::Type type) {
    switch (type) {
      case nu::Clipboard::Data::Type::Text:
        return vb::ToV8(context, "text");
      case nu::Clipboard::Data::Type::HTML:
        return vb::ToV8(context, "html");
      case nu::Clipboard::Data::Type::Image:
        return vb::ToV8(context, "image");
      case nu::Clipboard::Data::Type::FilePaths:
        return vb::ToV8(context, "file-paths");
      default:
        return vb::ToV8(context, "none");
    }
  }
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::Clipboard::Data::Type* out) {
    std::string type;
    if (!vb::FromV8(context, value, &type))
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
  static v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                   const nu::Clipboard::Data& data) {
    v8::Local<v8::Object> obj = v8::Object::New(context->GetIsolate());
    Set(context, obj, "type", data.type());
    switch (data.type()) {
      case nu::Clipboard::Data::Type::Text:
      case nu::Clipboard::Data::Type::HTML:
        Set(context, obj, "value", data.str());
        break;
      case nu::Clipboard::Data::Type::Image:
        Set(context, obj, "value", data.image());
        break;
      case nu::Clipboard::Data::Type::FilePaths:
        Set(context, obj, "value", data.file_paths());
        break;
      default:
        Set(context, obj, "value", nullptr);
        break;
    }
    return obj;
  }
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::Clipboard::Data* out) {
    if (!value->IsObject())
      return false;
    v8::Local<v8::Object> obj = value.As<v8::Object>();
    nu::Clipboard::Data::Type type;
    if (!Get(context, obj, "type", &type))
      return false;
    switch (type) {
      case nu::Clipboard::Data::Type::Text:
      case nu::Clipboard::Data::Type::HTML: {
        std::string str;
        if (!Get(context, obj, "value", &str))
          return false;
        *out = nu::Clipboard::Data(type, std::move(str));
        break;
      }
      case nu::Clipboard::Data::Type::Image: {
        nu::Image* image;
        if (!Get(context, obj, "value", &image))
          return false;
        *out = nu::Clipboard::Data(image);
        break;
      }
      case nu::Clipboard::Data::Type::FilePaths: {
        std::vector<base::FilePath> file_paths;
        if (!Get(context, obj, "value", &file_paths))
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
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::Clipboard::Type* out) {
    std::string type;
    if (!vb::FromV8(context, value, &type))
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
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor,
        "get", &nu::Clipboard::Get,
        "fromType", &nu::Clipboard::FromType);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "clear", &nu::Clipboard::Clear,
        "setText", &nu::Clipboard::SetText,
        "getText", &nu::Clipboard::GetText,
        "isDataAvailable", &nu::Clipboard::IsDataAvailable,
        "getData", &nu::Clipboard::GetData,
        "setData", &nu::Clipboard::SetData,
        "startWatching", &nu::Clipboard::StartWatching,
        "stopWatching", &nu::Clipboard::StopWatching);
    SetProperty(context, templ,
                "onChange", &nu::Clipboard::on_change);
  }
};

template<>
struct Type<nu::Color::Name> {
  static constexpr const char* name = "ColorName";
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::Color::Name* out) {
    std::string id;
    if (!vb::FromV8(context, value, &id))
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
        "get", &nu::Color::Get,
        "rgb", &CreateOnStack<nu::Color, unsigned, unsigned, unsigned>,
        "argb", &CreateOnStack<nu::Color, unsigned, unsigned, unsigned,
                               unsigned>);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
  }
};

template<>
struct Type<nu::Cursor::Type> {
  static constexpr const char* name = "CursorType";
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::Cursor::Type* out) {
    std::string type;
    if (!vb::FromV8(context, value, &type))
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
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor,
        "createWithType", &CreateOnHeap<nu::Cursor, nu::Cursor::Type>);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
  }
};

template<>
struct Type<nu::DraggingInfo> {
  static constexpr const char* name = "DraggingInfo";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor,
        "dragOperationNone", static_cast<int>(nu::DRAG_OPERATION_NONE),
        "dragOperationCopy", static_cast<int>(nu::DRAG_OPERATION_COPY),
        "dragOperationMove", static_cast<int>(nu::DRAG_OPERATION_MOVE),
        "dragOperationLink", static_cast<int>(nu::DRAG_OPERATION_LINK));
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "isDataAvailable", &nu::DraggingInfo::IsDataAvailable,
        "getData", &nu::DraggingInfo::GetData,
        "getDragOperations", &nu::DraggingInfo::GetDragOperations);
  }
};

template<>
struct Type<nu::DragOptions> {
  static constexpr const char* name = "DragOptions";
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::DragOptions* out) {
    if (!value->IsObject())
      return false;
    return ReadOptions(context, value.As<v8::Object>(),
                       "image", &out->image);
  }
};

template<>
struct Type<nu::Image> {
  static constexpr const char* name = "Image";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor,
        "createEmpty", &CreateOnHeap<nu::Image>,
        "createFromPath", &CreateOnHeap<nu::Image, const base::FilePath&>,
        "createFromBuffer", &CreateOnHeap<nu::Image, const nu::Buffer&, float>);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "isEmpty", &nu::Image::IsEmpty,
        "getSize", &nu::Image::GetSize,
        "getScaleFactor", &nu::Image::GetScaleFactor);
  }
};

template<>
struct Type<nu::TextAlign> {
  static constexpr const char* name = "TextAlign";
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
  static v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                   nu::TextAlign align) {
    switch (align) {
      case nu::TextAlign::Center:
        return vb::ToV8(context, "center");
      case nu::TextAlign::End:
        return vb::ToV8(context, "end");
      default:
        return vb::ToV8(context, "start");
    }
  }
};

template<>
struct Type<nu::TextFormat> {
  static constexpr const char* name = "TextFormat";
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::TextFormat* out) {
    if (!value->IsObject())
      return false;
    return ReadOptions(context, value.As<v8::Object>(),
                       "align", &out->align,
                       "valign", &out->valign,
                       "wrap", &out->wrap,
                       "ellipsis", &out->ellipsis);
  }
  static v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                   const nu::TextFormat& options) {
    v8::Local<v8::Object> obj = v8::Object::New(context->GetIsolate());
    Set(context, obj, "align", options.align);
    Set(context, obj, "valign", options.valign);
    Set(context, obj, "wrap", options.wrap);
    Set(context, obj, "ellipsis", options.ellipsis);
    return obj;
  }
};

template<>
struct Type<nu::TextAttributes> {
  static constexpr const char* name = "TextAttributes";
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::TextAttributes* out) {
    if (!Type<nu::TextFormat>::FromV8(context, value, out))
      return false;
    return ReadOptions(context, value.As<v8::Object>(),
                       "font", &out->font,
                       "color", &out->color);
  }
};

template<>
struct Type<nu::Painter> {
  static constexpr const char* name = "Painter";
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
        "clear", &nu::Painter::Clear,
        "strokeRect", &nu::Painter::StrokeRect,
        "fillRect", &nu::Painter::FillRect,
        "drawImage", &nu::Painter::DrawImage,
        "drawImageFromRect", &nu::Painter::DrawImageFromRect,
        "drawCanvas", &nu::Painter::DrawCanvas,
        "drawCanvasFromRect", &nu::Painter::DrawCanvasFromRect,
        "drawAttributedText", &nu::Painter::DrawAttributedText,
        "drawText", &nu::Painter::DrawText);
  }
};

template<>
struct Type<nu::EventType> {
  static constexpr const char* name = "EventType";
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
  static constexpr const char* name = "Event";
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
  static constexpr const char* name = "MouseEvent";
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
  static constexpr const char* name = "KeyboardCode";
  static v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                   nu::KeyboardCode code) {
    return vb::ToV8(context, nu::KeyboardCodeToStr(code));
  }
};

template<>
struct Type<nu::KeyEvent> {
  using base = nu::Event;
  static constexpr const char* name = "KeyEvent";
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
  static constexpr const char* name = "FileDialogFilter";
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
  static constexpr const char* name = "FileDialog";
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
  static constexpr const char* name = "FileOpenDialog";
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
  static constexpr const char* name = "FileSaveDialog";
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
  static constexpr const char* name = "MenuBase";
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
  static constexpr const char* name = "MenuBar";
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
  static constexpr const char* name = "Menu";
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
  static constexpr const char* name = "MenuItemType";
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
  static constexpr const char* name = "MenuItemRole";
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
#if defined(OS_MAC) || defined(OS_WIN)
        "setImage", &nu::MenuItem::SetImage,
        "getImage", &nu::MenuItem::GetImage,
#endif
        "setAccelerator", &nu::MenuItem::SetAccelerator);
    SetProperty(context, templ,
                "onClick", &nu::MenuItem::on_click,
                "validate", &nu::MenuItem::validate);
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
#if defined(OS_MAC) || defined(OS_WIN)
    nu::Image* image;
    if (Get(context, obj, "image", &image))
      item->SetImage(image);
#endif
    // The signal handler must not be referenced by C++.
    v8::Local<v8::Value> on_click_val;
    std::function<void(nu::MenuItem*)> on_click;
    if (Get(context, obj, "onClick", &on_click_val) &&
        WeakFunctionFromV8(context, on_click_val, &on_click))
      item->on_click.Connect(on_click);
    v8::Local<v8::Value> validate_val;
    std::function<bool(nu::MenuItem*)> validate;
    if (Get(context, obj, "validate", &validate_val) &&
        WeakFunctionFromV8(context, validate_val, &validate))
      item->validate = validate;
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
struct Type<nu::MessageBox::Type> {
  static constexpr const char* name = "MessageBoxType";
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::MessageBox::Type* out) {
    std::string type;
    if (!vb::FromV8(context, value, &type))
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
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor, "create", &CreateOnHeap<nu::MessageBox>);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "run", &nu::MessageBox::Run,
        "runForWindow", &nu::MessageBox::RunForWindow,
#if defined(OS_LINUX) || defined(OS_WIN)
        "show", &nu::MessageBox::Show,
#endif
        "showForWindow", &nu::MessageBox::ShowForWindow,
        "close", &nu::MessageBox::Close,
        "setType", &nu::MessageBox::SetType,
#if defined(OS_LINUX) || defined(OS_WIN)
        "setTitle", &nu::MessageBox::SetTitle,
#endif
        "addButton", &nu::MessageBox::AddButton,
        "setDefaultResponse", &nu::MessageBox::SetDefaultResponse,
        "setCancelResponse", &nu::MessageBox::SetCancelResponse,
        "setText", &nu::MessageBox::SetText,
        "setInformativeText", &nu::MessageBox::SetInformativeText,
#if defined(OS_LINUX) || defined(OS_MAC)
        "setAccessoryView",
        RefMethod(&nu::MessageBox::SetAccessoryView, RefType::Reset, "accv"),
        "getAccessoryView", &nu::MessageBox::GetAccessoryView,
#endif
        "setImage", &nu::MessageBox::SetImage,
        "getImage", &nu::MessageBox::GetImage);
    SetProperty(context, templ, "onResponse", &nu::MessageBox::on_response);
  }
};

template<>
struct Type<nu::Notification::Action> {
  static constexpr const char* name = "NotificationAction";
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::Notification::Action* out) {
    if (!value->IsObject())
      return false;
    return Get(context, value.As<v8::Object>(),
               "title", &out->title,
               "info", &out->info);
  }
};

template<>
struct Type<nu::Notification> {
  static constexpr const char* name = "Notification";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor, "create", &CreateOnHeap<nu::Notification>);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "show", &nu::Notification::Show,
        "close", &nu::Notification::Close,
        "setTitle", &nu::Notification::SetTitle,
        "setBody", &nu::Notification::SetBody,
        "setInfo", &nu::Notification::SetInfo,
        "getInfo", &nu::Notification::GetInfo,
        "setSilent", &nu::Notification::SetSilent,
        "setImage", &nu::Notification::SetImage,
        "setImagePath", &nu::Notification::SetImagePath,
#if defined(OS_MAC) || defined(OS_WIN)
        "setHasReplyButton", &nu::Notification::SetHasReplyButton,
        "setResponsePlaceholder", &nu::Notification::SetResponsePlaceholder,
        "setIdentifier", &nu::Notification::SetIdentifier,
        "getIdentifier", &nu::Notification::GetIdentifier,
#endif
#if defined(OS_WIN)
        "setImagePlacement", &nu::Notification::SetImagePlacement,
        "setXML", &nu::Notification::SetXML,
        "getXML", &nu::Notification::GetXML,
#endif
        "setActions", &nu::Notification::SetActions);
  }
};

#if defined(OS_WIN)
template<>
struct Type<nu::NotificationCenter::COMServerOptions> {
  static constexpr const char* name = "NotificationCenterCOMServerOptions";
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::NotificationCenter::COMServerOptions* out) {
    if (!value->IsObject())
      return false;
    return ReadOptions(context, value.As<v8::Object>(),
                       "writeRegistry", &out->write_registry,
                       "arguments", &out->arguments,
                       "toastActivatorClsid", &out->toast_activator_clsid);
  }
};

template<>
struct Type<nu::NotificationCenter::InputData> {
  static constexpr const char* name = "NotificationCenterInputData";
  static v8::Local<v8::Value> ToV8(
      v8::Local<v8::Context> context,
      const nu::NotificationCenter::InputData& data) {
    auto obj = v8::Object::New(context->GetIsolate());
    Set(context, obj, "key", data.key, "value", data.value);
    return obj;
  }
};
#endif

template<>
struct Type<nu::NotificationCenter> {
  static constexpr const char* name = "NotificationCenter";
  static void BuildConstructor(v8::Local<v8::Context>, v8::Local<v8::Object>) {
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "clear", &nu::NotificationCenter::Clear);
#if defined(OS_WIN)
    Set(context, templ,
        "setCOMServerOptions",
        &nu::NotificationCenter::SetCOMServerOptions,
        "registerCOMServer",
        &nu::NotificationCenter::RegisterCOMServer,
        "removeCOMServerFromRegistry",
        &nu::NotificationCenter::RemoveCOMServerFromRegistry,
        "getToastActivatorCLSID",
        &nu::NotificationCenter::GetToastActivatorCLSID);
#endif
    SetProperty(context, templ,
                "onNotificationShow",
                &nu::NotificationCenter::on_notification_show,
                "onNotificationClose",
                &nu::NotificationCenter::on_notification_close,
                "onNotificationClick",
                &nu::NotificationCenter::on_notification_click,
                "onNotificationAction",
                &nu::NotificationCenter::on_notification_action);
#if defined(OS_MAC) || defined(OS_WIN)
    SetProperty(context, templ,
                "onNotificationReply",
                &nu::NotificationCenter::on_notification_reply);
#endif
#if defined(OS_WIN)
    SetProperty(context, templ,
                "onToastActivate", &nu::NotificationCenter::on_toast_activate);
#endif
  }
};

template<>
struct Type<nu::Tray> {
  static constexpr const char* name = "Tray";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor,
#if defined(OS_MAC)
        "createWithTitle", &CreateOnHeap<nu::Tray, const std::string&>,
#endif
        "createWithImage", &CreateOnHeap<nu::Tray, scoped_refptr<nu::Image>>);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "remove", &nu::Tray::Remove,
#if defined(OS_MAC) || defined(OS_WIN)
        "getBounds", &nu::Tray::GetBounds,
#endif
#if defined(OS_MAC) || defined(OS_LINUX)
        "setTitle", &nu::Tray::SetTitle,
#endif
        "setImage", &nu::Tray::SetImage,
#if defined(OS_MAC)
        "setPressedImage", &nu::Tray::SetPressedImage,
#endif
        "setMenu", RefMethod(&nu::Tray::SetMenu, RefType::Reset, "menu"));
    SetProperty(context, templ, "onClick", &nu::Tray::on_click);
  }
};

#if defined(OS_MAC)
template<>
struct Type<nu::Toolbar::Item> {
  static constexpr const char* name = "ToolbarItem";
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::Toolbar::Item* out) {
    if (!value->IsObject())
      return false;
    return ReadOptions(context, value.As<v8::Object>(),
                       "image", &out->image,
                       "view", &out->view,
                       "label", &out->label,
                       "minSize", &out->min_size,
                       "maxSize", &out->max_size,
                       "subitems", &out->subitems,
                       "onClick", &out->on_click);
  }
};

template<>
struct Type<nu::Toolbar::DisplayMode> {
  static constexpr const char* name = "ToolbarDisplayMode";
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
  static constexpr const char* name = "Toolbar";
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
struct Type<nu::Responder> {
  static constexpr const char* name = "Responder";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "setCapture", &nu::Responder::SetCapture,
        "releaseCapture", &nu::Responder::ReleaseCapture,
        "hasCapture", &nu::Responder::HasCapture);
    SetProperty(context, templ,
                "onMouseDown", &nu::Responder::on_mouse_down,
                "onMouseUp", &nu::Responder::on_mouse_up,
                "onMouseMove", &nu::Responder::on_mouse_move,
                "onMouseEnter", &nu::Responder::on_mouse_enter,
                "onMouseLeave", &nu::Responder::on_mouse_leave,
                "onKeyDown", &nu::Responder::on_key_down,
                "onKeyUp", &nu::Responder::on_key_up,
                "onCaptureLost", &nu::View::on_capture_lost);
  }
};

template<>
struct Type<nu::Window::Options> {
  static constexpr const char* name = "WindowOptions";
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::Window::Options* out) {
    if (!value->IsObject())
      return false;
    return ReadOptions(context, value.As<v8::Object>(),
                       "frame", &out->frame,
#if defined(OS_MAC)
                       "showTrafficLights", &out->show_traffic_lights,
#endif
                       "transparent", &out->transparent);
  }
};

template<>
struct Type<nu::Window> {
  using base = nu::Responder;
  static constexpr const char* name = "Window";
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
#if defined(OS_MAC)
        "setToolbar", &nu::Window::SetToolbar,
        "getToolbar", &nu::Window::GetToolbar,
        "setTitleVisible", &nu::Window::SetTitleVisible,
        "isTitleVisible", &nu::Window::IsTitleVisible,
        "setFullSizeContentView", &nu::Window::SetFullSizeContentView,
        "isFullSizeContentView", &nu::Window::IsFullSizeContentView,
#endif
#if defined(OS_WIN) || defined(OS_LINUX)
        "setSkipTaskbar", &nu::Window::SetSkipTaskbar,
        "setIcon", &nu::Window::SetIcon,
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
  using base = nu::Responder;
  static constexpr const char* name = "View";
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
        "setMouseDownCanMoveWindow", &nu::View::SetMouseDownCanMoveWindow,
        "isMouseDownCanMoveWindow", &nu::View::IsMouseDownCanMoveWindow,
        "doDrag", &nu::View::DoDrag,
        "doDragWithOptions", &nu::View::DoDragWithOptions,
        "cancelDrag", &nu::View::CancelDrag,
        "isDragging", &nu::View::IsDragging,
        "registerDraggedTypes", &nu::View::RegisterDraggedTypes,
        "setCursor", &nu::View::SetCursor,
        "setFont", &nu::View::SetFont,
        "setColor", &nu::View::SetColor,
        "setBackgroundColor", &nu::View::SetBackgroundColor,
        "setStyle", &SetStyle,
        "getComputedLayout", &nu::View::GetComputedLayout,
        "getMinimumSize", &nu::View::GetMinimumSize,
#if defined(OS_MAC)
        "setWantsLayer", &nu::View::SetWantsLayer,
        "wantsLayer", &nu::View::WantsLayer,
#endif
        "getParent", &nu::View::GetParent,
        "getWindow", &nu::View::GetWindow);
    SetProperty(context, templ,
                "onDragLeave", &nu::View::on_drag_leave,
                "onSizeChanged", &nu::View::on_size_changed,
                "handleDragEnter", &nu::View::handle_drag_enter,
                "handleDragUpdate", &nu::View::handle_drag_update,
                "handleDrop", &nu::View::handle_drop);
  }
  static void SetStyle(
      Arguments* args,
      v8::Local<v8::Context> context,
      const std::map<std::string, v8::Local<v8::Value>>& styles) {
    nu::View* view;
    if (!args->GetHolder(&view))
      return;
    for (const auto& it : styles) {
      if (it.second->IsNumber())
        view->SetStyleProperty(
            it.first, it.second->NumberValue(context).ToChecked());
      else
        view->SetStyleProperty(
            it.first, *v8::String::Utf8Value(args->isolate(), it.second));
    }
    view->Layout();
  }
};

template<>
struct Type<nu::ComboBox> {
  using base = nu::Picker;
  static constexpr const char* name = "ComboBox";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor, "create", &CreateOnHeap<nu::ComboBox>);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "setText", &nu::ComboBox::SetText,
        "getText", &nu::ComboBox::GetText);
    SetProperty(context, templ,
                "onTextChange", &nu::ComboBox::on_text_change);
  }
};

template<>
struct Type<nu::Container> {
  using base = nu::View;
  static constexpr const char* name = "Container";
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
  static constexpr const char* name = "ButtonType";
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

#if defined(OS_MAC)
template<>
struct Type<nu::Button::Style> {
  static constexpr const char* name = "ButtonStyle";
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
  static constexpr const char* name = "Button";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor, "create", &Create);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "makeDefault", &nu::Button::MakeDefault,
        "setTitle", &nu::Button::SetTitle,
        "getTitle", &nu::Button::GetTitle,
#if defined(OS_MAC)
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
      nu::Button::Type type = nu::Button::Type::Normal;
      ReadOptions(context, value.As<v8::Object>(),
                  "title", &title,
                  "type", &type);
      return new nu::Button(title, type);
    } else {
      args->ThrowError("String or Object");
      return nullptr;
    }
  }
};

template<>
struct Type<nu::ProtocolJob> {
  static constexpr const char* name = "ProtocolJob";
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
  static constexpr const char* name = "ProtocolStringJob";
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
  static constexpr const char* name = "ProtocolFileJob";
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
  static constexpr const char* name = "ProtocolAsarJob";
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
  static constexpr const char* name = "BrowserOptions";
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::Browser::Options* out) {
    if (!value->IsObject())
      return false;
    return ReadOptions(
        context, value.As<v8::Object>(),
#if defined(OS_MAC) || defined(OS_LINUX)
        "allowFileAccessFromFiles", &out->allow_file_access_from_files,
#endif
#if defined(OS_LINUX)
        "hardwareAcceleration", &out->hardware_acceleration,
#endif
#if defined(OS_WIN) && defined(WEBVIEW2_SUPPORT)
        "webview2Support", &out->webview2_support,
#endif
        "devtools", &out->devtools,
        "contextMenu", &out->context_menu);
  }
};

template<>
struct Type<nu::Browser> {
  using base = nu::View;
  static constexpr const char* name = "Browser";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor,
        "create", &CreateOnHeap<nu::Browser, nu::Browser::Options>,
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
#if defined(OS_MAC)
        "isMagnifiable", &nu::Browser::IsMagnifiable,
        "setMagnifiable", &nu::Browser::SetMagnifiable,
#endif
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
                         const std::string& bname,
                         v8::Local<v8::Function> func) {
    nu::Browser* browser;
    if (!args->GetHolder(&browser))
      return;
    // this[bindings][bname] = func.
    v8::Local<v8::Context> context = args->GetContext();
    v8::Local<v8::Map> refs = vb::GetAttachedTable(
        context, args->This(), "bindings");
    std::ignore = refs->Set(context, ToV8(context, bname), func);
    // The func must be stored as weak reference.
    v8::Isolate* isolate = args->isolate();
    std::shared_ptr<internal::V8FunctionWrapper> func_ref(
        new internal::V8FunctionWrapper(isolate, func));
    func_ref->SetWeak();
    // Parse base::Value and call func with v8 args.
    browser->AddRawBinding(bname, [isolate, func_ref](nu::Browser* browser,
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
                         &args.front(), {0, 0});
    });
  }
  static void AddRawBinding(Arguments* args,
                            const std::string& bname,
                            v8::Local<v8::Function> func) {
    nu::Browser* browser;
    if (!args->GetHolder(&browser))
      return;
    // this[bindings][bname] = func
    v8::Local<v8::Context> context = args->GetContext();
    v8::Local<v8::Map> refs = vb::GetAttachedTable(
        context, args->This(), "bindings");
    std::ignore = refs->Set(context, ToV8(context, bname), func);
    // The func must be stored as weak reference.
    nu::Browser::BindingFunc callback;
    WeakFunctionFromV8(context, func, &callback);
    browser->AddRawBinding(bname, callback);
  }
  static void RemoveBinding(Arguments* args, const std::string& bname) {
    nu::Browser* browser;
    if (!args->GetHolder(&browser))
      return;
    // delete this[bindings][bname]
    v8::Local<v8::Context> context = args->GetContext();
    v8::Local<v8::Map> refs = vb::GetAttachedTable(
        context, args->This(), "bindings");
    std::ignore = refs->Delete(context, ToV8(context, bname));
    // Pass down.
    browser->RemoveBinding(bname);
  }
};

template<>
struct Type<nu::DatePicker::Options> {
  static constexpr const char* name = "DatePickerOptions";
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::DatePicker::Options* out) {
    if (!value->IsObject())
      return false;
    return ReadOptions(context, value.As<v8::Object>(),
                       "elements", &out->elements,
                       "hasStepper", &out->has_stepper);
  }
};

template<>
struct Type<nu::DatePicker> {
  using base = nu::View;
  static constexpr const char* name = "DatePicker";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor,
        "elementYearMonth",
        static_cast<int>(nu::DatePicker::ELEMENT_YEAR_MONTH),
        "elementYearMonthDay",
        static_cast<int>(nu::DatePicker::ELEMENT_YEAR_MONTH_DAY),
        "elementHourMinute",
        static_cast<int>(nu::DatePicker::ELEMENT_HOUR_MINUTE),
        "elementHourMinuteSecond",
        static_cast<int>(nu::DatePicker::ELEMENT_HOUR_MINUTE_SECOND),
        "create",
        &CreateOnHeap<nu::DatePicker, const nu::DatePicker::Options&>);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "setDate", &nu::DatePicker::SetDate,
        "getDate", &nu::DatePicker::GetDate,
        "setRange", &nu::DatePicker::SetRange,
        "getRange", &nu::DatePicker::GetRange,
        "hasStepper", &nu::DatePicker::HasStepper);
    SetProperty(context, templ,
                "onDateChange", &nu::DatePicker::on_date_change);
  }
};

template<>
struct Type<nu::Entry::Type> {
  static constexpr const char* name = "EntryType";
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
  static constexpr const char* name = "Entry";
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
  static constexpr const char* name = "Label";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor,
        "create", &CreateOnHeap<nu::Label, const std::string&>,
        "createWithAttributedText",
         &CreateOnHeap<nu::Label, nu::AttributedText*>);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "setText", &nu::Label::SetText,
        "getText", &nu::Label::GetText,
        "setAlign", &nu::Label::SetAlign,
        "setVAlign", &nu::Label::SetVAlign,
        "setAttributedText",
        RefMethod(&nu::Label::SetAttributedText, RefType::Reset, "atext"),
        "getAttributedText", &nu::Label::GetAttributedText);
  }
};

template<>
struct Type<nu::ProgressBar> {
  using base = nu::View;
  static constexpr const char* name = "ProgressBar";
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
struct Type<nu::Picker> {
  using base = nu::View;
  static constexpr const char* name = "Picker";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor, "create", &CreateOnHeap<nu::Picker>);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "addItem", &nu::Picker::AddItem,
        "removeItemAt", &nu::Picker::RemoveItemAt,
        "getItems", &nu::Picker::GetItems,
        "selectItemAt", &nu::Picker::SelectItemAt,
        "getSelectedItem", &nu::Picker::GetSelectedItem,
        "getSelectedItemIndex", &nu::Picker::GetSelectedItemIndex);
    SetProperty(context, templ,
                "onSelectionChange", &nu::Picker::on_selection_change);
  }
};

template<>
struct Type<nu::GifPlayer> {
  using base = nu::View;
  static constexpr const char* name = "GifPlayer";
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
        "isAnimating", &nu::GifPlayer::IsAnimating,
        "setScale", &nu::GifPlayer::SetScale,
        "getScale", &nu::GifPlayer::GetScale);
  }
};

template<>
struct Type<nu::Group> {
  using base = nu::View;
  static constexpr const char* name = "Group";
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
struct Type<nu::Screen> {
  static constexpr const char* name = "Screen";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "getPrimaryDisplay", &nu::Screen::GetPrimaryDisplay,
        "getAllDisplays", &nu::Screen::GetAllDisplays,
        "getDisplayNearestWindow", &nu::Screen::GetDisplayNearestWindow,
        "getDisplayNearestPoint", &nu::Screen::GetDisplayNearestPoint,
        "getCursorScreenPoint", &nu::Screen::GetCursorScreenPoint);
    SetProperty(context, templ,
                "onAddDisplay", &nu::Screen::on_add_display,
                "onRemoveDisplay", &nu::Screen::on_remove_display,
                "onUpdateDisplay", &nu::Screen::on_update_display);
  }
};

template<>
struct Type<nu::Scroll::Policy> {
  static constexpr const char* name = "ScrollPolicy";
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

#if defined(OS_MAC)
template<>
struct Type<nu::Scroll::Elasticity> {
  static constexpr const char* name = "ScrollElasticity";
  static v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                   nu::Scroll::Elasticity elasticity) {
    if (elasticity == nu::Scroll::Elasticity::Automatic)
      return vb::ToV8(context, "automatic");
    else if (elasticity == nu::Scroll::Elasticity::None)
      return vb::ToV8(context, "none");
    else
      return vb::ToV8(context, "allowed");
  }
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::Scroll::Elasticity* out) {
    std::string elasticity;
    if (!vb::FromV8(context, value, &elasticity))
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
};
#endif

template<>
struct Type<nu::Scroll> {
  using base = nu::View;
  static constexpr const char* name = "Scroll";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor, "create", &CreateOnHeap<nu::Scroll>);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "setContentSize", &nu::Scroll::SetContentSize,
        "getContentSize", &nu::Scroll::GetContentSize,
        "setScrollPosition", &nu::Scroll::SetScrollPosition,
        "getScrollPosition", &nu::Scroll::GetScrollPosition,
        "getMaximumScrollPosition", &nu::Scroll::GetMaximumScrollPosition,
        "setContentView",
        RefMethod(&nu::Scroll::SetContentView, RefType::Reset, "contentView"),
        "getContentView", &nu::Scroll::GetContentView,
#if !defined(OS_WIN)
        "setOverlayScrollbar", &nu::Scroll::SetOverlayScrollbar,
        "isOverlayScrollbar", &nu::Scroll::IsOverlayScrollbar,
#endif
#if defined(OS_MAC)
        "setScrollElasticity", &nu::Scroll::SetScrollElasticity,
        "getScrollElasticity", &nu::Scroll::GetScrollElasticity,
#endif
        "setScrollbarPolicy", &nu::Scroll::SetScrollbarPolicy,
        "getScrollbarPolicy", &nu::Scroll::GetScrollbarPolicy);
    SetProperty(context, templ,
                "onScroll", &nu::Scroll::on_scroll);
  }
};

template<>
struct Type<nu::Separator> {
  using base = nu::View;
  static constexpr const char* name = "Separator";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor,
        "create", &CreateOnHeap<nu::Separator, nu::Orientation>);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
  }
};

template<>
struct Type<nu::Slider> {
  using base = nu::View;
  static constexpr const char* name = "Slider";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor, "create", &CreateOnHeap<nu::Slider>);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "setValue", &nu::Slider::SetValue,
        "getValue", &nu::Slider::GetValue,
        "setStep", &nu::Slider::SetStep,
        "getStep", &nu::Slider::GetStep,
        "setRange", &nu::Slider::SetRange,
        "getRange", &nu::Slider::GetRange);
    SetProperty(context, templ,
                "onValueChange", &nu::Slider::on_value_change,
                "onSlidingComplete", &nu::Slider::on_sliding_complete);
  }
};

template<>
struct Type<nu::Tab> {
  using base = nu::View;
  static constexpr const char* name = "Tab";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor, "create", &CreateOnHeap<nu::Tab>);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "addPage", RefMethod(&nu::Tab::AddPage, RefType::Ref),
        "removePage", RefMethod(&nu::Tab::RemovePage, RefType::Deref),
        "pageCount", &nu::Tab::PageCount,
        "pageAt", &nu::Tab::PageAt,
        "selectPageAt", &nu::Tab::SelectPageAt,
        "getSelectedPage", &nu::Tab::GetSelectedPage,
        "getSelectedPageIndex", &nu::Tab::GetSelectedPageIndex);
    SetProperty(context, templ,
                "onSelectedPageChange", &nu::Tab::on_selected_page_change);
  }
};

template<>
struct Type<nu::TableModel> {
  static constexpr const char* name = "TableModel";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "getRowCount", &nu::TableModel::GetRowCount,
        "getValue", &nu::TableModel::GetValue,
        "notifyRowInsertion", &nu::TableModel::NotifyRowInsertion,
        "notifyRowDeletion", &nu::TableModel::NotifyRowDeletion,
        "notifyValueChange", &nu::TableModel::NotifyValueChange);
  }
};

template<>
struct Type<nu::AbstractTableModel> {
  using base = nu::TableModel;
  static constexpr const char* name = "AbstractTableModel";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor, "create", &CreateOnHeap<nu::AbstractTableModel>);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    SetProperty(context, templ,
                "getRowCount", &nu::AbstractTableModel::get_row_count,
                "setValue", &nu::AbstractTableModel::set_value,
                "getValue", &nu::AbstractTableModel::get_value);
  }
};

template<>
struct Type<nu::SimpleTableModel> {
  using base = nu::TableModel;
  static constexpr const char* name = "SimpleTableModel";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor,
        "create", &CreateOnHeap<nu::SimpleTableModel, uint32_t>);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "addRow", &nu::SimpleTableModel::AddRow,
        "removeRowAt", &nu::SimpleTableModel::RemoveRowAt,
        "setValue", &nu::SimpleTableModel::SetValue);
  }
};

template<>
struct Type<nu::Table::ColumnType> {
  static constexpr const char* name = "TableColumnType";
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::Table::ColumnType* out) {
    std::string type;
    if (!vb::FromV8(context, value, &type))
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
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::Table::ColumnOptions* out) {
    if (!value->IsObject())
      return false;
    auto obj = value.As<v8::Object>();
    v8::Local<v8::Value> on_draw_val;
    if (Get(context, obj, "onDraw", &on_draw_val))
      WeakFunctionFromV8(context, on_draw_val, &out->on_draw);
    return ReadOptions(context, obj,
                       "type", &out->type,
                       "column", &out->column,
                       "width", &out->width);
  }
};

template<>
struct Type<nu::Table> {
  using base = nu::View;
  static constexpr const char* name = "Table";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor, "create", &CreateOnHeap<nu::Table>);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "setModel", RefMethod(&nu::Table::SetModel, RefType::Reset, "model"),
        "getModel", &nu::Table::GetModel,
        "addColumn", &nu::Table::AddColumn,
        "addColumnWithOptions",
        RefMethod(&nu::Table::AddColumnWithOptions, RefType::Ref, nullptr, 1),
        "getColumnCount", &nu::Table::GetColumnCount,
        "setColumnsVisible", &nu::Table::SetColumnsVisible,
        "isColumnsVisible", &nu::Table::IsColumnsVisible,
        "setRowHeight", &nu::Table::SetRowHeight,
        "getRowHeight", &nu::Table::GetRowHeight,
        "selectRow", &nu::Table::SelectRow,
        "getSelectedRow", &nu::Table::GetSelectedRow);
  }
};

template<>
struct Type<nu::TextEdit> {
  using base = nu::View;
  static constexpr const char* name = "TextEdit";
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
#if defined(OS_MAC)
        "setScrollElasticity", &nu::TextEdit::SetScrollElasticity,
        "getScrollElasticity", &nu::TextEdit::GetScrollElasticity,
#endif
        "getTextBounds", &nu::TextEdit::GetTextBounds);
    SetProperty(context, templ,
                "onTextChange", &nu::TextEdit::on_text_change,
                "shouldInsertNewLine", &nu::TextEdit::should_insert_new_line);
  }
};

#if defined(OS_MAC)
template<>
struct Type<nu::Vibrant::Material> {
  static constexpr const char* name = "VibrantMaterial";
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
  static constexpr const char* name = "VibrantBlendingMode";
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
  static constexpr const char* name = "Vibrant";
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
  static constexpr const char* name = "ChromeView";
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
  *is_electron = vb::Get(context, versions, "electron", &tmp) && !tmp.empty();
  *is_yode =  vb::Get(context, versions, "yode", &tmp) && !tmp.empty();
  return true;
}

void MemoryPressureNotification(v8::Local<v8::Context> context, int level) {
  level = std::max(0, std::min(level, 2));
  context->GetIsolate()->MemoryPressureNotification(
      static_cast<v8::MemoryPressureLevel>(level));
}

void Initialize(v8::Local<v8::Object> exports,
                v8::Local<v8::Value> unused,
                v8::Local<v8::Context> context,
                void* priv) {
  CHECK(GetRuntime(context, &is_electron, &is_yode));

#if defined(OS_LINUX)
  // Both node and WebKit are using SIGUSR1, avoid conflict.
  std::unique_ptr<base::Environment> env = base::Environment::Create();
  if (!env->HasVar("JSC_SIGNAL_FOR_GC"))
    env->SetVar("JSC_SIGNAL_FOR_GC", base::NumberToString(SIGUSR2));
#endif

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
          "App",                vb::Constructor<nu::App>(),
          "Appearance",         vb::Constructor<nu::Appearance>(),
          "AttributedText",     vb::Constructor<nu::AttributedText>(),
          "Font",               vb::Constructor<nu::Font>(),
          "Canvas",             vb::Constructor<nu::Canvas>(),
          "Clipboard",          vb::Constructor<nu::Clipboard>(),
          "Color",              vb::Constructor<nu::Color>(),
          "Cursor",             vb::Constructor<nu::Cursor>(),
          "DraggingInfo",       vb::Constructor<nu::DraggingInfo>(),
          "Image",              vb::Constructor<nu::Image>(),
          "Painter",            vb::Constructor<nu::Painter>(),
          "Event",              vb::Constructor<nu::Event>(),
          "FileDialog",         vb::Constructor<nu::FileDialog>(),
          "FileOpenDialog",     vb::Constructor<nu::FileOpenDialog>(),
          "FileSaveDialog",     vb::Constructor<nu::FileSaveDialog>(),
          "MenuBar",            vb::Constructor<nu::MenuBar>(),
          "Menu",               vb::Constructor<nu::Menu>(),
          "MenuItem",           vb::Constructor<nu::MenuItem>(),
          "MessageBox",         vb::Constructor<nu::MessageBox>(),
          "Notification",       vb::Constructor<nu::Notification>(),
          "NotificationCenter", vb::Constructor<nu::NotificationCenter>(),
          "Responder",          vb::Constructor<nu::Responder>(),
          "Window",             vb::Constructor<nu::Window>(),
          "View",               vb::Constructor<nu::View>(),
          "ComboBox",           vb::Constructor<nu::ComboBox>(),
          "Container",          vb::Constructor<nu::Container>(),
          "Button",             vb::Constructor<nu::Button>(),
          "ProtocolStringJob",  vb::Constructor<nu::ProtocolStringJob>(),
          "ProtocolFileJob",    vb::Constructor<nu::ProtocolFileJob>(),
          "ProtocolAsarJob",    vb::Constructor<nu::ProtocolAsarJob>(),
          "Browser",            vb::Constructor<nu::Browser>(),
          "DatePicker",         vb::Constructor<nu::DatePicker>(),
          "Entry",              vb::Constructor<nu::Entry>(),
          "Label",              vb::Constructor<nu::Label>(),
          "Picker",             vb::Constructor<nu::Picker>(),
          "ProgressBar",        vb::Constructor<nu::ProgressBar>(),
          "GifPlayer",          vb::Constructor<nu::GifPlayer>(),
          "Group",              vb::Constructor<nu::Group>(),
          "Screen",             vb::Constructor<nu::Screen>(),
          "Scroll",             vb::Constructor<nu::Scroll>(),
          "Separator",          vb::Constructor<nu::Separator>(),
          "Slider",             vb::Constructor<nu::Slider>(),
          "TableModel",         vb::Constructor<nu::TableModel>(),
          "AbstractTableModel", vb::Constructor<nu::AbstractTableModel>(),
          "SimpleTableModel",   vb::Constructor<nu::SimpleTableModel>(),
          "Tab",                vb::Constructor<nu::Tab>(),
          "Table",              vb::Constructor<nu::Table>(),
          "TextEdit",           vb::Constructor<nu::TextEdit>(),
          "Tray",               vb::Constructor<nu::Tray>(),
#if defined(OS_MAC)
          "Toolbar",            vb::Constructor<nu::Toolbar>(),
          "Vibrant",            vb::Constructor<nu::Vibrant>(),
#endif
          // Properties.
          "app",                nu::App::GetCurrent(),
          "appearance",         nu::Appearance::GetCurrent(),
          "notificationCenter", nu::NotificationCenter::GetCurrent(),
          "screen",             nu::Screen::GetCurrent(),
          // Functions.
          "memoryPressureNotification", &MemoryPressureNotification);
  if (is_electron) {
#if defined(OS_MAC)
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

NODE_MODULE_CONTEXT_AWARE(gui, node_yue::Initialize);
