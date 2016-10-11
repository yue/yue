// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/gtk/font_gtk.h"

#include "nativeui/label.h"

namespace nu {

FontGtk::FontGtk(PangoFontDescription* font)
    : font_(pango_font_description_copy(font)) {
}

FontGtk::FontGtk(const std::string& font_name, int font_size)
    : font_(pango_font_description_new()) {
  pango_font_description_set_family(font_, font_name.c_str());
  pango_font_description_set_size(font_, font_size);
}

FontGtk::~FontGtk() {
  pango_font_description_free(font_);
}

std::string FontGtk::GetName() const {
  return pango_font_description_get_family(font_);
}

int FontGtk::GetSize() const {
  return pango_font_description_get_size(font_);
}

NativeFont FontGtk::GetNative() const {
  return font_;
}

// static
Font* Font::CreateDefault() {
  // Receive the default font from control.
  scoped_refptr<nu::Label> label = new nu::Label;
  gtk_widget_ensure_style(label->view());
  GtkStyle* style = gtk_widget_get_style(label->view());
  return new FontGtk(style->font_desc);
}

// static
Font* Font::CreateFromNameAndSize(const std::string& font_name,
                                                  int font_size) {
  return new FontGtk(font_name, font_size);
}

}  // namespace nu
