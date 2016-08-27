// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/gtk/platform_font_gtk.h"

#include "nativeui/label.h"

namespace nu {

PlatformFontGtk::PlatformFontGtk(PangoFontDescription* font)
    : font_(pango_font_description_copy(font)) {
}

PlatformFontGtk::PlatformFontGtk(const std::string& font_name, int font_size)
    : font_(pango_font_description_new()) {
  pango_font_description_set_family(font_, font_name.c_str());
  pango_font_description_set_size(font_, font_size);
}

PlatformFontGtk::~PlatformFontGtk() {
  pango_font_description_free(font_);
}

std::string PlatformFontGtk::GetFontName() const {
  return pango_font_description_get_family(font_);
}

int PlatformFontGtk::GetFontSize() const {
  return pango_font_description_get_size(font_);
}

// static
PlatformFont* PlatformFont::CreateDefault() {
  // Receive the default font from control.
  scoped_refptr<nu::Label> label = new nu::Label;
  gtk_widget_ensure_style(label->view());
  GtkStyle* style = gtk_widget_get_style(label->view());
  return new PlatformFontGtk(style->font_desc);
}

// static
PlatformFont* PlatformFont::CreateFromNameAndSize(const std::string& font_name,
                                                  int font_size) {
  return new PlatformFontGtk(font_name, font_size);
}

}  // namespace nu
