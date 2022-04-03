// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/font.h"

#include <gtk/gtk.h>
#include <pango/pangofc-fontmap.h>

#include <iostream>

#include "nativeui/gtk/util/fontconfig.h"

namespace nu {

namespace {

PangoFontDescription* GetDefaultFontDescription() {
  GtkSettings* settings = gtk_settings_get_default();
  gchar* font_name = nullptr;
  g_object_get(settings, "gtk-font-name", &font_name, NULL);
  CHECK(font_name) << "Failed to get default font name";

  PangoFontDescription* desc = pango_font_description_from_string(font_name);
  g_free(font_name);
  return desc;
}

PangoFontDescription* FontDescriptionFromPath(const base::FilePath& path) {
  // Add the font path to FcConfig first.
  FcConfig* config = GetGlobalFontConfig();
  FcConfigAppFontAddFile(
      config, reinterpret_cast<const FcChar8*>(path.value().c_str()));
  FcFontSet* font_set = FcConfigGetFonts(config, FcSetApplication);
  // And then receive the FcPattern from FcConfig.
  if (font_set) {
    for (int i = 0; i < font_set->nfont; ++i) {
      FcPattern* pattern = font_set->fonts[i];
      if (GetFilename(pattern) == path.value())
        return pango_fc_font_description_from_pattern(pattern, FALSE);
    }
  }
  return GetDefaultFontDescription();
}

}  // namespace

Font::Font() : font_(GetDefaultFontDescription()) {
}

Font::Font(const std::string& name, float size, Weight weight, Style style)
    : font_(pango_font_description_new()) {
  pango_font_description_set_family(font_, name.data());
  pango_font_description_set_absolute_size(font_, size * PANGO_SCALE);
  pango_font_description_set_weight(font_, static_cast<PangoWeight>(weight));
  if (style == Style::Italic)
    pango_font_description_set_style(font_, PANGO_STYLE_ITALIC);
}

Font::Font(const base::FilePath& path, float size)
    : font_(FontDescriptionFromPath(path)) {
  pango_font_description_set_absolute_size(font_, size * PANGO_SCALE);
}

Font::~Font() {
  pango_font_description_free(font_);
}

std::string Font::GetName() const {
  return pango_font_description_get_family(font_);
}

float Font::GetSize() const {
  float size = pango_font_description_get_size(font_) / PANGO_SCALE;
  // Convert to aboslute size.
  if (!pango_font_description_get_size_is_absolute(font_))
    size = size * 96.f / 72.f;
  return size;
}

Font::Weight Font::GetWeight() const {
  return static_cast<Weight>(pango_font_description_get_weight(font_));
}

Font::Style Font::GetStyle() const {
  PangoStyle style = pango_font_description_get_style(font_);
  if (style == PANGO_STYLE_NORMAL)
    return Style::Normal;
  else
    return Style::Italic;
}

NativeFont Font::GetNative() const {
  return font_;
}

}  // namespace nu
