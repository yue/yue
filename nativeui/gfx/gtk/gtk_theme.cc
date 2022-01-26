// Copyright 2020 Cheng Zhao. All rights reserved.
// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/gtk/gtk_theme.h"

#include <dlfcn.h>
#include <gdk/gdk.h>

#include <algorithm>
#include <string>

#include "base/notreached.h"
#include "base/strings/string_split.h"
#include "base/strings/string_tokenizer.h"
#include "base/strings/string_util.h"
#include "nativeui/gtk/util/scoped_gobject.h"
#include "nativeui/gtk/util/widget_util.h"

#if GTK_MAJOR_VERSION > 2
// These constants are defined in gtk/gtkenums.h in Gtk3.12 or later.
// They are added here as a convenience to avoid version checks, and
// can be removed once the sysroot is switched from Wheezy to Jessie.
#define GTK_STATE_FLAG_LINK static_cast<GtkStateFlags>(1 << 9)
#define GTK_STATE_FLAG_VISITED static_cast<GtkStateFlags>(1 << 10)
#define GTK_STATE_FLAG_CHECKED static_cast<GtkStateFlags>(1 << 11)
#endif

// Function availability can be tested by checking if the address of gtk_* is
// not nullptr.
#define WEAK_GTK_FN(x) extern "C" __attribute__((weak)) decltype(x) x

WEAK_GTK_FN(gtk_widget_path_iter_set_object_name);
WEAK_GTK_FN(gtk_widget_path_iter_set_state);

namespace nu {

using ScopedStyleContext = ScopedGObject<GtkStyleContext>;
using ScopedCssProvider = ScopedGObject<GtkCssProvider>;

template <>
inline void ScopedStyleContext::Unref() {
  // Versions of GTK earlier than 3.15.4 had a bug where a g_assert
  // would be triggered when trying to free a GtkStyleContext that had
  // a parent whose only reference was the child context in question.
  // This is a hack to work around that case.  See GTK commit
  // "gtkstylecontext: Don't try to emit a signal when finalizing".
  GtkStyleContext* context = obj_;
  while (context) {
    GtkStyleContext* parent = gtk_style_context_get_parent(context);
    if (parent && G_OBJECT(context)->ref_count == 1 &&
        !GtkVersionCheck(3, 15, 4)) {
      g_object_ref(parent);
      gtk_style_context_set_parent(context, nullptr);
      g_object_unref(context);
    } else {
      g_object_unref(context);
      return;
    }
    context = parent;
  }
}

namespace {

class CairoSurface {
 public:
  // Creates a new cairo surface with the given size.  The memory for
  // this surface is deallocated when this CairoSurface is destroyed.
  explicit CairoSurface(const Size& size)
      : surface_(cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
                                            size.width(),
                                            size.height())),
        cairo_(cairo_create(surface_)) {
    DCHECK(cairo_surface_status(surface_) == CAIRO_STATUS_SUCCESS);
    // Clear the surface.
    cairo_save(cairo_);
    cairo_set_source_rgba(cairo_, 0, 0, 0, 0);
    cairo_set_operator(cairo_, CAIRO_OPERATOR_SOURCE);
    cairo_paint(cairo_);
    cairo_restore(cairo_);
  }

  ~CairoSurface() {
    cairo_destroy(cairo_);
    cairo_surface_destroy(surface_);
  }

  // Get the drawing context for GTK to use.
  cairo_t* cairo() { return cairo_; }

  // Returns the average of all pixels in the surface.  If |frame| is
  // true, the resulting alpha will be the average alpha, otherwise it
  // will be the max alpha across all pixels.
  Color GetAveragePixelValue(bool frame) {
    cairo_surface_flush(surface_);
    uint32_t* data =
        reinterpret_cast<uint32_t*>(cairo_image_surface_get_data(surface_));
    int width = cairo_image_surface_get_width(surface_);
    int height = cairo_image_surface_get_height(surface_);
    DCHECK(4 * width == cairo_image_surface_get_stride(surface_));
    int32_t a = 0, r = 0, g = 0, b = 0;
    unsigned int max_alpha = 0;
    for (int i = 0; i < width * height; i++) {
      Color color(data[i]);
      max_alpha = std::max(color.a(), max_alpha);
      a += color.a();
      r += color.r();
      g += color.g();
      b += color.b();
    }
    if (a == 0)
      return Color();
    return Color(frame ? max_alpha : a / (width * height),
                 r * 255 / a, g * 255 / a, b * 255 / a);
  }

 private:
  cairo_surface_t* surface_;
  cairo_t* cairo_;
};

ScopedStyleContext AppendCssNodeToStyleContext(GtkStyleContext* context,
                                               const std::string& css_node) {
  GtkWidgetPath* path =
      context ? gtk_widget_path_copy(gtk_style_context_get_path(context))
              : gtk_widget_path_new();

  enum {
    CSS_TYPE,
    CSS_NAME,
    CSS_OBJECT_NAME,
    CSS_CLASS,
    CSS_PSEUDOCLASS,
    CSS_NONE,
  } part_type = CSS_TYPE;
  static const struct {
    const char* name;
    GtkStateFlags state_flag;
  } pseudo_classes[] = {
      {"active", GTK_STATE_FLAG_ACTIVE},
      {"hover", GTK_STATE_FLAG_PRELIGHT},
      {"selected", GTK_STATE_FLAG_SELECTED},
      {"disabled", GTK_STATE_FLAG_INSENSITIVE},
      {"indeterminate", GTK_STATE_FLAG_INCONSISTENT},
      {"focus", GTK_STATE_FLAG_FOCUSED},
      {"backdrop", GTK_STATE_FLAG_BACKDROP},
      {"link", GTK_STATE_FLAG_LINK},
      {"visited", GTK_STATE_FLAG_VISITED},
      {"checked", GTK_STATE_FLAG_CHECKED},
  };
  GtkStateFlags state = GTK_STATE_FLAG_NORMAL;
  base::StringTokenizer t(css_node, ".:#()");
  t.set_options(base::StringTokenizer::RETURN_DELIMS);
  while (t.GetNext()) {
    if (t.token_is_delim()) {
      if (t.token_begin() == css_node.begin()) {
        // Special case for the first token.
        gtk_widget_path_append_type(path, G_TYPE_NONE);
      }
      switch (*t.token_begin()) {
        case '(':
          part_type = CSS_NAME;
          break;
        case ')':
          part_type = CSS_NONE;
          break;
        case '#':
          part_type = CSS_OBJECT_NAME;
          break;
        case '.':
          part_type = CSS_CLASS;
          break;
        case ':':
          part_type = CSS_PSEUDOCLASS;
          break;
        default:
          NOTREACHED();
      }
    } else {
      switch (part_type) {
        case CSS_NAME:
          gtk_widget_path_iter_set_name(path, -1, t.token().c_str());
          break;
        case CSS_OBJECT_NAME:
          if (GtkVersionCheck(3, 20)) {
            DCHECK(gtk_widget_path_iter_set_object_name);
            gtk_widget_path_iter_set_object_name(path, -1, t.token().c_str());
          } else {
            gtk_widget_path_iter_add_class(path, -1, t.token().c_str());
          }
          break;
        case CSS_TYPE: {
          GType type = g_type_from_name(t.token().c_str());
          gtk_widget_path_append_type(path, type);
          if (GtkVersionCheck(3, 20) && t.token() == "GtkLabel") {
            DCHECK(gtk_widget_path_iter_set_object_name);
            gtk_widget_path_iter_set_object_name(path, -1, "label");
          }
          break;
        }
        case CSS_CLASS:
          gtk_widget_path_iter_add_class(path, -1, t.token().c_str());
          break;
        case CSS_PSEUDOCLASS: {
          GtkStateFlags state_flag = GTK_STATE_FLAG_NORMAL;
          for (const auto& pseudo_class_entry : pseudo_classes) {
            if (strcmp(pseudo_class_entry.name, t.token().c_str()) == 0) {
              state_flag = pseudo_class_entry.state_flag;
              break;
            }
          }
          state = static_cast<GtkStateFlags>(state | state_flag);
          break;
        }
        case CSS_NONE:
          NOTREACHED();
      }
    }
  }

  if (GtkVersionCheck(3, 14)) {
    DCHECK(gtk_widget_path_iter_set_state);
    gtk_widget_path_iter_set_state(path, -1, state);
  }

  ScopedStyleContext child_context(gtk_style_context_new());
  gtk_style_context_set_path(child_context, path);
  if (GtkVersionCheck(3, 14)) {
    gtk_style_context_set_state(child_context, state);
  } else {
    GtkStateFlags child_state = state;
    if (context) {
      child_state = static_cast<GtkStateFlags>(
          child_state | gtk_style_context_get_state(context));
    }
    gtk_style_context_set_state(child_context, child_state);
  }
  gtk_style_context_set_parent(child_context, context);
  gtk_widget_path_unref(path);
  return child_context;
}

ScopedStyleContext GetStyleContextFromCss(const std::string& css_selector) {
  // Prepend a window node to the selector since all widgets must live
  // in a window, but we don't want to specify that every time.
  auto context =
      AppendCssNodeToStyleContext(nullptr, "GtkWindow#window.background");

  for (const auto& widget_type :
       base::SplitString(css_selector, base::kWhitespaceASCII,
                         base::TRIM_WHITESPACE, base::SPLIT_WANT_NONEMPTY)) {
    context = AppendCssNodeToStyleContext(context, widget_type);
  }
  return context;
}

Color GetFgColorFromStyleContext(GtkStyleContext* context) {
  GdkRGBA color;
#if GTK_CHECK_VERSION(3, 90, 0)
  gtk_style_context_get_color(context, &color);
#else
  gtk_style_context_get_color(context, gtk_style_context_get_state(context),
                              &color);
#endif
  return Color(color.alpha * 255, color.red * 255, color.green * 255,
               color.blue * 255);
}

Color GetFgColor(const std::string& css_selector) {
  return GetFgColorFromStyleContext(GetStyleContextFromCss(css_selector));
}

void ApplyCssProviderToContext(GtkStyleContext* context,
                               GtkCssProvider* provider) {
  while (context) {
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider),
                                   G_MAXUINT);
    context = gtk_style_context_get_parent(context);
  }
}

ScopedCssProvider GetCssProvider(const std::string& css) {
  GtkCssProvider* provider = gtk_css_provider_new();
#if GTK_CHECK_VERSION(3, 90, 0)
  gtk_css_provider_load_from_data(provider, css.c_str(), -1);
#else
  GError* error = nullptr;
  gtk_css_provider_load_from_data(provider, css.c_str(), -1, &error);
  DCHECK(!error);
#endif
  return ScopedCssProvider(provider);
}

void ApplyCssToContext(GtkStyleContext* context, const std::string& css) {
  auto provider = GetCssProvider(css);
  ApplyCssProviderToContext(context, provider);
}

void RenderBackground(const Size& size, cairo_t* cr, GtkStyleContext* context) {
  if (!context)
    return;
  RenderBackground(size, cr, gtk_style_context_get_parent(context));
  gtk_render_background(context, cr, 0, 0, size.width(), size.height());
}

Color GetBgColorFromStyleContext(GtkStyleContext* context) {
  // Backgrounds are more general than solid colors (eg. gradients),
  // but chromium requires us to boil this down to one color.  We
  // cannot use the background-color here because some themes leave it
  // set to a garbage color because a background-image will cover it
  // anyway. So we instead render the background into a 24x24 bitmap,
  // removing any borders, and hope that we get a good color.
  ApplyCssToContext(context,
                    "* {"
                    "border-radius: 0px;"
                    "border-style: none;"
                    "box-shadow: none;"
                    "}");
  Size size(24, 24);
  CairoSurface surface(size);
  RenderBackground(size, surface.cairo(), context);
  return surface.GetAveragePixelValue(false);
}

Color GetBgColor(const std::string& css_selector) {
  return GetBgColorFromStyleContext(GetStyleContextFromCss(css_selector));
}

Color GetBorderColor(const std::string& css_selector) {
  // Borders have the same issue as backgrounds, due to the
  // border-image property.
  auto context = GetStyleContextFromCss(css_selector);
  Size size(24, 24);
  CairoSurface surface(size);
  gtk_render_frame(context, surface.cairo(), 0, 0, size.width(), size.height());
  return surface.GetAveragePixelValue(true);
}

}  // namespace

GtkTheme::GtkTheme() {
  GtkSettings* settings = gtk_settings_get_default();
  signal_gtk_theme_name_ = g_signal_connect(
      settings, "notify::gtk-theme-name",
      G_CALLBACK(OnThemeChange), this);
  signal_gtk_prefer_dark_theme_ = g_signal_connect(
      settings, "notify::gtk-application-prefer-dark-theme",
      G_CALLBACK(OnThemeChange), this);
}

GtkTheme::~GtkTheme() {
  GtkSettings* settings = gtk_settings_get_default();
  g_signal_handler_disconnect(settings, signal_gtk_theme_name_);
  g_signal_handler_disconnect(settings, signal_gtk_prefer_dark_theme_);
}

Color GtkTheme::GetColor(Color::Name name) {
  int key = static_cast<int>(name);
  auto it = colors_.find(key);
  if (it != colors_.end())
    return it->second;
  Color color;
  switch (name) {
    case Color::Name::Text:
      color = GetFgColor("GtkLabel");
      break;
    case Color::Name::DisabledText:
      color = GetFgColor("GtkLabel:disabled");
      break;
    case Color::Name::Control:
    case Color::Name::WindowBackground:
      color = GetBgColor("");
      break;
    case Color::Name::Border:
      color = GetBorderColor("GtkEntry#entry");
      break;
    default:
      NOTREACHED();
      return Color();
  }
  colors_[key] = color;
  return color;
}

// static
void GtkTheme::OnThemeChange(GtkSettings*, GParamSpec*, GtkTheme* self) {
  self->colors_.clear();
  self->on_theme_change.Emit();
}

}  // namespace nu
