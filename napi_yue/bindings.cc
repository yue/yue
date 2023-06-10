// Copyright 2023 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "base/environment.h"
#include "base/notreached.h"
#include "base/time/time.h"
#include "napi_yue/binding_ptr.h"
#include "napi_yue/binding_signal.h"
#include "napi_yue/binding_value.h"
#include "napi_yue/node_integration.h"

#if defined(OS_LINUX) || defined(OS_MAC)
#include "base/strings/string_number_conversions.h"
#elif defined(OS_WIN)
#include "base/strings/string_util_win.h"
#endif

namespace {

bool is_electron = false;
bool is_yode = false;

template<typename T, typename... ArgTypes>
T* CreateOnHeap(ArgTypes&&... args) {
  return new T(std::forward<ArgTypes>(args)...);
}

template<typename T, typename... ArgTypes>
T CreateOnStack(ArgTypes&&... args) {
  return T(std::forward<ArgTypes>(args)...);
}

}  // namespace

namespace ki {

using napi_yue::Signal;
using napi_yue::Delegate;

template<typename T>
struct Type<absl::optional<T>> {
  static constexpr const char* name = Type<T>::name;
  static napi_status ToNode(napi_env env,
                            const absl::optional<T>& value,
                            napi_value* result) {
    if (value)
      return ConvertToNode(env, *value, result);
    else
      return ConvertToNode(env, nullptr, result);
  }
  static napi_status FromNode(napi_env env,
                              napi_value value,
                              absl::optional<T>* out) {
    napi_valuetype type;
    napi_status s = napi_typeof(env, value, &type);
    if (s == napi_ok) {
      if (type == napi_undefined || type == napi_null) {
        out->reset();
        return napi_ok;
      }
      T copy;
      s = ConvertFromNode(env, value, &copy);
      if (s == napi_ok)
        out->emplace(std::move(copy));
    }
    return s;
  }
};

#if defined(OS_WIN)
template<>
struct Type<std::wstring> {
  static constexpr const char* name = "String";
  static inline napi_status ToNode(napi_env env,
                                   const std::wstring& value,
                                   napi_value* result) {
    return ConvertToNode(env, base::as_u16cstr(value), result);
  }
  static napi_status FromNode(napi_env env,
                              napi_value value,
                              std::wstring* out) {
    std::u16string str;
    napi_status s = ConvertFromNode(env, value, &str);
    if (s == napi_ok)
      *out = base::AsWString(str);
    return s;
  }
};
#endif

template<>
struct Type<nu::Accelerator> {
  static constexpr const char* name = "Accelerator";
  static napi_status FromNode(napi_env env,
                              napi_value value,
                              nu::Accelerator* out) {
    std::string description;
    napi_status s = ConvertFromNode(env, value, &description);
    if (s == napi_ok) {
      *out = nu::Accelerator(description);
      if (out->IsEmpty())
        return napi_invalid_arg;
    }
    return s;
  }
};

template<>
struct Type<nu::Buffer> {
  static constexpr const char* name = "Buffer";
  static napi_status ToNode(napi_env env,
                            const nu::Buffer& value,
                            napi_value* result) {
    return napi_create_buffer_copy(
        env, value.size(), value.content(), nullptr, result);
  }
  static napi_status FromNode(napi_env env, napi_value value, nu::Buffer* out) {
    void* data;
    size_t length;
    napi_status s = napi_get_arraybuffer_info(env, value, &data, &length);
    // We are assuming the Buffer is consumed immediately.
    if (s == napi_ok)
      *out = nu::Buffer::Wrap(data, length);
    return s;
  }
};

template<>
struct Type<base::FilePath> {
  static constexpr const char* name = "FilePath";
  static napi_status ToNode(napi_env env,
                            const base::FilePath& value,
                            napi_value* result) {
    return ConvertToNode(env, value.value(), result);
  }
  static napi_status FromNode(napi_env env,
                              napi_value value,
                              base::FilePath* out) {
    base::FilePath::StringType str;
    napi_status s = ConvertFromNode(env, value, &str);
    if (s != napi_ok)
      return s;
    *out = base::FilePath(str);
    return napi_ok;
  }
};

template<>
struct Type<base::Time> {
  static constexpr const char* name = "Time";
  static napi_status ToNode(napi_env env,
                            const base::Time& value,
                            napi_value* result) {
    return napi_create_date(env, value.ToJsTimeIgnoringNull(), result);
  }
  static napi_status FromNode(napi_env env, napi_value value, base::Time* out) {
    double date;
    napi_status s = napi_get_date_value(env, value, &date);
    if (s == napi_ok)
      *out = base::Time::FromJsTime(date);
    return s;
  }
};

template<>
struct Type<nu::Size> {
  static constexpr const char* name = "Size";
  static napi_status ToNode(napi_env env,
                            const nu::Size& value,
                            napi_value* result) {
    *result = CreateObject(env);
    Set(env, *result, "width", value.width(), "height", value.height());
    return napi_ok;
  }
  static napi_status FromNode(napi_env env, napi_value value, nu::Size* out) {
    int width = 0, height = 0;
    if (!ReadOptions(env, value, "width", &width, "height", &height))
      return napi_invalid_arg;
    *out = nu::Size(width, height);
    return napi_ok;
  }
};

template<>
struct Type<nu::SizeF> {
  static constexpr const char* name = "SizeF";
  static napi_status ToNode(napi_env env,
                            const nu::SizeF& value,
                            napi_value* result) {
    *result = CreateObject(env);
    Set(env, *result, "width", value.width(), "height", value.height());
    return napi_ok;
  }
  static napi_status FromNode(napi_env env, napi_value value, nu::SizeF* out) {
    float width = 0, height = 0;
    if (!ReadOptions(env, value, "width", &width, "height", &height))
      return napi_invalid_arg;
    *out = nu::SizeF(width, height);
    return napi_ok;
  }
};

template<>
struct Type<nu::RectF> {
  static constexpr const char* name = "RectF";
  static napi_status ToNode(napi_env env,
                            const nu::RectF& value,
                            napi_value* result) {
    *result = CreateObject(env);
    Set(env, *result,
        "x", value.x(), "y", value.y(),
        "width", value.width(), "height", value.height());
    return napi_ok;
  }
  static napi_status FromNode(napi_env env, napi_value value, nu::RectF* out) {
    float x = 0, y = 0, width = 0, height = 0;
    if (!ReadOptions(env, value,
                     "x", &x, "y", &y,
                     "width", &width, "height", &height))
      return napi_invalid_arg;
    *out = nu::RectF(x, y, width, height);
    return napi_ok;
  }
};

template<>
struct Type<nu::Vector2dF> {
  static constexpr const char* name = "Vector2dF";
  static napi_status ToNode(napi_env env,
                            const nu::Vector2dF& value,
                            napi_value* result) {
    *result = CreateObject(env);
    Set(env, *result, "x", value.x(), "y", value.y());
    return napi_ok;
  }
  static napi_status FromNode(napi_env env,
                     napi_value value,
                     nu::Vector2dF* out) {
    float x = 0, y = 0;
    if (!ReadOptions(env, value, "x", &x, "y", &y))
      return napi_invalid_arg;
    *out = nu::Vector2dF(x, y);
    return napi_ok;
  }
};

template<>
struct Type<nu::PointF> {
  static constexpr const char* name = "PointF";
  static napi_status ToNode(napi_env env,
                            const nu::PointF& value,
                            napi_value* result) {
    *result = CreateObject(env);
    Set(env, *result, "x", value.x(), "y", value.y());
    return napi_ok;
  }
  static napi_status FromNode(napi_env env, napi_value value, nu::PointF* out) {
    float x = 0, y = 0;
    if (!ReadOptions(env, value, "x", &x, "y", &y))
      return napi_invalid_arg;
    *out = nu::PointF(x, y);
    return napi_ok;
  }
};

template<>
struct Type<nu::BlendMode> {
  static constexpr const char* name = "BlendMode";
  static napi_status FromNode(napi_env env,
                              napi_value value,
                              nu::BlendMode* out) {
    std::string mode;
    napi_status s = ConvertFromNode(env, value, &mode);
    if (s == napi_ok) {
      if (mode == "normal")
        *out = nu::BlendMode::Normal;
      else if (mode == "multiply")
        *out = nu::BlendMode::Multiply;
      else if (mode == "screen")
        *out = nu::BlendMode::Screen;
      else if (mode == "overlay")
        *out = nu::BlendMode::Overlay;
      else if (mode == "darken")
        *out = nu::BlendMode::Darken;
      else if (mode == "lighten")
        *out = nu::BlendMode::Lighten;
      else if (mode == "color-dodge")
        *out = nu::BlendMode::ColorDodge;
      else if (mode == "color-burn")
        *out = nu::BlendMode::ColorBurn;
      else if (mode == "soft-light")
        *out = nu::BlendMode::SoftLight;
      else if (mode == "hard-light")
        *out = nu::BlendMode::HardLight;
      else if (mode == "difference")
        *out = nu::BlendMode::Difference;
      else if (mode == "exclusion")
        *out = nu::BlendMode::Exclusion;
      else if (mode == "hue")
        *out = nu::BlendMode::Hue;
      else if (mode == "saturation")
        *out = nu::BlendMode::Saturation;
      else if (mode == "color")
        *out = nu::BlendMode::Color;
      else if (mode == "luminosity")
        *out = nu::BlendMode::Luminosity;
      else if (mode == "clear")
        *out = nu::BlendMode::Clear;
      else if (mode == "copy")
        *out = nu::BlendMode::Copy;
      else if (mode == "source-in")
        *out = nu::BlendMode::SourceIn;
      else if (mode == "source-out")
        *out = nu::BlendMode::SourceOut;
      else if (mode == "source-atop")
        *out = nu::BlendMode::SourceAtop;
      else if (mode == "destination-over")
        *out = nu::BlendMode::DestinationOver;
      else if (mode == "destination-in")
        *out = nu::BlendMode::DestinationIn;
      else if (mode == "destination-atop")
        *out = nu::BlendMode::DestinationAtop;
      else if (mode == "xor")
        *out = nu::BlendMode::Xor;
      else
        return napi_invalid_arg;
    }
    return s;
  }
};

#if defined(OS_MAC)
template<>
struct Type<nu::ControlSize> {
  static constexpr const char* name = "ControlSize";
  static napi_status FromNode(napi_env env,
                              napi_value value,
                              nu::ControlSize* out) {
    std::string size;
    napi_status s = ConvertFromNode(env, value, &size);
    if (s == napi_ok) {
      if (size == "mini")
        *out = nu::ControlSize::Mini;
      else if (size == "small")
        *out = nu::ControlSize::Small;
      else if (size == "regular")
        *out = nu::ControlSize::Regular;
      else if (size == "large")
        *out = nu::ControlSize::Large;
      else
        return napi_invalid_arg;
    }
    return s;
  }
};
#endif

template<>
struct Type<nu::Cookie> {
  static constexpr const char* name = "Cookie";
  static napi_status ToNode(napi_env env,
                            const nu::Cookie& value,
                            napi_value* result) {
    *result = CreateObject(env);
    Set(env, *result,
        "name", value.name,
        "value", value.value,
        "domain", value.domain,
        "path", value.path,
        "httpOnly", value.http_only,
        "secure", value.secure);
    return napi_ok;
  }
  static napi_status FromNode(napi_env env, napi_value value, nu::Cookie* out) {
    if (!ReadOptions(env, value,
                     "name", &out->name,
                     "value", &out->value,
                     "domain", &out->domain,
                     "path", &out->path,
                     "httpOnly", &out->http_only,
                     "secure", &out->secure))
      return napi_invalid_arg;
    return napi_ok;
  }
};

template<>
struct Type<nu::Display> {
  static constexpr const char* name = "Display";
  static napi_status ToNode(napi_env env,
                            const nu::Display& display,
                            napi_value* result) {
    *result = CreateObject(env);
    Set(env, *result,
        "id", display.id,
        "scaleFactor", display.scale_factor,
#if defined(OS_MAC)
        "internal", display.internal,
#endif
        "bounds", display.bounds,
        "workArea", display.work_area);
    return napi_ok;
  }
};

template<>
struct Type<nu::ImageScale> {
  static constexpr const char* name = "ImageScale";
  static napi_status ToNode(napi_env env,
                            nu::ImageScale scale,
                            napi_value* result) {
    switch (scale) {
      case nu::ImageScale::None:
        return ConvertToNode(env, "none", result);
      case nu::ImageScale::Fill:
        return ConvertToNode(env, "fill", result);
      case nu::ImageScale::Down:
        return ConvertToNode(env, "down", result);
      case nu::ImageScale::UpOrDown:
        return ConvertToNode(env, "up-or-down", result);
    }
    NOTREACHED();
    return napi_generic_failure;
  }
  static napi_status FromNode(napi_env env,
                              napi_value value,
                              nu::ImageScale* out) {
    std::string scale;
    napi_status s = ConvertFromNode(env, value, &scale);
    if (s == napi_ok) {
      if (scale == "none")
        *out = nu::ImageScale::None;
      else if (scale == "fill")
        *out = nu::ImageScale::Fill;
      else if (scale == "down")
        *out = nu::ImageScale::Down;
      else if (scale == "up-or-down")
        *out = nu::ImageScale::UpOrDown;
      else
        return napi_invalid_arg;
    }
    return s;
  }
};

template<>
struct Type<nu::KeyboardCode> {
  static constexpr const char* name = "KeyboardCode";
  static napi_status ToNode(napi_env env,
                            const nu::KeyboardCode& code,
                            napi_value* result) {
    return ConvertToNode(env, nu::KeyboardCodeToStr(code), result);
  }
};

template<>
struct Type<nu::Orientation> {
  static constexpr const char* name = "Orientation";
  static napi_status ToNode(napi_env env,
                            nu::Orientation orientation,
                            napi_value* result) {
    switch (orientation) {
      case nu::Orientation::Horizontal:
        return ConvertToNode(env, "horizontal", result);
      case nu::Orientation::Vertical:
        return ConvertToNode(env, "vertical", result);
    }
    NOTREACHED();
    return napi_generic_failure;
  }
  static napi_status FromNode(napi_env env,
                              napi_value value,
                              nu::Orientation* out) {
    std::string orientation;
    napi_status s = ConvertFromNode(env, value, &orientation);
    if (s == napi_ok) {
      if (orientation == "horizontal")
        *out = nu::Orientation::Horizontal;
      else if (orientation == "vertical")
        *out = nu::Orientation::Vertical;
      else
        return napi_invalid_arg;
    }
    return s;
  }
};

template<>
struct Type<nu::TextAlign> {
  static constexpr const char* name = "TextAlign";
  static napi_status ToNode(napi_env env,
                            nu::TextAlign align,
                            napi_value* result) {
    switch (align) {
      case nu::TextAlign::Start:
        return ConvertToNode(env, "start", result);
      case nu::TextAlign::Center:
        return ConvertToNode(env, "center", result);
      case nu::TextAlign::End:
        return ConvertToNode(env, "end", result);
    }
    NOTREACHED();
    return napi_generic_failure;
  }
  static napi_status FromNode(napi_env env,
                              napi_value value,
                              nu::TextAlign* out) {
    std::string align;
    napi_status s = ConvertFromNode(env, value, &align);
    if (s == napi_ok) {
      if (align == "start")
        *out = nu::TextAlign::Start;
      else if (align == "center")
        *out = nu::TextAlign::Center;
      else if (align == "end")
        *out = nu::TextAlign::End;
      else
        return napi_invalid_arg;
    }
    return s;
  }
};

template<>
struct Type<nu::TextFormat> {
  static constexpr const char* name = "TextFormat";
  static napi_status ToNode(napi_env env,
                            const nu::TextFormat& value,
                            napi_value* result) {
    *result = CreateObject(env);
    Set(env, *result,
        "align", value.align,
        "valign", value.valign,
        "wrap", value.wrap,
        "ellipsis", value.ellipsis);
    return napi_ok;
  }
  static napi_status FromNode(napi_env env,
                              napi_value value,
                              nu::TextFormat* out) {
    if (!ReadOptions(env, value,
                     "align", &out->align,
                     "valign", &out->valign,
                     "wrap", &out->wrap,
                     "ellipsis", &out->ellipsis))
      return napi_invalid_arg;
    return napi_ok;
  }
};

template<>
struct Type<nu::TextAttributes> {
  static constexpr const char* name = "TextAttributes";
  static napi_status FromNode(napi_env env,
                              napi_value value,
                              nu::TextAttributes* out) {
    napi_status s = Type<nu::TextFormat>::FromNode(env, value, out);
    if (s != napi_ok)
      return s;
    if (!ReadOptions(env, value,
                     "font", &out->font,
                     "color", &out->color))
      return napi_invalid_arg;
    return napi_ok;
  }
};

#if defined(OS_MAC)
template<>
struct Type<nu::App::ActivationPolicy> {
  static constexpr const char* name = "AppActivationPolicy";
  static napi_status ToNode(napi_env env,
                            nu::App::ActivationPolicy policy,
                            napi_value* result) {
    switch (policy) {
      case nu::App::ActivationPolicy::Regular:
        return ConvertToNode(env, "regular", result);
      case nu::App::ActivationPolicy::Accessory:
        return ConvertToNode(env, "accessory", result);
      case nu::App::ActivationPolicy::Prohibited:
        return ConvertToNode(env, "prohibited", result);
    }
    NOTREACHED();
    return napi_generic_failure;
  }
  static napi_status FromNode(napi_env env,
                              napi_value value,
                              nu::App::ActivationPolicy* out) {
    std::string policy;
    napi_status s = ConvertFromNode(env, value, &policy);
    if (s == napi_ok) {
      if (policy == "regular")
        *out = nu::App::ActivationPolicy::Regular;
      else if (policy == "accessory")
        *out = nu::App::ActivationPolicy::Accessory;
      else if (policy == "prohibited")
        *out = nu::App::ActivationPolicy::Prohibited;
      else
        return napi_invalid_arg;
    }
    return s;
  }
};
#endif

#if defined(OS_WIN)
template<>
struct Type<nu::App::ShortcutOptions> {
  static constexpr const char* name = "AppShortcutOptions";
  static napi_status FromNode(napi_env env,
                              napi_value value,
                              nu::App::ShortcutOptions* out) {
    if (!ReadOptions(env, value,
                     "arguments", &out->arguments,
                     "description", &out->description,
                     "workingDir", &out->working_dir))
      return napi_invalid_arg;
    return napi_ok;
  }
};
#endif

template<>
struct Type<nu::App> {
  static constexpr const char* name = "App";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, prototype,
        "setName", &nu::App::SetName,
        "getName", &nu::App::GetName,
#if defined(OS_LINUX) || defined(OS_WIN)
        "setID", &nu::App::SetID,
#endif
        "getID", &nu::App::GetID);
#if defined(OS_MAC)
    Set(env, prototype,
        "setApplicationMenu",
        WrapMethod(&nu::App::SetApplicationMenu, [](Arguments args) {
          AttachedTable(args).Set("appMenu", args[0]);
        }),
        "setDockBadgeLabel", &nu::App::SetDockBadgeLabel,
        "getDockBadgeLabel", &nu::App::GetDockBadgeLabel,
        "activate", &nu::App::Activate,
        "deactivate", &nu::App::Deactivate,
        "isActive", &nu::App::IsActive,
        "setActivationPolicy", &nu::App::SetActivationPolicy,
        "getActivationPolicy", &nu::App::GetActivationPolicy);
#elif defined(OS_WIN)
    Set(env, prototype,
        "createStartMenuShortcut", &nu::App::CreateStartMenuShortcut,
        "getStartMenuShortcutPath", &nu::App::GetStartMenuShortcutPath);
#endif
  }
};

template<>
struct Type<nu::Appearance> {
  static constexpr const char* name = "Appearance";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, prototype,
#if defined(OS_WIN)
        "setDarkModeEnabled", &nu::Appearance::SetDarkModeEnabled,
#endif
        "isDarkScheme", &nu::Appearance::IsDarkScheme);
    DefineProperties(
        env, prototype,
        Signal("onColorSchemeChange", &nu::Appearance::on_color_scheme_change));
  }
};

template<>
struct Type<nu::AttributedText> {
  static constexpr const char* name = "AttributedText";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, constructor,
        "create", &CreateOnHeap<nu::AttributedText,
                                const std::string&,
                                nu::TextAttributes>);
    Set(env, prototype,
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
struct Type<nu::Browser::Options> {
  static constexpr const char* name = "BrowserOptions";
  static napi_status FromNode(napi_env env,
                              napi_value value,
                              nu::Browser::Options* out) {
    if (!ReadOptions(
            env, value,
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
           "contextMenu", &out->context_menu))
      return napi_invalid_arg;
    return napi_ok;
  }
};

template<>
struct Type<nu::Browser> {
  using Base = nu::View;
  static constexpr const char* name = "Browser";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, constructor,
        "create", &CreateOnHeap<nu::Browser, nu::Browser::Options>,
        "registerProtocol", &nu::Browser::RegisterProtocol,
        "unregisterProtocol", &nu::Browser::UnregisterProtocol);
    Set(env, prototype,
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
        "getCookiesForURL", &nu::Browser::GetCookiesForURL,
        "goBack", &nu::Browser::GoBack,
        "canGoBack", &nu::Browser::CanGoBack,
        "goForward", &nu::Browser::GoForward,
        "canGoForward", &nu::Browser::CanGoForward,
        "reload", &nu::Browser::Reload,
        "stop", &nu::Browser::Stop,
        "isLoading", &nu::Browser::IsLoading,
        "setBindingName", &nu::Browser::SetBindingName,
        "addBinding", &AddBinding,
        "addRawBinding",
        WrapMethod(&nu::Browser::AddRawBinding, [](Arguments args) {
          AttachedTable(args).GetOrCreateMap("bindings").Set(args[0], args[1]);
        }),
        "removeBinding",
        WrapMethod(&nu::Browser::RemoveBinding, [](Arguments args) {
          AttachedTable(args).GetOrCreateMap("bindings").Delete(args[0]);
        }),
        "beginAddingBindings", &nu::Browser::BeginAddingBindings,
        "endAddingBindings", &nu::Browser::EndAddingBindings);
    DefineProperties(
        env, prototype,
        Signal("onClose", &nu::Browser::on_close),
        Signal("onUpdateCommand", &nu::Browser::on_update_command),
        Signal("onChangeLoading", &nu::Browser::on_change_loading),
        Signal("onUpdateTitle", &nu::Browser::on_update_title),
        Signal("onStartNavigation", &nu::Browser::on_start_navigation),
        Signal("onCommitNavigation", &nu::Browser::on_commit_navigation),
        Signal("onFinishNavigation", &nu::Browser::on_finish_navigation),
        Signal("onFailNavigation", &nu::Browser::on_fail_navigation));
  }
  static void AddBinding(Arguments args,
                         const std::string& bname,
                         napi_value func) {
    nu::Browser* browser;
    if (!args.GetThis(&browser))
      return;
    // this[bindings][bname] = func.
    AttachedTable(args).GetOrCreateMap("bindings").Set(args[0], args[1]);
    // The func must be stored as weak reference.
    Persistent ref(args.Env(), func, 0);
    // Parse base::Value and call func with v8 args.
    browser->AddRawBinding(bname, [ref = std::move(ref)](nu::Browser* browser,
                                                         base::Value value) {
      HandleScope handle_scope(ref.Env());
      napi_value func = ref.Value();
      if (!func)
        return;
      std::vector<napi_value> args;
      args.reserve(value.GetList().size());
      for (const auto& it : value.GetList())
        args.push_back(ToNode(ref.Env(), it));
      napi_status s = napi_make_callback(
          ref.Env(), nullptr, func, func, args.size(),
          args.empty() ? nullptr : &args.front(), nullptr);
      if (s == napi_pending_exception) {
        napi_value fatal_exception;
        napi_get_and_clear_last_exception(ref.Env(), &fatal_exception);
        napi_fatal_exception(ref.Env(), fatal_exception);
      }
    });
  }
};

#if defined(OS_MAC)
template<>
struct Type<nu::Button::Style> {
  static constexpr const char* name = "ButtonStyle";
  static napi_status FromNode(napi_env env,
                              napi_value value,
                              nu::Button::Style* out) {
    std::string style;
    napi_status s = ConvertFromNode(env, value, &style);
    if (s == napi_ok) {
      if (style == "rounded")
        *out = nu::Button::Style::Rounded;
      else if (style == "regular-square")
        *out = nu::Button::Style::RegularSquare;
      else if (style == "thick-square")
        *out = nu::Button::Style::ThickSquare;
      else if (style == "thicker-square")
        *out = nu::Button::Style::ThickerSquare;
      else if (style == "disclosure")
        *out = nu::Button::Style::Disclosure;
      else if (style == "shadowless-square")
        *out = nu::Button::Style::ShadowlessSquare;
      else if (style == "circular")
        *out = nu::Button::Style::Circular;
      else if (style == "textured-square")
        *out = nu::Button::Style::TexturedSquare;
      else if (style == "help-button")
        *out = nu::Button::Style::HelpButton;
      else if (style == "small-square")
        *out = nu::Button::Style::SmallSquare;
      else if (style == "textured-rounded")
        *out = nu::Button::Style::TexturedRounded;
      else if (style == "round-rect")
        *out = nu::Button::Style::RoundRect;
      else if (style == "recessed")
        *out = nu::Button::Style::Recessed;
      else if (style == "rounded-disclosure")
        *out = nu::Button::Style::RoundedDisclosure;
      else if (style == "inline")
        *out = nu::Button::Style::Inline;
      else
        return napi_invalid_arg;
    }
    return s;
  }
};
#endif

template<>
struct Type<nu::Button::Type> {
  static constexpr const char* name = "ButtonType";
  static napi_status FromNode(napi_env env,
                              napi_value value,
                              nu::Button::Type* out) {
    std::string type;
    napi_status s = ConvertFromNode(env, value, &type);
    if (s == napi_ok) {
      if (type == "normal")
        *out = nu::Button::Type::Normal;
      else if (type == "checkbox")
        *out = nu::Button::Type::Checkbox;
      else if (type == "radio")
        *out = nu::Button::Type::Radio;
#if defined(OS_MAC)
      else if (type == "disclosure")
        *out = nu::Button::Type::Disclosure;
#endif
      else
        return napi_invalid_arg;
    }
    return s;
  }
};

template<>
struct Type<nu::Button> {
  using Base = nu::View;
  static constexpr const char* name = "Button";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, constructor, "create", &Create);
    Set(env, prototype,
        "makeDefault", &nu::Button::MakeDefault,
        "setTitle", &nu::Button::SetTitle,
        "getTitle", &nu::Button::GetTitle,
#if defined(OS_MAC)
        "setButtonStyle", &nu::Button::SetButtonStyle,
        "setControlSize", &nu::Button::SetControlSize,
        "setHasBorder", &nu::Button::SetHasBorder,
        "hasBorder", &nu::Button::HasBorder,
#endif
        "setChecked", &nu::Button::SetChecked,
        "isChecked", &nu::Button::IsChecked,
        "setImage", &nu::Button::SetImage,
        "getImage", &nu::Button::GetImage);
    DefineProperties(env, prototype,
                     Signal("onClick", &nu::Button::on_click));
  }
  static nu::Button* Create(Arguments args, napi_value options) {
    std::string title;
    if (FromNode(args.Env(), options, &title)) {
      return new nu::Button(title);
    } else if (IsType(args.Env(), options, napi_object)) {
      nu::Button::Type type = nu::Button::Type::Normal;
      ReadOptions(args.Env(), options, "title", &title, "type", &type);
      return new nu::Button(title, type);
    } else {
      args.ThrowError("String or Object");
      return nullptr;
    }
  }
};

template<>
struct Type<nu::Canvas> {
  static constexpr const char* name = "Canvas";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, constructor,
        "create", &CreateOnHeap<nu::Canvas, const nu::SizeF&, float>,
        "createForMainScreen", &CreateOnHeap<nu::Canvas, const nu::SizeF&>);
    Set(env, prototype,
        "getScaleFactor", &nu::Canvas::GetScaleFactor,
        "getPainter", &nu::Canvas::GetPainter,
        "getSize", &nu::Canvas::GetSize);
  }
};

template<>
struct Type<nu::Clipboard::Data::Type> {
  static constexpr const char* name = "ClipboardDataType";
  static napi_status ToNode(napi_env env,
                            nu::Clipboard::Data::Type type,
                            napi_value* result) {
    switch (type) {
      case nu::Clipboard::Data::Type::None:
        return ConvertToNode(env, "none", result);
      case nu::Clipboard::Data::Type::Text:
        return ConvertToNode(env, "text", result);
      case nu::Clipboard::Data::Type::HTML:
        return ConvertToNode(env, "html", result);
      case nu::Clipboard::Data::Type::Image:
        return ConvertToNode(env, "image", result);
      case nu::Clipboard::Data::Type::FilePaths:
        return ConvertToNode(env, "file-paths", result);
    }
    NOTREACHED();
    return napi_generic_failure;
  }
  static napi_status FromNode(napi_env env,
                              napi_value value,
                              nu::Clipboard::Data::Type* out) {
    std::string type;
    napi_status s = ConvertFromNode(env, value, &type);
    if (s == napi_ok) {
      if (type == "none")
        *out = nu::Clipboard::Data::Type::None;
      else if (type == "text")
        *out = nu::Clipboard::Data::Type::Text;
      else if (type == "html")
        *out = nu::Clipboard::Data::Type::HTML;
      else if (type == "image")
        *out = nu::Clipboard::Data::Type::Image;
      else if (type == "file-paths")
        *out = nu::Clipboard::Data::Type::FilePaths;
      else
        return napi_invalid_arg;
    }
    return s;
  }
};

template<>
struct Type<nu::Clipboard::Data> {
  static constexpr const char* name = "ClipboardData";
  static napi_status ToNode(napi_env env,
                            const nu::Clipboard::Data& data,
                            napi_value* result) {
    *result = CreateObject(env);
    Set(env, *result, "type", data.type());
    switch (data.type()) {
      case nu::Clipboard::Data::Type::Text:
      case nu::Clipboard::Data::Type::HTML:
        Set(env, *result, "value", data.str());
        break;
      case nu::Clipboard::Data::Type::Image:
        Set(env, *result, "value", data.image());
        break;
      case nu::Clipboard::Data::Type::FilePaths:
        Set(env, *result, "value", data.file_paths());
        break;
      default:
        Set(env, *result, "value", nullptr);
        break;
    }
    return napi_ok;
  }
  static napi_status FromNode(napi_env env,
                              napi_value value,
                              nu::Clipboard::Data* out) {
    if (!IsType(env, value, napi_object))
      return napi_object_expected;
    nu::Clipboard::Data::Type type;
    if (!Get(env, value, "type", &type))
      return napi_invalid_arg;
    switch (type) {
      case nu::Clipboard::Data::Type::Text:
      case nu::Clipboard::Data::Type::HTML: {
        std::string str;
        if (!Get(env, value, "value", &str))
          return napi_invalid_arg;
        *out = nu::Clipboard::Data(type, std::move(str));
        break;
      }
      case nu::Clipboard::Data::Type::Image: {
        nu::Image* image;
        if (!Get(env, value, "value", &image))
          return napi_invalid_arg;
        *out = nu::Clipboard::Data(image);
        break;
      }
      case nu::Clipboard::Data::Type::FilePaths: {
        std::vector<base::FilePath> file_paths;
        if (!Get(env, value, "value", &file_paths))
          return napi_invalid_arg;
        *out = nu::Clipboard::Data(std::move(file_paths));
        break;
      }
      default:
        return napi_invalid_arg;
    }
    return napi_ok;
  }
};

template<>
struct Type<nu::Clipboard::Type> {
  static constexpr const char* name = "ClipboardType";
  static napi_status FromNode(napi_env env,
                              napi_value value,
                              nu::Clipboard::Type* out) {
    std::string type;
    napi_status s = ConvertFromNode(env, value, &type);
    if (s == napi_ok) {
      if (type == "copy-paste")
        *out = nu::Clipboard::Type::CopyPaste;
#if defined(OS_MAC)
      else if (type == "drag")
        *out = nu::Clipboard::Type::Drag;
      else if (type == "find")
        *out = nu::Clipboard::Type::Find;
      else if (type == "font")
        *out = nu::Clipboard::Type::Font;
#elif defined(OS_LINUX)
      else if (type == "selection")
        *out = nu::Clipboard::Type::Selection;
#endif
      else
        return napi_invalid_arg;
    }
    return s;
  }
};

template<>
struct Type<nu::Clipboard> {
  static constexpr const char* name = "Clipboard";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, constructor,
        "get", &nu::Clipboard::Get,
        "fromType", &nu::Clipboard::FromType);
    Set(env, prototype,
        "clear", &nu::Clipboard::Clear,
        "setText", &nu::Clipboard::SetText,
        "getText", &nu::Clipboard::GetText,
        "isDataAvailable", &nu::Clipboard::IsDataAvailable,
        "getData", &nu::Clipboard::GetData,
        "setData", &nu::Clipboard::SetData,
        "startWatching", &nu::Clipboard::StartWatching,
        "stopWatching", &nu::Clipboard::StopWatching);
    DefineProperties(env, prototype,
                     Signal("onChange", &nu::Clipboard::on_change));
  }
};

template<>
struct Type<nu::Color::Name> {
  static constexpr const char* name = "ColorName";
  static napi_status FromNode(napi_env env,
                              napi_value value,
                              nu::Color::Name* out) {
    std::string color_name;
    napi_status s = ConvertFromNode(env, value, &color_name);
    if (s == napi_ok) {
      if (color_name == "text")
        *out = nu::Color::Name::Text;
      else if (color_name == "disabled-text")
        *out = nu::Color::Name::DisabledText;
      else if (color_name == "text-edit-background")
        *out = nu::Color::Name::TextEditBackground;
      else if (color_name == "disabled-text-edit-background")
        *out = nu::Color::Name::DisabledTextEditBackground;
      else if (color_name == "control")
        *out = nu::Color::Name::Control;
      else if (color_name == "window-background")
        *out = nu::Color::Name::WindowBackground;
      else
        return napi_invalid_arg;
    }
    return s;
  }
};

template<>
struct Type<nu::Color> {
  static constexpr const char* name = "Color";
  static napi_status ToNode(napi_env env,
                            nu::Color color,
                            napi_value* result) {
    return ConvertToNode(env, color.value(), result);
  }
  static napi_status FromNode(napi_env env,
                              napi_value value,
                              nu::Color* out) {
    // Direct value.
    uint32_t result;
    if (napi_get_value_uint32(env, value, &result) == napi_ok) {
      *out = nu::Color(result);
      return napi_ok;
    }
    // String representation.
    std::string hex;
    napi_status s = ConvertFromNode(env, value, &hex);
    if (s == napi_ok)
      *out = nu::Color(hex);
    return s;
  }
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, constructor,
        "get", &nu::Color::Get,
        "rgb", &CreateOnStack<nu::Color, unsigned, unsigned, unsigned>,
        "argb", &CreateOnStack<nu::Color, unsigned, unsigned, unsigned,
                               unsigned>);
  }
};

template<>
struct Type<nu::ComboBox> {
  using Base = nu::Picker;
  static constexpr const char* name = "ComboBox";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, constructor,
        "create", &CreateOnHeap<nu::ComboBox>);
    Set(env, prototype,
        "setText", &nu::ComboBox::SetText,
        "getText", &nu::ComboBox::GetText);
    DefineProperties(
        env, prototype,
        Signal("onTextChange", &nu::ComboBox::on_text_change));
  }
};

template<>
struct Type<nu::Container> {
  using Base = nu::View;
  static constexpr const char* name = "Container";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, constructor,
        "create", &CreateOnHeap<nu::Container>);
    Set(env, prototype,
        "getPreferredSize", &nu::Container::GetPreferredSize,
        "getPreferredWidthForHeight",
        &nu::Container::GetPreferredWidthForHeight,
        "getPreferredHeightForWidth",
        &nu::Container::GetPreferredHeightForWidth,
        "addChildView",
        WrapMethod(&nu::Container::AddChildView, [](Arguments args) {
          AttachedTable(args).Set(args[0], true);
        }),
        "addChildViewAt",
        WrapMethod(&nu::Container::AddChildViewAt, [](Arguments args) {
          AttachedTable(args).Set(args[0], true);
        }),
        "removeChildView",
        WrapMethod(&nu::Container::RemoveChildView, [](Arguments args) {
          AttachedTable(args).Delete(args[0]);
        }),
        "childCount", &nu::Container::ChildCount,
        "childAt", &nu::Container::ChildAt);
    DefineProperties(
        env, prototype,
        Signal("onDraw", &nu::Container::on_draw));
  }
};

template<>
struct Type<nu::Cursor::Type> {
  static constexpr const char* name = "CursorType";
  static napi_status FromNode(napi_env env,
                              napi_value value,
                              nu::Cursor::Type* out) {
    std::string type;
    napi_status s = ConvertFromNode(env, value, &type);
    if (s == napi_ok) {
      if (type == "default")
        *out = nu::Cursor::Type::Default;
      else if (type == "hand")
        *out = nu::Cursor::Type::Hand;
      else if (type == "crosshair")
        *out = nu::Cursor::Type::Crosshair;
      else if (type == "progress")
        *out = nu::Cursor::Type::Progress;
      else if (type == "text")
        *out = nu::Cursor::Type::Text;
      else if (type == "not-allowed")
        *out = nu::Cursor::Type::NotAllowed;
      else if (type == "help")
        *out = nu::Cursor::Type::Help;
      else if (type == "move")
        *out = nu::Cursor::Type::Move;
      else if (type == "resize-ew")
        *out = nu::Cursor::Type::ResizeEW;
      else if (type == "resize-ns")
        *out = nu::Cursor::Type::ResizeNS;
      else if (type == "resize-nesw")
        *out = nu::Cursor::Type::ResizeNESW;
      else if (type == "resize-nwse")
        *out = nu::Cursor::Type::ResizeNWSE;
      else
        return napi_invalid_arg;
    }
    return s;
  }
};

template<>
struct Type<nu::Cursor> {
  static constexpr const char* name = "Cursor";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, constructor,
        "createWithType", &CreateOnHeap<nu::Cursor, nu::Cursor::Type>);
  }
};

template<>
struct Type<nu::DatePicker::Options> {
  static constexpr const char* name = "DatePickerOptions";
  static napi_status FromNode(napi_env env,
                              napi_value value,
                              nu::DatePicker::Options* out) {
    if (!ReadOptions(env, value,
                     "elements", &out->elements,
                     "hasStepper", &out->has_stepper))
      return napi_invalid_arg;
    return napi_ok;
  }
};

template<>
struct Type<nu::DatePicker> {
  using Base = nu::View;
  static constexpr const char* name = "DatePicker";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, constructor,
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
    Set(env, prototype,
        "setDate", &nu::DatePicker::SetDate,
        "getDate", &nu::DatePicker::GetDate,
        "setRange", &nu::DatePicker::SetRange,
        "getRange", &nu::DatePicker::GetRange,
        "hasStepper", &nu::DatePicker::HasStepper);
    DefineProperties(env, prototype,
                     Signal("onDateChange", &nu::DatePicker::on_date_change));
  }
};

template<>
struct Type<nu::DraggingInfo> {
  static constexpr const char* name = "DraggingInfo";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, constructor,
        "dragOperationNone", static_cast<int>(nu::DRAG_OPERATION_NONE),
        "dragOperationCopy", static_cast<int>(nu::DRAG_OPERATION_COPY),
        "dragOperationMove", static_cast<int>(nu::DRAG_OPERATION_MOVE),
        "dragOperationLink", static_cast<int>(nu::DRAG_OPERATION_LINK));
    Set(env, prototype,
        "isDataAvailable", &nu::DraggingInfo::IsDataAvailable,
        "getData", &nu::DraggingInfo::GetData,
        "getDragOperations", &nu::DraggingInfo::GetDragOperations);
  }
};

template<>
struct Type<nu::DragOptions> {
  static constexpr const char* name = "DragOptions";
  static napi_status FromNode(napi_env env,
                              napi_value value,
                              nu::DragOptions* out) {
    if (!ReadOptions(env, value, "image", &out->image))
      return napi_invalid_arg;
    return napi_ok;
  }
};

template<>
struct Type<nu::Entry::Type> {
  static constexpr const char* name = "EntryType";
  static napi_status FromNode(napi_env env,
                              napi_value value,
                              nu::Entry::Type* out) {
    std::string type;
    napi_status s = ConvertFromNode(env, value, &type);
    if (s == napi_ok) {
      if (type == "normal")
        *out = nu::Entry::Type::Normal;
      else if (type == "password")
        *out = nu::Entry::Type::Password;
      else
        return napi_invalid_arg;
    }
    return s;
  }
};

template<>
struct Type<nu::Entry> {
  using Base = nu::View;
  static constexpr const char* name = "Entry";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, constructor,
        "create", &CreateOnHeap<nu::Entry>,
        "createType", &CreateOnHeap<nu::Entry, nu::Entry::Type>);
    Set(env, prototype,
        "setText", &nu::Entry::SetText,
        "getText", &nu::Entry::GetText);
    DefineProperties(env, prototype,
                     Signal("onActivate", &nu::Entry::on_activate),
                     Signal("onTextChange", &nu::Entry::on_text_change));
  }
};

template<>
struct Type<nu::EventType> {
  static constexpr const char* name = "EventType";
  static napi_status ToNode(napi_env env,
                            nu::EventType type,
                            napi_value* result) {
    switch (type) {
      case nu::EventType::Unknown:
        return ConvertToNode(env, "unknown", result);
      case nu::EventType::MouseDown:
        return ConvertToNode(env, "mouseDown", result);
      case nu::EventType::MouseUp:
        return ConvertToNode(env, "mouseUp", result);
      case nu::EventType::MouseMove:
        return ConvertToNode(env, "mouseMove", result);
      case nu::EventType::MouseEnter:
        return ConvertToNode(env, "mouseEnter", result);
      case nu::EventType::MouseLeave:
        return ConvertToNode(env, "mouseLeave", result);
      case nu::EventType::KeyDown:
        return ConvertToNode(env, "keyDown", result);
      case nu::EventType::KeyUp:
        return ConvertToNode(env, "keyUp", result);
    }
    NOTREACHED();
    return napi_generic_failure;
  }
};

template<>
struct Type<nu::Event> {
  static constexpr const char* name = "Event";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, constructor,
        "maskShift", static_cast<int>(nu::MASK_SHIFT),
        "maskControl", static_cast<int>(nu::MASK_CONTROL),
        "maskAlt", static_cast<int>(nu::MASK_ALT),
        "maskMeta", static_cast<int>(nu::MASK_META),
        "isShiftPressed", &nu::Event::IsShiftPressed,
        "isControlPressed", &nu::Event::IsControlPressed,
        "isAltPressed", &nu::Event::IsAltPressed,
        "isMetaPressed", &nu::Event::IsMetaPressed);
  }
  // Used by subclasses.
  static void SetEventProperties(napi_env env,
                                 napi_value value,
                                 const nu::Event& event) {
    Set(env, value,
        "type", event.type,
        "modifiers", event.modifiers,
        "timestamp", event.timestamp);
  }
};

template<>
struct Type<nu::MouseEvent> {
  using Base = nu::Event;
  static constexpr const char* name = "MouseEvent";
  static napi_status ToNode(napi_env env,
                            const nu::MouseEvent& event,
                            napi_value* result) {
    *result = CreateObject(env);
    Type<nu::Event>::SetEventProperties(env, *result, event);
    Set(env, *result,
        "button", event.button,
        "positionInView", event.position_in_view,
        "positionInWindow", event.position_in_window);
    return napi_ok;
  }
};

template<>
struct Type<nu::KeyEvent> {
  using Base = nu::Event;
  static constexpr const char* name = "KeyEvent";
  static napi_status ToNode(napi_env env,
                            const nu::KeyEvent& event,
                            napi_value* result) {
    *result = CreateObject(env);
    Type<nu::Event>::SetEventProperties(env, *result, event);
    Set(env, *result, "key", event.key);
    return napi_ok;
  }
};

template<>
struct Type<nu::FileDialog::Filter> {
  static constexpr const char* name = "FileDialogFilter";
  static napi_status ToNode(napi_env env,
                            const nu::FileDialog::Filter& filter,
                            napi_value* result) {
    *result = CreateObject(env);
    Set(env, *result,
        "description", std::get<0>(filter),
        "extensions", std::get<1>(filter));
    return napi_ok;
  }
  static napi_status FromNode(napi_env env,
                              napi_value value,
                              nu::FileDialog::Filter* out) {
    return Get(env, value,
               "description", &std::get<0>(*out),
               "extensions", &std::get<1>(*out)) ? napi_ok : napi_invalid_arg;
  }
};

template<>
struct Type<nu::FileDialog> {
  static constexpr const char* name = "FileDialog";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, constructor,
        "optionPickFolders",
        static_cast<int>(nu::FileDialog::OPTION_PICK_FOLDERS),
        "optionMultiSelect",
        static_cast<int>(nu::FileDialog::OPTION_MULTI_SELECT),
        "optionShowHidden",
        static_cast<int>(nu::FileDialog::OPTION_SHOW_HIDDEN));
    Set(env, prototype,
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
  using Base = nu::FileDialog;
  static constexpr const char* name = "FileOpenDialog";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, constructor,
        "create", &CreateOnHeap<nu::FileOpenDialog>);
    Set(env, prototype,
        "getResults", &nu::FileOpenDialog::GetResults);
  }
};

template<>
struct Type<nu::FileSaveDialog> {
  using Base = nu::FileDialog;
  static constexpr const char* name = "FileSaveDialog";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, constructor,
        "create", &CreateOnHeap<nu::FileSaveDialog>);
  }
};

template<>
struct Type<nu::Font::Weight> {
  static constexpr const char* name = "FontWeight";
  static napi_status ToNode(napi_env env,
                            nu::Font::Weight weight,
                            napi_value* result) {
    switch (weight) {
      case nu::Font::Weight::Thin:
        return ConvertToNode(env, "thin", result);
      case nu::Font::Weight::ExtraLight:
        return ConvertToNode(env, "extra-light", result);
      case nu::Font::Weight::Light:
        return ConvertToNode(env, "light", result);
      case nu::Font::Weight::Normal:
        return ConvertToNode(env, "normal", result);
      case nu::Font::Weight::Medium:
        return ConvertToNode(env, "medium", result);
      case nu::Font::Weight::SemiBold:
        return ConvertToNode(env, "semi-bold", result);
      case nu::Font::Weight::Bold:
        return ConvertToNode(env, "bold", result);
      case nu::Font::Weight::ExtraBold:
        return ConvertToNode(env, "extra-bold", result);
      case nu::Font::Weight::Black:
        return ConvertToNode(env, "black", result);
    }
    NOTREACHED();
    return napi_generic_failure;
  }
  static napi_status FromNode(napi_env env,
                              napi_value value,
                              nu::Font::Weight* out) {
    std::string weight;
    napi_status s = ConvertFromNode(env, value, &weight);
    if (s == napi_ok) {
      if (weight == "thin")
        *out = nu::Font::Weight::Thin;
      else if (weight == "extra-light")
        *out = nu::Font::Weight::ExtraLight;
      else if (weight == "light")
        *out = nu::Font::Weight::Light;
      else if (weight == "normal")
        *out = nu::Font::Weight::Normal;
      else if (weight == "medium")
        *out = nu::Font::Weight::Medium;
      else if (weight == "semi-bold")
        *out = nu::Font::Weight::SemiBold;
      else if (weight == "bold")
        *out = nu::Font::Weight::Bold;
      else if (weight == "extra-bold")
        *out = nu::Font::Weight::ExtraBold;
      else if (weight == "black")
        *out = nu::Font::Weight::Black;
      else
        return napi_invalid_arg;
    }
    return s;
  }
};

template<>
struct Type<nu::Font::Style> {
  static constexpr const char* name = "FontStyle";
  static napi_status ToNode(napi_env env,
                            nu::Font::Style style,
                            napi_value* result) {
    switch (style) {
      case nu::Font::Style::Normal:
        return ConvertToNode(env, "normal", result);
      case nu::Font::Style::Italic:
        return ConvertToNode(env, "italic", result);
    }
    NOTREACHED();
    return napi_generic_failure;
  }
  static napi_status FromNode(napi_env env,
                              napi_value value,
                              nu::Font::Style* out) {
    std::string style;
    napi_status s = ConvertFromNode(env, value, &style);
    if (s == napi_ok) {
      if (style == "normal")
        *out = nu::Font::Style::Normal;
      else if (style == "italic")
        *out = nu::Font::Style::Italic;
      else
        return napi_invalid_arg;
    }
    return s;
  }
};

template<>
struct Type<nu::Font> {
  static constexpr const char* name = "Font";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, constructor,
        "create", &CreateOnHeap<nu::Font, const std::string&, float,
                                nu::Font::Weight, nu::Font::Style>,
        "createFromPath", &CreateOnHeap<nu::Font, const base::FilePath&, float>,
        "default", &nu::Font::Default);
    Set(env, prototype,
        "derive", &nu::Font::Derive,
        "getName", &nu::Font::GetName,
        "getSize", &nu::Font::GetSize,
        "getWeight", &nu::Font::GetWeight,
        "getStyle", &nu::Font::GetStyle);
  }
};

template<>
struct Type<nu::GifPlayer> {
  using Base = nu::View;
  static constexpr const char* name = "GifPlayer";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, constructor,
        "create", &CreateOnHeap<nu::GifPlayer>);
    Set(env, prototype,
        "setImage", &nu::GifPlayer::SetImage,
        "getImage", &nu::GifPlayer::GetImage,
        "setAnimating", &nu::GifPlayer::SetAnimating,
        "isAnimating", &nu::GifPlayer::IsAnimating,
        "setScale", &nu::GifPlayer::SetScale,
        "getScale", &nu::GifPlayer::GetScale);
  }
};

template<>
struct Type<nu::GlobalShortcut> {
  static constexpr const char* name = "GlobalShortcut";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, prototype,
        "register", &nu::GlobalShortcut::Register,
        "unregister", &nu::GlobalShortcut::Unregister,
        "unregisterAll", &nu::GlobalShortcut::UnregisterAll);
  }
};

template<>
struct Type<nu::Group> {
  using Base = nu::View;
  static constexpr const char* name = "Group";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, constructor,
        "create", &CreateOnHeap<nu::Group, const std::string&>);
    Set(env, prototype,
        "setContentView",
        WrapMethod(&nu::Group::SetContentView, [](Arguments args) {
          AttachedTable(args).Set("contentView", args[0]);
        }),
        "getContentView", &nu::Group::GetContentView,
        "setTitle", &nu::Group::SetTitle,
        "getTitle", &nu::Group::GetTitle);
  }
};

template<>
struct Type<nu::Image> {
  static constexpr const char* name = "Image";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, constructor,
        "createEmpty", &CreateOnHeap<nu::Image>,
        "createFromPath", &CreateOnHeap<nu::Image, const base::FilePath&>,
        "createFromBuffer", &CreateOnHeap<nu::Image, const nu::Buffer&, float>);
    Set(env, prototype,
        "isEmpty", &nu::Image::IsEmpty,
#if defined(OS_MAC)
        "setTemplate", &nu::Image::SetTemplate,
        "isTemplate", &nu::Image::IsTemplate,
#endif
        "getSize", &nu::Image::GetSize,
        "getScaleFactor", &nu::Image::GetScaleFactor,
        "tint", &nu::Image::Tint,
        "resize", &nu::Image::Resize,
        "toPNG", &nu::Image::ToPNG,
        "toJPEG", &nu::Image::ToJPEG);
  }
};

template<>
struct Type<nu::Label> {
  using Base = nu::View;
  static constexpr const char* name = "Label";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, constructor,
        "create", &CreateOnHeap<nu::Label, const std::string&>,
        "createWithAttributedText",
        &CreateOnHeap<nu::Label, nu::AttributedText*>);
    Set(env, prototype,
        "setText", &nu::Label::SetText,
        "getText", &nu::Label::GetText,
        "setAlign", &nu::Label::SetAlign,
        "setVAlign", &nu::Label::SetVAlign,
        "setAttributedText",
        WrapMethod(&nu::Label::SetAttributedText, [](Arguments args) {
          AttachedTable(args).Set("attributedText", args[0]);
        }),
        "getAttributedText", &nu::Label::GetAttributedText);
  }
};

#if defined(OS_MAC)
template<>
struct Type<nu::Lifetime::Reply> {
  static constexpr const char* name = "LifetimeReply";
  static napi_status ToNode(napi_env env,
                            nu::Lifetime::Reply reply,
                            napi_value* result) {
    switch (reply) {
      case nu::Lifetime::Reply::Success:
        return ConvertToNode(env, "success", result);
      case nu::Lifetime::Reply::Cancel:
        return ConvertToNode(env, "cancel", result);
      case nu::Lifetime::Reply::Failure:
        return ConvertToNode(env, "failure", result);
    }
    NOTREACHED();
    return napi_generic_failure;
  }
  static napi_status FromNode(napi_env env,
                              napi_value value,
                              nu::Lifetime::Reply* out) {
    std::string reply;
    napi_status s = ConvertFromNode(env, value, &reply);
    if (s == napi_ok) {
      if (reply == "success")
        *out = nu::Lifetime::Reply::Success;
      else if (reply == "cancel")
        *out = nu::Lifetime::Reply::Cancel;
      else if (reply == "failure")
        *out = nu::Lifetime::Reply::Failure;
      else
        return napi_invalid_arg;
    }
    return s;
  }
};
#endif

template<>
struct Type<nu::Lifetime> {
  static constexpr const char* name = "Lifetime";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
#if defined(OS_MAC)
    DefineProperties(env, prototype,
                     Signal("onReady", &nu::Lifetime::on_ready),
                     Signal("onActivate", &nu::Lifetime::on_activate),
                     Delegate("openFiles", &nu::Lifetime::open_files));
#endif
  }
};

template<>
struct Type<nu::Locale> {
  static constexpr const char* name = "Locale";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, constructor,
        "getCurrentIdentifier", &nu::Locale::GetCurrentIdentifier,
        "getPreferredLanguages", &nu::Locale::GetPreferredLanguages);
  }
};

template<>
struct Type<nu::MessageBox::Type> {
  static constexpr const char* name = "MessageBoxType";
  static napi_status FromNode(napi_env env,
                              napi_value value,
                              nu::MessageBox::Type* out) {
    std::string type;
    napi_status s = ConvertFromNode(env, value, &type);
    if (s == napi_ok) {
      if (type == "none")
        *out = nu::MessageBox::Type::None;
      else if (type == "information")
        *out = nu::MessageBox::Type::Information;
      else if (type == "warning")
        *out = nu::MessageBox::Type::Warning;
      else if (type == "error")
        *out = nu::MessageBox::Type::Error;
      else
        return napi_invalid_arg;
    }
    return s;
  }
};

template<>
struct Type<nu::MessageBox> {
  static constexpr const char* name = "MessageBox";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, constructor,
        "create", &CreateOnHeap<nu::MessageBox>);
    Set(env, prototype,
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
        WrapMethod(&nu::MessageBox::SetAccessoryView, [](Arguments args) {
          AttachedTable(args).Set("accessoryView", args[0]);
        }),
        "getAccessoryView", &nu::MessageBox::GetAccessoryView,
#endif
        "setImage", &nu::MessageBox::SetImage,
        "getImage", &nu::MessageBox::GetImage);
  }
};

template<>
struct Type<nu::MenuBase> {
  static constexpr const char* name = "MenuBase";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, prototype,
        "append", WrapMethod(&nu::MenuBase::Append, [](Arguments args) {
          AttachedTable(args).Set(args[0], true);
        }),
        "insert", WrapMethod(&nu::MenuBase::Insert, [](Arguments args) {
          AttachedTable(args).Set(args[0], true);
        }),
        "remove", WrapMethod(&nu::MenuBase::Remove, [](Arguments args) {
          AttachedTable(args).Delete(args[0]);
        }),
        "itemCount", &nu::MenuBase::ItemCount,
        "itemAt", &nu::MenuBase::ItemAt);
  }
};

void ReadMenuItems(napi_env env, napi_value options, nu::MenuBase* menu);

template<>
struct Type<nu::MenuBar> {
  using Base = nu::MenuBase;
  static constexpr const char* name = "MenuBar";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, constructor, "create", &Create);
  }
  static napi_value Create(napi_env env, napi_value options) {
    nu::MenuBar* menu = new nu::MenuBar;
    ReadMenuItems(env, options, menu);
    napi_value ret = ToNode(env, menu);
    // Remember the options.
    AttachedTable(env, ret).Set("options", options);
    return ret;
  }
};

template<>
struct Type<nu::Menu> {
  using Base = nu::MenuBase;
  static constexpr const char* name = "Menu";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, constructor, "create", &Create);
    Set(env, prototype,
        "popup", &nu::Menu::Popup,
        "popupAt", &nu::Menu::PopupAt);
  }
  static nu::Menu* CreateRaw(napi_env env, napi_value options) {
    nu::Menu* menu = new nu::Menu;
    ReadMenuItems(env, options, menu);
    return menu;
  }
  static napi_value Create(napi_env env, napi_value options) {
    napi_value ret = ToNode(env, CreateRaw(env, options));
    // Remember the options.
    AttachedTable(env, ret).Set("options", options);
    return ret;
  }
};

template<>
struct Type<nu::MenuItem::Type> {
  static constexpr const char* name = "MenuItemType";
  static napi_status FromNode(napi_env env,
                              napi_value value,
                              nu::MenuItem::Type* out) {
    std::string type;
    napi_status s = ConvertFromNode(env, value, &type);
    if (s == napi_ok) {
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
        return napi_invalid_arg;
    }
    return s;
  }
};

template<>
struct Type<nu::MenuItem::Role> {
  static constexpr const char* name = "MenuItemRole";
  static napi_status FromNode(napi_env env,
                              napi_value value,
                              nu::MenuItem::Role* out) {
    std::string role;
    napi_status s = ConvertFromNode(env, value, &role);
    if (s == napi_ok) {
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
        return napi_invalid_arg;
    }
    return s;
  }
};

template<>
struct Type<nu::MenuItem> {
  static constexpr const char* name = "MenuItem";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, constructor, "create", &Create);
    Set(env, prototype,
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
    DefineProperties(env, prototype,
                     Signal("onClick", &nu::MenuItem::on_click),
                     Delegate("validate", &nu::MenuItem::validate));
  }
  static nu::MenuItem* CreateRaw(napi_env env, napi_value options) {
    nu::MenuItem::Type type = nu::MenuItem::Type::Label;
    if (FromNode(env, options, &type) || !IsType(env, options, napi_object))
      return new nu::MenuItem(type);
    nu::MenuItem* item = nullptr;
    // First read role.
    nu::MenuItem::Role role;
    if (Get(env, options, "role", &role))
      item = new nu::MenuItem(role);
    // Use label if "type" is not specified.
    if (Get(env, options, "type", &type))
      item = new nu::MenuItem(type);
    // Read table fields and set attributes.
    bool b = false;
    if (Get(env, options, "checked", &b)) {
      if (!item) item = new nu::MenuItem(nu::MenuItem::Type::Checkbox);
      item->SetChecked(b);
    }
    nu::Menu* submenu = nullptr;
    napi_value arr;
    if (Get(env, options, "submenu", &submenu) ||
        (Get(env, options, "submenu", &arr) && IsArray(env, arr))) {
      if (!submenu) submenu = Type<nu::Menu>::CreateRaw(env, arr);
      if (!item) item = new nu::MenuItem(nu::MenuItem::Type::Submenu);
      item->SetSubmenu(submenu);
    }
    if (!item)  // can not deduce type from property, assuming Label item.
      item = new nu::MenuItem(nu::MenuItem::Type::Label);
    if (Get(env, options, "visible", &b))
      item->SetVisible(b);
    if (Get(env, options, "enabled", &b))
      item->SetEnabled(b);
    std::string label;
    if (Get(env, options, "label", &label))
      item->SetLabel(label);
    nu::Accelerator accelerator;
    if (Get(env, options, "accelerator", &accelerator))
      item->SetAccelerator(accelerator);
#if defined(OS_MAC) || defined(OS_WIN)
    nu::Image* image;
    if (Get(env, options, "image", &image))
      item->SetImage(image);
#endif
    // The signal handler must not be referenced by C++.
    napi_value on_click_val = nullptr;
    std::function<void(nu::MenuItem*)> on_click;
    if (Get(env, options, "onClick", &on_click_val) &&
        ConvertWeakFunctionFromNode(env, on_click_val, &on_click) == napi_ok)
      item->on_click.Connect(on_click);
    napi_value validate_val = nullptr;
    std::function<bool(nu::MenuItem*)> validate;
    if (Get(env, options, "validate", &validate_val) &&
        ConvertWeakFunctionFromNode(env, validate_val, &validate) == napi_ok)
      item->validate = validate;
    return item;
  }
  static napi_value Create(napi_env env, napi_value options) {
    napi_value ret = ToNode(env, CreateRaw(env, options));
    // Remember the options.
    AttachedTable(env, ret).Set("options", options);
    return ret;
  }
};

void ReadMenuItems(napi_env env, napi_value arr, nu::MenuBase* menu) {
  std::vector<napi_value> items;
  if (!FromNode(env, arr, &items))
    return;
  for (napi_value obj : items) {
    // Create the item if an object is passed.
    nu::MenuItem* item;
    if (!FromNode(env, obj, &item))
      item = Type<nu::MenuItem>::CreateRaw(env, obj);
    menu->Append(item);
  }
}

template<>
struct Type<nu::MessageLoop> {
  static constexpr const char* name = "MessageLoop";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, constructor,
        "quit", &nu::MessageLoop::Quit,
        "postTask", &nu::MessageLoop::PostTask,
        "postDelayedTask", &nu::MessageLoop::PostDelayedTask);
    // The "run" method should never be used in yode runtime.
    if (!is_yode) {
      Set(env, constructor, "run", &nu::MessageLoop::Run);
    }
  }
};

template<>
struct Type<nu::Notification::Action> {
  static constexpr const char* name = "NotificationAction";
  static napi_status FromNode(napi_env env,
                              napi_value value,
                              nu::Notification::Action* out) {
    return Get(env, value,
               "title", &out->title,
               "info", &out->info) ? napi_ok : napi_invalid_arg;
  }
};

template<>
struct Type<nu::Notification> {
  static constexpr const char* name = "Notification";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, constructor,
        "create", &CreateOnHeap<nu::Notification>);
    Set(env, prototype,
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
  static napi_status FromNode(napi_env env,
                              napi_value value,
                              nu::NotificationCenter::COMServerOptions* out) {
    if (!ReadOptions(env, value,
                     "writeRegistry", &out->write_registry,
                     "arguments", &out->arguments,
                     "toastActivatorClsid", &out->toast_activator_clsid))
      return napi_invalid_arg;
    return napi_ok;
  }
};

template<>
struct Type<nu::NotificationCenter::InputData> {
  static constexpr const char* name = "NotificationCenterInputData";
  static napi_status ToNode(napi_env env,
                            const nu::NotificationCenter::InputData& data,
                            napi_value* result) {
    *result = CreateObject(env);
    Set(env, *result, "key", data.key, "value", data.value);
    return napi_ok;
  }
};
#endif

template<>
struct Type<nu::NotificationCenter> {
  static constexpr const char* name = "NotificationCenter";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, prototype,
        "clear", &nu::NotificationCenter::Clear);
#if defined(OS_WIN)
    Set(env, prototype,
        "setCOMServerOptions",
        &nu::NotificationCenter::SetCOMServerOptions,
        "registerCOMServer",
        &nu::NotificationCenter::RegisterCOMServer,
        "removeCOMServerFromRegistry",
        &nu::NotificationCenter::RemoveCOMServerFromRegistry,
        "getToastActivatorCLSID",
        &nu::NotificationCenter::GetToastActivatorCLSID);
#endif
    DefineProperties(
        env, prototype,
#if defined(OS_MAC) || defined(OS_WIN)
        Signal("onNotificationReply",
               &nu::NotificationCenter::on_notification_reply),
#endif
#if defined(OS_WIN)
        Signal("onToastActivate", &nu::NotificationCenter::on_toast_activate),
#endif
        Signal("onNotificationShow",
               &nu::NotificationCenter::on_notification_show),
        Signal("onNotificationClose",
               &nu::NotificationCenter::on_notification_close),
        Signal("onNotificationClick",
               &nu::NotificationCenter::on_notification_click),
        Signal("onNotificationAction",
               &nu::NotificationCenter::on_notification_action));
  }
};

template<>
struct Type<nu::Painter> {
  static constexpr const char* name = "Painter";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, prototype,
        "save", &nu::Painter::Save,
        "restore", &nu::Painter::Restore,
        "setBlendMode", &nu::Painter::SetBlendMode,
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
struct Type<nu::Picker> {
  using Base = nu::View;
  static constexpr const char* name = "Picker";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, constructor,
        "create", &CreateOnHeap<nu::Picker>);
    Set(env, prototype,
        "addItem", &nu::Picker::AddItem,
        "removeItemAt", &nu::Picker::RemoveItemAt,
        "clear", &nu::Picker::Clear,
        "getItems", &nu::Picker::GetItems,
        "selectItemAt", &nu::Picker::SelectItemAt,
        "getSelectedItem", &nu::Picker::GetSelectedItem,
        "getSelectedItemIndex", &nu::Picker::GetSelectedItemIndex);
    DefineProperties(
        env, prototype,
        Signal("onSelectionChange", &nu::Picker::on_selection_change));
  }
};

template<>
struct Type<nu::ProgressBar> {
  using Base = nu::View;
  static constexpr const char* name = "ProgressBar";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, constructor,
        "create", &CreateOnHeap<nu::ProgressBar>);
    Set(env, prototype,
        "setValue", &nu::ProgressBar::SetValue,
        "getValue", &nu::ProgressBar::GetValue,
        "setIndeterminate", &nu::ProgressBar::SetIndeterminate,
        "isIndeterminate", &nu::ProgressBar::IsIndeterminate);
  }
};

template<>
struct Type<nu::ProtocolJob> {
  static constexpr const char* name = "ProtocolJob";
};

template<>
struct Type<nu::ProtocolAsarJob> {
  using Base = nu::ProtocolFileJob;
  static constexpr const char* name = "ProtocolAsarJob";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, constructor,
        "create", &CreateOnHeap<nu::ProtocolAsarJob,
                                const base::FilePath&,
                                const std::string&>);
    Set(env, prototype,
        "setDecipher", &nu::ProtocolAsarJob::SetDecipher);
  }
};

template<>
struct Type<nu::ProtocolFileJob> {
  using Base = nu::ProtocolJob;
  static constexpr const char* name = "ProtocolFileJob";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, constructor,
        "create", &CreateOnHeap<nu::ProtocolFileJob,
                                const ::base::FilePath&>);
  }
};

template<>
struct Type<nu::ProtocolStringJob> {
  using Base = nu::ProtocolJob;
  static constexpr const char* name = "ProtocolStringJob";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, constructor,
        "create", &CreateOnHeap<nu::ProtocolStringJob,
                                const std::string&,
                                const std::string&>);
  }
};

template<>
struct Type<nu::Responder> {
  static constexpr const char* name = "Responder";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, prototype,
        "setCapture", &nu::Responder::SetCapture,
        "releaseCapture", &nu::Responder::ReleaseCapture,
        "hasCapture", &nu::Responder::HasCapture);
#if defined(OS_LINUX) || defined(OS_MAC)
    Set(env, prototype, "getNative", GetNative);
#endif
    DefineProperties(
        env, prototype,
        Signal("onMouseDown", &nu::Responder::on_mouse_down),
        Signal("onMouseUp", &nu::Responder::on_mouse_up),
        Signal("onMouseMove", &nu::Responder::on_mouse_move),
        Signal("onMouseEnter", &nu::Responder::on_mouse_enter),
        Signal("onMouseLeave", &nu::Responder::on_mouse_leave),
        Signal("onKeyDown", &nu::Responder::on_key_down),
        Signal("onKeyUp", &nu::Responder::on_key_up),
        Signal("onCaptureLost", &nu::View::on_capture_lost));
  }
#if defined(OS_LINUX) || defined(OS_MAC)
  static napi_value GetNative(Arguments args) {
    nu::Responder* responder;
    if (!args.GetThis(&responder))
      return nullptr;
    napi_value buffer;
    void* data;
    if (napi_create_buffer(args.Env(), sizeof(nu::NativeResponder), &data,
                           &buffer) != napi_ok)
      return nullptr;
    nu::NativeResponder native = responder->GetNative();
    memcpy(data, &native, sizeof(nu::NativeResponder));
    return buffer;
  }
#endif
};

template<>
struct Type<nu::Screen> {
  static constexpr const char* name = "Screen";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, prototype,
        "getPrimaryDisplay", &nu::Screen::GetPrimaryDisplay,
        "getAllDisplays", &nu::Screen::GetAllDisplays,
        "getDisplayNearestWindow", &nu::Screen::GetDisplayNearestWindow,
        "getDisplayNearestPoint", &nu::Screen::GetDisplayNearestPoint,
        "getCursorScreenPoint", &nu::Screen::GetCursorScreenPoint);
    DefineProperties(env, prototype,
                     Signal("onAddDisplay", &nu::Screen::on_add_display),
                     Signal("onRemoveDisplay", &nu::Screen::on_remove_display),
                     Signal("onUpdateDisplay", &nu::Screen::on_update_display));
  }
};

template<>
struct Type<nu::Scroll::Policy> {
  static constexpr const char* name = "ScrollPolicy";
  static napi_status ToNode(napi_env env,
                            nu::Scroll::Policy policy,
                            napi_value* result) {
    switch (policy) {
      case nu::Scroll::Policy::Always:
        return ConvertToNode(env, "always", result);
      case nu::Scroll::Policy::Never:
        return ConvertToNode(env, "never", result);
      case nu::Scroll::Policy::Automatic:
        return ConvertToNode(env, "automatic", result);
    }
    NOTREACHED();
    return napi_generic_failure;
  }
  static napi_status FromNode(napi_env env,
                              napi_value value,
                              nu::Scroll::Policy* out) {
    std::string policy;
    napi_status s = ConvertFromNode(env, value, &policy);
    if (s == napi_ok) {
      if (policy == "always")
        *out = nu::Scroll::Policy::Always;
      else if (policy == "never")
        *out = nu::Scroll::Policy::Never;
      else if (policy == "automatic")
        *out = nu::Scroll::Policy::Automatic;
      else
        return napi_invalid_arg;
    }
    return s;
  }
};

#if defined(OS_MAC)
template<>
struct Type<nu::Scroll::Elasticity> {
  static constexpr const char* name = "ScrollElasticity";
  static napi_status ToNode(napi_env env,
                            nu::Scroll::Elasticity elasticity,
                            napi_value* result) {
    switch (elasticity) {
      case nu::Scroll::Elasticity::Allowed:
        return ConvertToNode(env, "allowed", result);
      case nu::Scroll::Elasticity::None:
        return ConvertToNode(env, "none", result);
      case nu::Scroll::Elasticity::Automatic:
        return ConvertToNode(env, "automatic", result);
    }
    NOTREACHED();
    return napi_generic_failure;
  }
  static napi_status FromNode(napi_env env,
                              napi_value value,
                              nu::Scroll::Elasticity* out) {
    std::string elasticity;
    napi_status s = ConvertFromNode(env, value, &elasticity);
    if (s == napi_ok) {
      if (elasticity == "allowed")
        *out = nu::Scroll::Elasticity::Allowed;
      else if (elasticity == "none")
        *out = nu::Scroll::Elasticity::None;
      else if (elasticity == "automatic")
        *out = nu::Scroll::Elasticity::Automatic;
      else
        return napi_invalid_arg;
    }
    return s;
  }
};
#endif

template<>
struct Type<nu::Scroll> {
  using Base = nu::View;
  static constexpr const char* name = "Scroll";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, constructor,
        "create", &CreateOnHeap<nu::Scroll>);
    Set(env, prototype,
        "setContentSize", &nu::Scroll::SetContentSize,
        "getContentSize", &nu::Scroll::GetContentSize,
        "setScrollPosition", &nu::Scroll::SetScrollPosition,
        "getScrollPosition", &nu::Scroll::GetScrollPosition,
        "getMaximumScrollPosition", &nu::Scroll::GetMaximumScrollPosition,
        "setContentView",
        WrapMethod(&nu::Scroll::SetContentView, [](Arguments args) {
          AttachedTable(args).Set("contentView", args[0]);
        }),
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
    DefineProperties(env, prototype,
                     Signal("onScroll", &nu::Scroll::on_scroll));
  }
};

template<>
struct Type<nu::Separator> {
  using Base = nu::View;
  static constexpr const char* name = "Separator";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, constructor,
        "create", &CreateOnHeap<nu::Separator, nu::Orientation>);
  }
};

template<>
struct Type<nu::Slider> {
  using Base = nu::View;
  static constexpr const char* name = "Slider";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, constructor,
        "create", &CreateOnHeap<nu::Slider>);
    Set(env, prototype,
        "setValue", &nu::Slider::SetValue,
        "getValue", &nu::Slider::GetValue,
        "setStep", &nu::Slider::SetStep,
        "getStep", &nu::Slider::GetStep,
        "setRange", &nu::Slider::SetRange,
        "getRange", &nu::Slider::GetRange);
    DefineProperties(
        env, prototype,
        Signal("onValueChange", &nu::Slider::on_value_change),
        Signal("onSlidingComplete", &nu::Slider::on_sliding_complete));
  }
};

template<>
struct Type<nu::Tab> {
  using Base = nu::View;
  static constexpr const char* name = "Tab";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, constructor,
        "create", &CreateOnHeap<nu::Tab>);
    Set(env, prototype,
        "addPage", WrapMethod(&nu::Tab::AddPage, [](Arguments args) {
          AttachedTable(args).Set(args[1], true);
        }),
        "removePage", WrapMethod(&nu::Tab::RemovePage, [](Arguments args) {
          AttachedTable(args).Delete(args[0]);
        }),
        "pageCount", &nu::Tab::PageCount,
        "pageAt", &nu::Tab::PageAt,
        "selectPageAt", &nu::Tab::SelectPageAt,
        "getSelectedPage", &nu::Tab::GetSelectedPage,
        "getSelectedPageIndex", &nu::Tab::GetSelectedPageIndex);
    DefineProperties(
        env, prototype,
        Signal("onSelectedPageChange", &nu::Tab::on_selected_page_change));
  }
};

template<>
struct Type<nu::TableModel> {
  static constexpr const char* name = "TableModel";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, prototype,
        "getRowCount", &nu::TableModel::GetRowCount,
        "getValue", &nu::TableModel::GetValue,
        "notifyRowInsertion", &nu::TableModel::NotifyRowInsertion,
        "notifyRowDeletion", &nu::TableModel::NotifyRowDeletion,
        "notifyValueChange", &nu::TableModel::NotifyValueChange);
  }
};

template<>
struct Type<nu::AbstractTableModel> {
  using Base = nu::TableModel;
  static constexpr const char* name = "AbstractTableModel";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, constructor,
        "create", &CreateOnHeap<nu::AbstractTableModel>);
    DefineProperties(
        env, prototype,
        Delegate("getRowCount", &nu::AbstractTableModel::get_row_count),
        Delegate("setValue", &nu::AbstractTableModel::set_value),
        Delegate("getValue", &nu::AbstractTableModel::get_value));
  }
};

template<>
struct Type<nu::SimpleTableModel> {
  using Base = nu::TableModel;
  static constexpr const char* name = "SimpleTableModel";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, constructor,
        "create", &CreateOnHeap<nu::SimpleTableModel, uint32_t>);
    Set(env, prototype,
        "addRow", &nu::SimpleTableModel::AddRow,
        "removeRowAt", &nu::SimpleTableModel::RemoveRowAt,
        "setValue", &nu::SimpleTableModel::SetValue);
  }
};

template<>
struct Type<nu::Table::ColumnType> {
  static constexpr const char* name = "TableColumnType";
  static napi_status FromNode(napi_env env,
                              napi_value value,
                              nu::Table::ColumnType* out) {
    std::string type;
    napi_status s = ConvertFromNode(env, value, &type);
    if (s == napi_ok) {
      if (type == "text")
        *out = nu::Table::ColumnType::Text;
      else if (type == "edit")
        *out = nu::Table::ColumnType::Edit;
      else if (type == "custom")
        *out = nu::Table::ColumnType::Custom;
      else
        return napi_invalid_arg;
    }
    return s;
  }
};

template<>
struct Type<nu::Table::ColumnOptions> {
  static constexpr const char* name = "TableColumnOptions";
  static napi_status FromNode(napi_env env,
                              napi_value value,
                              nu::Table::ColumnOptions* out) {
    napi_value on_draw_val = nullptr;
    if (!ReadOptions(env, value,
                     "onDraw", &on_draw_val,
                     "type", &out->type,
                     "column", &out->column,
                     "width", &out->width))
      return napi_invalid_arg;
    if (on_draw_val)
      ConvertWeakFunctionFromNode(env, on_draw_val, &out->on_draw);
    return napi_ok;
  }
};

template<>
struct Type<nu::Table> {
  using Base = nu::View;
  static constexpr const char* name = "Table";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, constructor,
        "create", &CreateOnHeap<nu::Table>);
    Set(env, prototype,
        "setModel",
        WrapMethod(&nu::Table::SetModel, [](Arguments args) {
          AttachedTable(args).Set("model", args[0]);
        }),
        "getModel", &nu::Table::GetModel,
        "addColumn", &nu::Table::AddColumn,
        "addColumnWithOptions",
        WrapMethod(&nu::Table::AddColumnWithOptions, [](Arguments args) {
          AttachedTable(args).Set(args[1], true);
        }),
        "getColumnCount", &nu::Table::GetColumnCount,
        "setColumnsVisible", &nu::Table::SetColumnsVisible,
        "isColumnsVisible", &nu::Table::IsColumnsVisible,
        "setRowHeight", &nu::Table::SetRowHeight,
        "getRowHeight", &nu::Table::GetRowHeight,
        "setHasBorder", &nu::Table::SetHasBorder,
        "hasBorder", &nu::Table::HasBorder,
        "enableMultipleSelection", &nu::Table::EnableMultipleSelection,
        "isMultipleSelectionEnabled", &nu::Table::IsMultipleSelectionEnabled,
        "selectRow", &nu::Table::SelectRow,
        "getSelectedRow", &nu::Table::GetSelectedRow,
        "selectRows", &nu::Table::SelectRows,
        "getSelectedRows", &nu::Table::GetSelectedRows);
    DefineProperties(
        env, prototype,
        Signal("onSelectionChange", &nu::Table::on_selection_change),
        Signal("onRowActivate", &nu::Table::on_row_activate));
  }
};

template<>
struct Type<nu::TextEdit> {
  using Base = nu::View;
  static constexpr const char* name = "TextEdit";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, constructor,
        "create", &CreateOnHeap<nu::TextEdit>);
    Set(env, prototype,
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
    DefineProperties(
        env, prototype,
        Signal("onTextChange", &nu::TextEdit::on_text_change),
        Delegate("shouldInsertNewLine", &nu::TextEdit::should_insert_new_line));
  }
};

#if defined(OS_MAC)
template<>
struct Type<nu::Toolbar::Item> {
  static constexpr const char* name = "ToolbarItem";
  static napi_status ToNode(napi_env env,
                            const nu::Toolbar::Item& item,
                            napi_value* result) {
    *result = CreateObject(env);
    Set(env, *result,
        "image", item.image,
        "view", item.view,
        "label", item.label,
        "minSize", item.min_size,
        "maxSize", item.max_size,
        "subitems", item.subitems,
        "onClick", item.on_click);
    return napi_ok;
  }
  static napi_status FromNode(napi_env env,
                              napi_value value,
                              nu::Toolbar::Item* out) {
    if (!ReadOptions(env, value,
                     "image", &out->image,
                     "view", &out->view,
                     "label", &out->label,
                     "minSize", &out->min_size,
                     "maxSize", &out->max_size,
                     "subitems", &out->subitems,
                     "onClick", &out->on_click))
      return napi_invalid_arg;
    return napi_ok;
  }
};

template<>
struct Type<nu::Toolbar::DisplayMode> {
  static constexpr const char* name = "ToolbarDisplayMode";
  static napi_status FromNode(napi_env env,
                              napi_value value,
                              nu::Toolbar::DisplayMode* out) {
    std::string mode;
    napi_status s = ConvertFromNode(env, value, &mode);
    if (s == napi_ok) {
      if (mode == "default")
        *out = nu::Toolbar::DisplayMode::Default;
      else if (mode == "icon-and-label")
        *out = nu::Toolbar::DisplayMode::IconAndLabel;
      else if (mode == "icon")
        *out = nu::Toolbar::DisplayMode::Icon;
      else if (mode == "label")
        *out = nu::Toolbar::DisplayMode::Label;
      else
        return napi_invalid_arg;
    }
    return s;
  }
};

template<>
struct Type<nu::Toolbar> {
  static constexpr const char* name = "Toolbar";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, constructor,
        "create", &CreateOnHeap<nu::Toolbar, const std::string&>);
    Set(env, prototype,
        "setDefaultItemIdentifiers", &nu::Toolbar::SetDefaultItemIdentifiers,
        "setAllowedItemIdentifiers", &nu::Toolbar::SetAllowedItemIdentifiers,
        "setAllowCustomization", &nu::Toolbar::SetAllowCustomization,
        "setDisplayMode", &nu::Toolbar::SetDisplayMode,
        "setVisible", &nu::Toolbar::SetVisible,
        "isVisible", &nu::Toolbar::IsVisible,
        "getIdentifier", &nu::Toolbar::GetIdentifier);
    DefineProperties(env, prototype,
                     Delegate("getItem", &nu::Toolbar::get_item));
  }
};
#endif

template<>
struct Type<nu::Tray> {
  static constexpr const char* name = "Tray";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, constructor,
#if defined(OS_MAC)
        "createWithTitle", &CreateOnHeap<nu::Tray, const std::string&>,
#endif
        "createWithImage", &CreateOnHeap<nu::Tray, scoped_refptr<nu::Image>>);
    Set(env, prototype,
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
        "setMenu", WrapMethod(&nu::Tray::SetMenu, [](Arguments args) {
          AttachedTable(args).Set("menu", args[0]);
        }));
    DefineProperties(env, prototype,
                     Signal("onClick", &nu::Tray::on_click));
  }
};

#if defined(OS_MAC)
template<>
struct Type<nu::Vibrant::Material> {
  static constexpr const char* name = "VibrantMaterial";
  static napi_status ToNode(napi_env env,
                            nu::Vibrant::Material material,
                            napi_value* result) {
    switch (material) {
      case nu::Vibrant::Material::AppearanceBased:
        return ConvertToNode(env, "appearance-based", result);
      case nu::Vibrant::Material::Light:
        return ConvertToNode(env, "light", result);
      case nu::Vibrant::Material::Dark:
        return ConvertToNode(env, "dark", result);
      case nu::Vibrant::Material::Titlebar:
        return ConvertToNode(env, "titlebar", result);
      case nu::Vibrant::Material::Selection:
        return ConvertToNode(env, "selection", result);
      case nu::Vibrant::Material::Menu:
        return ConvertToNode(env, "menu", result);
      case nu::Vibrant::Material::Popover:
        return ConvertToNode(env, "popover", result);
      case nu::Vibrant::Material::Sidebar:
        return ConvertToNode(env, "sidebar", result);
      case nu::Vibrant::Material::HeaderView:
        return ConvertToNode(env, "header-view", result);
      case nu::Vibrant::Material::Sheet:
        return ConvertToNode(env, "sheet", result);
      case nu::Vibrant::Material::WindowBackground:
        return ConvertToNode(env, "window-background", result);
      case nu::Vibrant::Material::HUDWindow:
        return ConvertToNode(env, "hudwindow", result);
      case nu::Vibrant::Material::FullscreenUI:
        return ConvertToNode(env, "fullscreen-ui", result);
      case nu::Vibrant::Material::Tooltip:
        return ConvertToNode(env, "tooltip", result);
      case nu::Vibrant::Material::ContentBackground:
        return ConvertToNode(env, "content-background", result);
      case nu::Vibrant::Material::UnderWindowBackground:
        return ConvertToNode(env, "under-window-background", result);
      case nu::Vibrant::Material::UnderPageBackground:
        return ConvertToNode(env, "under-page-background", result);
    }
    NOTREACHED();
    return napi_generic_failure;
  }
  static napi_status FromNode(napi_env env,
                              napi_value value,
                              nu::Vibrant::Material* out) {
    std::string material;
    napi_status s = ConvertFromNode(env, value, &material);
    if (s == napi_ok) {
      if (material == "appearance-based")
        *out = nu::Vibrant::Material::AppearanceBased;
      else if (material == "light")
        *out = nu::Vibrant::Material::Light;
      else if (material == "dark")
        *out = nu::Vibrant::Material::Dark;
      else if (material == "titlebar")
        *out = nu::Vibrant::Material::Titlebar;
      else if (material == "selection")
        *out = nu::Vibrant::Material::Selection;
      else if (material == "menu")
        *out = nu::Vibrant::Material::Menu;
      else if (material == "popover")
        *out = nu::Vibrant::Material::Popover;
      else if (material == "sidebar")
        *out = nu::Vibrant::Material::Sidebar;
      else if (material == "header-view")
        *out = nu::Vibrant::Material::HeaderView;
      else if (material == "sheet")
        *out = nu::Vibrant::Material::Sheet;
      else if (material == "window-background")
        *out = nu::Vibrant::Material::WindowBackground;
      else if (material == "hudwindow")
        *out = nu::Vibrant::Material::HUDWindow;
      else if (material == "fullscreen-ui")
        *out = nu::Vibrant::Material::FullscreenUI;
      else if (material == "tooltip")
        *out = nu::Vibrant::Material::Tooltip;
      else if (material == "content-background")
        *out = nu::Vibrant::Material::ContentBackground;
      else if (material == "under-window-background")
        *out = nu::Vibrant::Material::UnderWindowBackground;
      else if (material == "under-page-background")
        *out = nu::Vibrant::Material::UnderPageBackground;
      else
        return napi_invalid_arg;
    }
    return s;
  }
};

template<>
struct Type<nu::Vibrant::BlendingMode> {
  static constexpr const char* name = "VibrantBlendingMode";
  static napi_status ToNode(napi_env env,
                            nu::Vibrant::BlendingMode material,
                            napi_value* result) {
    switch (material) {
      case nu::Vibrant::BlendingMode::WithinWindow:
        return ConvertToNode(env, "within-window", result);
      case nu::Vibrant::BlendingMode::BehindWindow:
        return ConvertToNode(env, "behind-window", result);
    }
    NOTREACHED();
    return napi_generic_failure;
  }
  static napi_status FromNode(napi_env env,
                              napi_value value,
                              nu::Vibrant::BlendingMode* out) {
    std::string material;
    napi_status s = ConvertFromNode(env, value, &material);
    if (s == napi_ok) {
      if (material == "within-window")
        *out = nu::Vibrant::BlendingMode::WithinWindow;
      else if (material == "behind-window")
        *out = nu::Vibrant::BlendingMode::BehindWindow;
      else
        return napi_invalid_arg;
    }
    return s;
  }
};

template<>
struct Type<nu::Vibrant> {
  using Base = nu::Container;
  static constexpr const char* name = "Vibrant";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, constructor,
        "create", &CreateOnHeap<nu::Vibrant>);
    Set(env, prototype,
        "setMaterial", &nu::Vibrant::SetMaterial,
        "getMaterial", &nu::Vibrant::GetMaterial,
        "setBlendingMode", &nu::Vibrant::SetBlendingMode,
        "getBlendingMode", &nu::Vibrant::GetBlendingMode);
  }
};
#endif

template<>
struct Type<nu::View> {
  using Base = nu::Responder;
  static constexpr const char* name = "View";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, prototype,
        "offsetFromView", &nu::View::OffsetFromView,
        "offsetFromWindow", &nu::View::OffsetFromWindow,
        "setBounds", &nu::View::SetBounds,
        "getBounds", &nu::View::GetBounds,
        "getBoundsInScreen", &nu::View::GetBoundsInScreen,
        "layout", &nu::View::Layout,
        "schedulePaint", &nu::View::SchedulePaint,
        "schedulePaintRect", &nu::View::SchedulePaintRect,
        "setVisible", &nu::View::SetVisible,
        "isVisible", &nu::View::IsVisible,
        "isVisibleInHierarchy", &nu::View::IsVisibleInHierarchy,
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
        "setTooltip", &nu::View::SetTooltip,
        "addTooltipForRect", &nu::View::AddTooltipForRect,
        "removeTooltip", &nu::View::RemoveTooltip,
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
    DefineProperties(
        env, prototype,
        Signal("onDragLeave", &nu::View::on_drag_leave),
        Signal("onSizeChanged", &nu::View::on_size_changed),
        Signal("onFocusIn", &nu::View::on_focus_in),
        Signal("onFocusOut", &nu::View::on_focus_out),
        Delegate("handleDragEnter", &nu::View::handle_drag_enter),
        Delegate("handleDragUpdate", &nu::View::handle_drag_update),
        Delegate("handleDrop", &nu::View::handle_drop));
  }
  static void SetStyle(Arguments args,
                       const std::map<std::string, napi_value>& styles) {
    nu::View* view;
    if (!args.GetThis(&view))
      return;
    for (const auto& it : styles) {
      float number;
      if (FromNode(args.Env(), it.second, &number))
        view->SetStyleProperty(it.first, number);
      else
        view->SetStyleProperty(it.first,
                               FromNodeTo<std::string>(args.Env(), it.second));
    }
    view->Layout();
  }
};

template<>
struct Type<nu::Window::Options> {
  static constexpr const char* name = "WindowOptions";
  static napi_status FromNode(napi_env env,
                              napi_value value,
                              nu::Window::Options* out) {
    if (!ReadOptions(env, value,
                     "frame", &out->frame,
#if defined(OS_MAC)
                     "showTrafficLights", &out->show_traffic_lights,
#endif
                     "transparent", &out->transparent))
      return napi_invalid_arg;
    return napi_ok;
  }
};

template<>
struct Type<nu::Window> {
  using Base = nu::Responder;
  static constexpr const char* name = "Window";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, constructor,
        "create", &CreateOnHeap<nu::Window, nu::Window::Options>);
    Set(env, prototype,
        "close", &nu::Window::Close,
        "setHasShadow", &nu::Window::SetHasShadow,
        "hasShadow", &nu::Window::HasShadow,
        "center", &nu::Window::Center,
        "setContentView",
        WrapMethod(&nu::Window::SetContentView, [](Arguments args) {
          AttachedTable(args).Set("contentView", args[0]);
        }),
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
        "setToolbar",
        WrapMethod(&nu::Window::SetToolbar, [](Arguments args) {
          AttachedTable(args).Set("toolbar", args[0]);
        }),
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
        WrapMethod(&nu::Window::SetMenuBar, [](Arguments args) {
          AttachedTable(args).Set("menuBar", args[0]);
        }),
        "getMenuBar", &nu::Window::GetMenuBar,
        "setMenuBarVisible", &nu::Window::SetMenuBarVisible,
#endif
        "addChildWindow",
        WrapMethod(&nu::Window::AddChildWindow, [](Arguments args) {
          AttachedTable(args).Set(args[0], true);
        }),
        "removeChildView",
        WrapMethod(&nu::Window::RemoveChildWindow, [](Arguments args) {
          AttachedTable(args).Delete(args[0]);
        }),
        "getChildWindows", &nu::Window::GetChildWindows);
    DefineProperties(env, prototype,
                     Signal("onClose", &nu::Window::on_close),
                     Signal("onFocus", &nu::Window::on_focus),
                     Signal("onBlur", &nu::Window::on_blur),
                     Delegate("shouldClose", &nu::Window::should_close));
  }
};

}  // namespace ki

namespace napi_yue {

bool GetRuntime(napi_env env, bool* is_electron, bool* is_yode) {
  napi_value global;
  if (napi_get_global(env, &global) != napi_ok)
    return false;
  napi_value process;
  if (!ki::Get(env, global, "process", &process) ||
      !ki::IsType(env, process, napi_object))
    return false;
  napi_value versions;
  if (!ki::Get(env, process, "versions", &versions) ||
      !ki::IsType(env, versions, napi_object))
    return false;
  std::string tmp;
  *is_electron = ki::Get(env, versions, "electron", &tmp) && !tmp.empty();
  *is_yode =  ki::Get(env, versions, "yode", &tmp) && !tmp.empty();
  return true;
}

void AddFinalizer(napi_env env, napi_value object,
                  std::function<void()> callback) {
  auto holder = std::make_unique<std::function<void()>>(std::move(callback));
  napi_status s = napi_add_finalizer(env, object, holder.get(),
                                     [](napi_env, void* ptr, void*) {
    auto* func = static_cast<std::function<void()>*>(ptr);
    (*func)();
    delete func;
  }, nullptr, nullptr);
  if (s != napi_ok)
    return;
  holder.release();
}

napi_value GetAttachedTable(napi_env env, napi_value value) {
  return ki::AttachedTable(env, value).Value();
}

napi_value Initialize(napi_env env, napi_value exports) {
  CHECK(GetRuntime(env, &is_electron, &is_yode));

  std::unique_ptr<base::Environment> sys_env = base::Environment::Create();
#if defined(OS_LINUX)
  // Both node and WebKit are using SIGUSR1, avoid conflict.
  if (!sys_env->HasVar("JSC_SIGNAL_FOR_GC"))
    sys_env->SetVar("JSC_SIGNAL_FOR_GC", base::NumberToString(SIGUSR2));
#endif

#if defined(OS_WIN)
  if (!is_electron && !sys_env->HasVar("CI")) {
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

  ki::Set(env, exports,
          // Classes.
          "App",                ki::Class<nu::App>(),
          "Appearance",         ki::Class<nu::Appearance>(),
          "AttributedText",     ki::Class<nu::AttributedText>(),
          "Browser",            ki::Class<nu::Browser>(),
          "Button",             ki::Class<nu::Button>(),
          "Canvas",             ki::Class<nu::Canvas>(),
          "Clipboard",          ki::Class<nu::Clipboard>(),
          "Color",              ki::Class<nu::Color>(),
          "ComboBox",           ki::Class<nu::ComboBox>(),
          "Container",          ki::Class<nu::Container>(),
          "Cursor",             ki::Class<nu::Cursor>(),
          "DatePicker",         ki::Class<nu::DatePicker>(),
          "DraggingInfo",       ki::Class<nu::DraggingInfo>(),
          "Entry",              ki::Class<nu::Entry>(),
          "Event",              ki::Class<nu::Event>(),
          "FileDialog",         ki::Class<nu::FileDialog>(),
          "FileOpenDialog",     ki::Class<nu::FileOpenDialog>(),
          "FileSaveDialog",     ki::Class<nu::FileSaveDialog>(),
          "Font",               ki::Class<nu::Font>(),
          "GifPlayer",          ki::Class<nu::GifPlayer>(),
          "GlobalShortcut",     ki::Class<nu::GlobalShortcut>(),
          "Group",              ki::Class<nu::Group>(),
          "Image",              ki::Class<nu::Image>(),
          "Label",              ki::Class<nu::Label>(),
          "Locale",             ki::Class<nu::Locale>(),
          "MessageBox",         ki::Class<nu::MessageBox>(),
          "MenuBar",            ki::Class<nu::MenuBar>(),
          "Menu",               ki::Class<nu::Menu>(),
          "MenuItem",           ki::Class<nu::MenuItem>(),
          "Notification",       ki::Class<nu::Notification>(),
          "NotificationCenter", ki::Class<nu::NotificationCenter>(),
          "Painter",            ki::Class<nu::Painter>(),
          "Picker",             ki::Class<nu::Picker>(),
          "ProgressBar",        ki::Class<nu::ProgressBar>(),
          "ProtocolAsarJob",    ki::Class<nu::ProtocolAsarJob>(),
          "ProtocolFileJob",    ki::Class<nu::ProtocolFileJob>(),
          "ProtocolStringJob",  ki::Class<nu::ProtocolStringJob>(),
          "Responder",          ki::Class<nu::Responder>(),
          "Screen",             ki::Class<nu::Screen>(),
          "Scroll",             ki::Class<nu::Scroll>(),
          "Separator",          ki::Class<nu::Separator>(),
          "Slider",             ki::Class<nu::Slider>(),
          "Tab",                ki::Class<nu::Tab>(),
          "TableModel",         ki::Class<nu::TableModel>(),
          "AbstractTableModel", ki::Class<nu::AbstractTableModel>(),
          "SimpleTableModel",   ki::Class<nu::SimpleTableModel>(),
          "Table",              ki::Class<nu::Table>(),
          "TextEdit",           ki::Class<nu::TextEdit>(),
#if defined(OS_MAC)
          "Toolbar",            ki::Class<nu::Toolbar>(),
#endif
          "Tray",               ki::Class<nu::Tray>(),
#if defined(OS_MAC)
          "Vibrant",            ki::Class<nu::Vibrant>(),
#endif
          "View",               ki::Class<nu::View>(),
          "Window",             ki::Class<nu::Window>(),
          // Properties.
          "app",                nu::App::GetCurrent(),
          "appearance",         nu::Appearance::GetCurrent(),
          "globalShortcut",     nu::GlobalShortcut::GetCurrent(),
          "notificationCenter", nu::NotificationCenter::GetCurrent(),
          "screen",             nu::Screen::GetCurrent(),
          // Helper functions.
          "addFinalizer",       &AddFinalizer,
          "getAttachedTable",   &GetAttachedTable);

  if (!is_electron) {
    ki::Set(env, exports,
            "Lifetime", ki::Class<nu::Lifetime>(),
            "MessageLoop", ki::Class<nu::MessageLoop>(),
            "lifetime", nu::Lifetime::GetCurrent());
  }

  return exports;
}

}  // namespace napi_yue

NAPI_MODULE(NODE_GYP_MODULE_NAME, napi_yue::Initialize)
