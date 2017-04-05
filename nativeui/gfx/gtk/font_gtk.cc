// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/font.h"

#include <gtk/gtk.h>

#include "nativeui/label.h"

namespace nu {

namespace {

PangoFontDescription* GetDefaultFontDescription() {
  // Receive the default font from control.
  scoped_refptr<nu::Label> label = new nu::Label;
  gtk_widget_ensure_style(label->GetNative());
  GtkStyle* style = gtk_widget_get_style(label->GetNative());
  return pango_font_description_copy(style->font_desc);
}

}  // namespace

Font::Font() : font_(GetDefaultFontDescription()) {
}

Font::Font(base::StringPiece name, float size)
    : font_(pango_font_description_new()) {
  pango_font_description_set_family(font_, name.data());
  pango_font_description_set_absolute_size(font_, size * PANGO_SCALE);
}

Font::~Font() {
  pango_font_description_free(font_);
}

std::string Font::GetName() const {
  return pango_font_description_get_family(font_);
}

float Font::GetSize() const {
  return pango_font_description_get_size(font_) / PANGO_SCALE;
}

NativeFont Font::GetNative() const {
  return font_;
}

}  // namespace nu
