// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/font.h"

#include "nativeui/label.h"

namespace nu {

namespace {

PangoFontDescription* GetDefaultFontDescription() {
  // Receive the default font from control.
  scoped_refptr<nu::Label> label = new nu::Label;
  gtk_widget_ensure_style(label->view());
  GtkStyle* style = gtk_widget_get_style(label->view());
  return pango_font_description_copy(style->font_desc);
}

}  // namespace

Font::Font() : font_(GetDefaultFontDescription()) {
}

Font::Font(const std::string& font_name, int font_size)
    : font_(pango_font_description_new()) {
  pango_font_description_set_family(font_, font_name.c_str());
  pango_font_description_set_size(font_, font_size);
}

Font::~Font() {
  pango_font_description_free(font_);
}

std::string Font::GetName() const {
  return pango_font_description_get_family(font_);
}

int Font::GetSize() const {
  return pango_font_description_get_size(font_);
}

NativeFont Font::GetNative() const {
  return font_;
}

}  // namespace nu
