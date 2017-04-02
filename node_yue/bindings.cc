// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include <node.h>

#include "nativeui/nativeui.h"
#include "node_yue/node_integration.h"
#include "node_yue/signal.h"

namespace vb {

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
    int width, height;
    if (!Get(context, value.As<v8::Object>(),
             "width", &width, "height", &height))
      return false;
    *out = nu::Size(width, height);
    return true;
  }
};

template<>
struct Type<nu::SizeF> {
  static constexpr const char* name = "yue.Size";
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
    float width, height;
    if (!Get(context, value.As<v8::Object>(),
             "width", &width, "height", &height))
      return false;
    *out = nu::SizeF(width, height);
    return true;
  }
};

template<>
struct Type<nu::RectF> {
  static constexpr const char* name = "yue.Rect";
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
    float x, y, width, height;
    if (!Get(context, value.As<v8::Object>(),
             "x", &x, "y", &y, "width", &width, "height", &height))
      return false;
    *out = nu::RectF(x, y, width, height);
    return true;
  }
};

template<>
struct Type<nu::Vector2dF> {
  static constexpr const char* name = "yue.Vector2d";
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
    float x, y;
    if (!Get(context, value.As<v8::Object>(), "x", &x, "y", &y))
      return false;
    *out = nu::Vector2dF(x, y);
    return true;
  }
};

template<>
struct Type<nu::PointF> {
  static constexpr const char* name = "yue.Point";
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
    float x, y;
    if (!Get(context, value.As<v8::Object>(), "x", &x, "y", &y))
      return false;
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
struct Type<nu::App> {
  static constexpr const char* name = "yue.App";
  static void BuildConstructor(v8::Local<v8::Context>, v8::Local<v8::Object>) {
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
#if defined(OS_MACOSX)
    Set(context, templ,
        "setApplicationMenu", &nu::App::SetApplicationMenu);
#endif
  }
};

template<>
struct Type<nu::Font> {
  static constexpr const char* name = "yue.Font";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor,
        "create", &CreateInstance<nu::Font, const std::string&, int>,
        "default", &GetDefault);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "getName", &nu::Font::GetName,
        "getSize", &nu::Font::GetSize);
  }
  static nu::Font* GetDefault() {
    return nu::State::GetCurrent()->GetDefaultFont();
  }
};

template<>
struct Type<nu::SystemColor> {
  static constexpr const char* name = "yue.SystemColor";
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::SystemColor* out) {
    std::string id;
    if (!vb::FromV8(context, value, &id))
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
        "getSystem", &nu::GetSystemColor,
        "rgb", NativeConstructor<nu::Color,
                                 unsigned, unsigned, unsigned>(),
        "argb", NativeConstructor<nu::Color,
                                  unsigned, unsigned, unsigned, unsigned>());
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
        "createFromFile", &CreateInstance<nu::Image, const nu::String&>);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "getSize", &nu::Image::GetSize);
  }
};

template<>
struct Type<nu::Painter> {
  static constexpr const char* name = "yue.Painter";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor,
        "TextAlignLeft",   static_cast<int>(nu::Painter::TextAlignLeft),
        "TextAlignCenter", static_cast<int>(nu::Painter::TextAlignCenter),
        "TextAlignRight",  static_cast<int>(nu::Painter::TextAlignRight));
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
        "clip", &nu::Painter::Clip,
        "clipRect", &nu::Painter::ClipRect,
        "translate", &nu::Painter::Translate,
        "setColor", &nu::Painter::SetColor,
        "setLineWidth", &nu::Painter::SetLineWidth,
        "stroke", &nu::Painter::Stroke,
        "fill", &nu::Painter::Fill,
        "strokeRect", &nu::Painter::StrokeRect,
        "fillRect", &nu::Painter::FillRect,
        "drawText", &nu::Painter::DrawText,
        "drawTextWithFlags", &nu::Painter::DrawTextWithFlags,
        "drawColoredText", &nu::Painter::DrawColoredText,
        "drawColoredTextWithFlags", &nu::Painter::DrawColoredTextWithFlags);
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

template<>
struct Type<nu::MenuBar> {
  using base = nu::MenuBase;
  static constexpr const char* name = "yue.MenuBar";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor,
        "create", &CreateInstance<nu::MenuBar>);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
  }
};

template<>
struct Type<nu::Menu> {
  using base = nu::MenuBase;
  static constexpr const char* name = "yue.Menu";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor,
        "create", &CreateInstance<nu::Menu>);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "popup", &nu::Menu::Popup);
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
      *out = nu::MenuItem::Label;
    else if (type == "checkbox")
      *out = nu::MenuItem::CheckBox;
    else if (type == "radio")
      *out = nu::MenuItem::Radio;
    else if (type == "separator")
      *out = nu::MenuItem::Separator;
    else if (type == "submenu")
      *out = nu::MenuItem::Submenu;
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
    Set(context, constructor,
        "create", &CreateInstance<nu::MenuItem, nu::MenuItem::Type>);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
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
                "onclick", &nu::MenuItem::on_click);
  }
};

template<>
struct Type<nu::Window::Options> {
  static constexpr const char* name = "yue.Window.Options";
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::Window::Options* out) {
    auto obj = value.As<v8::Object>();
    if (obj.IsEmpty())
      return false;
    Get(context, obj, "bounds", &out->bounds);
    return true;
  }
};

template<>
struct Type<nu::Window> {
  static constexpr const char* name = "yue.Window";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor,
        "create", &CreateInstance<nu::Window, nu::Window::Options>);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "close", &nu::Window::Close,
        "setContentBounds", &nu::Window::SetContentBounds,
        "getContentBounds", &nu::Window::GetContentBounds,
        "setBounds", &nu::Window::SetBounds,
        "getBounds", &nu::Window::GetBounds,
        "setContentView", &nu::Window::SetContentView,
        "getContentView", &nu::Window::GetContentView,
        "setVisible", &nu::Window::SetVisible,
        "isVisible", &nu::Window::IsVisible,
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
        "setBackgroundColor", &nu::View::SetBackgroundColor,
        "setStyle", &nu::View::SetStyle,
        "printStyle", &nu::View::PrintStyle,
        "layout", &nu::View::Layout,
        "setBounds", &nu::View::SetBounds,
        "getBounds", &nu::View::GetBounds,
        "setVisible", &nu::View::SetVisible,
        "isVisible", &nu::View::IsVisible,
        "getParent", &nu::View::GetParent);
  }
};

template<>
struct Type<nu::Container> {
  using base = nu::View;
  static constexpr const char* name = "yue.Container";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor,
        "create", &CreateInstance<nu::Container>);
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
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor,
        "create", &CreateInstance<nu::Button,
                                  const std::string&, nu::Button::Type>);
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
};

template<>
struct Type<nu::Entry> {
  using base = nu::View;
  static constexpr const char* name = "yue.Entry";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor, "create", &CreateInstance<nu::Entry>);
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
        "create", &CreateInstance<nu::Label, const std::string&>);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "setText", &nu::Label::SetText,
        "getText", &nu::Label::GetText);
  }
};

template<>
struct Type<nu::Progress> {
  using base = nu::View;
  static constexpr const char* name = "yue.Progress";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor, "create", &CreateInstance<nu::Progress>);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "setValue", &nu::Progress::SetValue,
        "getValue", &nu::Progress::GetValue,
        "setIndeterminate", &nu::Progress::SetIndeterminate,
        "isIndeterminate", &nu::Progress::IsIndeterminate);
  }
};

template<>
struct Type<nu::Group> {
  using base = nu::View;
  static constexpr const char* name = "yue.Group";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor,
        "create", &CreateInstance<nu::Group, const std::string&>);
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
    Set(context, constructor, "create", &CreateInstance<nu::Scroll>);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "setScrollBarPolicy", &nu::Scroll::SetScrollBarPolicy,
        "getScrollBarPolicy", &nu::Scroll::GetScrollBarPolicy,
        "setContentSize", &nu::Scroll::SetContentSize,
        "getContentSize", &nu::Scroll::GetContentSize,
        "setContentView", &nu::Scroll::SetContentView,
        "getContentView", &nu::Scroll::GetContentView);
  }
};

#if defined(OS_MACOSX)
template<>
struct Type<nu::Vibrant> {
  using base = nu::Container;
  static constexpr const char* name = "yue.Vibrant";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor, "create", &CreateInstance<nu::Vibrant>);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
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
  new nu::Lifetime;
  // Initialize node integration and leak it.
  NodeIntegration* node_integration = NodeIntegration::Create();
  node_integration->PrepareMessageLoop();
  node_integration->RunMessageLoop();

  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  vb::Set(context, exports,
          // Classes.
#ifndef ELECTRON_BUILD
          "Lifetime",  vb::Constructor<nu::Lifetime>(),
#endif
          "App",       vb::Constructor<nu::App>(),
          "Font",      vb::Constructor<nu::Font>(),
          "Color",     vb::Constructor<nu::Color>(),
          "Image",     vb::Constructor<nu::Image>(),
          "Painter",   vb::Constructor<nu::Painter>(),
          "MenuBar",   vb::Constructor<nu::MenuBar>(),
          "Menu",      vb::Constructor<nu::Menu>(),
          "MenuItem",  vb::Constructor<nu::MenuItem>(),
          "Window",    vb::Constructor<nu::Window>(),
          "View",      vb::Constructor<nu::View>(),
          "Container", vb::Constructor<nu::Container>(),
          "Button",    vb::Constructor<nu::Button>(),
          "Entry",     vb::Constructor<nu::Entry>(),
          "Label",     vb::Constructor<nu::Label>(),
          "Progress",  vb::Constructor<nu::Progress>(),
          "Group",     vb::Constructor<nu::Group>(),
          "Scroll",    vb::Constructor<nu::Scroll>(),
#if defined(OS_MACOSX)
          "Vibrant",   vb::Constructor<nu::Vibrant>(),
#endif
          // Properties.
#ifndef ELECTRON_BUILD
          "lifetime", nu::Lifetime::GetCurrent(),
#endif
          "app",      nu::State::GetCurrent()->GetApp());
}

}  // namespace node_yue

NODE_MODULE(yue, node_yue::Initialize)
