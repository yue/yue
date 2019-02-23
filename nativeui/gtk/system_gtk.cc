// Copyright 2019 Cheng Zhao. All rights reserved.
// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#include "nativeui/system.h"

#include <dlfcn.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include "base/strings/string_split.h"
#include "base/strings/string_tokenizer.h"
#include "base/strings/string_util.h"
#include "nativeui/gtk/widget_util.h"

#if GTK_MAJOR_VERSION > 2
// These constants are defined in gtk/gtkenums.h in Gtk3.12 or later.
// They are added here as a convenience to avoid version checks, and
// can be removed once the sysroot is switched from Wheezy to Jessie.
#define GTK_STATE_FLAG_LINK static_cast<GtkStateFlags>(1 << 9)
#define GTK_STATE_FLAG_VISITED static_cast<GtkStateFlags>(1 << 10)
#define GTK_STATE_FLAG_CHECKED static_cast<GtkStateFlags>(1 << 11)
#endif

namespace nu {

namespace {

void* GetGtkSharedLibrary() {
  std::string lib_name =
      "libgtk-" + std::to_string(GTK_MAJOR_VERSION) + ".so.0";
  static void* gtk_lib = dlopen(lib_name.c_str(), RTLD_LAZY);
  DCHECK(gtk_lib);
  return gtk_lib;
}

GtkStyleContext* AppendCssNodeToStyleContext(GtkStyleContext* context,
                                             const std::string& css_node) {
  GtkWidgetPath* path =
      context ? gtk_widget_path_copy(gtk_style_context_get_path(context))
              : gtk_widget_path_new();

  enum {
    CSS_TYPE,
    CSS_NAME,
    CSS_CLASS,
    CSS_PSEUDOCLASS,
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
  base::StringTokenizer t(css_node, ".:#");
  t.set_options(base::StringTokenizer::RETURN_DELIMS);
  while (t.GetNext()) {
    if (t.token_is_delim()) {
      if (t.token_begin() == css_node.begin()) {
        // Special case for the first token.
        gtk_widget_path_append_type(path, G_TYPE_NONE);
      }
      switch (*t.token_begin()) {
        case '#':
          part_type = CSS_NAME;
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
      static auto* _gtk_widget_path_iter_set_object_name =
          reinterpret_cast<void (*)(GtkWidgetPath*, gint, const char*)>(dlsym(
              GetGtkSharedLibrary(), "gtk_widget_path_iter_set_object_name"));
      switch (part_type) {
        case CSS_NAME: {
          if (GtkVersionCheck(3, 20)) {
            _gtk_widget_path_iter_set_object_name(path, -1, t.token().c_str());
          } else {
            gtk_widget_path_iter_add_class(path, -1, t.token().c_str());
          }
          break;
        }
        case CSS_TYPE: {
          GType type = g_type_from_name(t.token().c_str());
          DCHECK(type);
          gtk_widget_path_append_type(path, type);
          if (GtkVersionCheck(3, 20)) {
            if (t.token() == "GtkLabel")
              _gtk_widget_path_iter_set_object_name(path, -1, "label");
          }
          break;
        }
        case CSS_CLASS: {
          gtk_widget_path_iter_add_class(path, -1, t.token().c_str());
          break;
        }
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
      }
    }
  }

  if (GtkVersionCheck(3, 14)) {
    static auto* _gtk_widget_path_iter_set_state =
        reinterpret_cast<void (*)(GtkWidgetPath*, gint, GtkStateFlags)>(
            dlsym(GetGtkSharedLibrary(), "gtk_widget_path_iter_set_state"));
    DCHECK(_gtk_widget_path_iter_set_state);
    _gtk_widget_path_iter_set_state(path, -1, state);
  }

  GtkStyleContext* child_context = gtk_style_context_new();
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

GtkStyleContext* GetStyleContextFromCss(const std::string& css_selector) {
  // Prepend a window node to the selector since all widgets must live
  // in a window, but we don't want to specify that every time.
  auto* context =
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
  gtk_style_context_get_color(context, gtk_style_context_get_state(context),
                              &color);
  return Color(color.alpha * 255, color.red * 255, color.green * 255,
               color.blue * 255);
}

Color GetFgColor(const std::string& css_selector) {
  GtkStyleContext* context = GetStyleContextFromCss(css_selector);
  Color color = GetFgColorFromStyleContext(context);
  g_object_unref(context);
  return color;
}

}  // namespace

Color System::GetColor(System::Color name) {
  switch (name) {
    case System::Color::Text:
      return GetFgColor("GtkLabel");
    case System::Color::DisabledText:
      return GetFgColor("GtkLabel:disabled");
    default:
      NOTREACHED() << "Unkown color name: " << static_cast<int>(name);
      return nu::Color();
  }
}

}  // namespace nu
