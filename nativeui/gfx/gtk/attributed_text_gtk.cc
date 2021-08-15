// Copyright 2020 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/attributed_text.h"

#include <math.h>

#include <gtk/gtk.h>
#include <pango/pango.h>

#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "nativeui/gfx/font.h"
#include "nativeui/gfx/geometry/rect_f.h"
#include "nativeui/gfx/geometry/size_f.h"
#include "nativeui/gfx/text.h"

namespace nu {

namespace {

// Convert the character index to byte index.
guint CharIndexToByteIndex(const std::u16string& text, int index) {
  if (index < 0)
    return G_MAXUINT;
  return base::UTF16ToUTF8(text.substr(0, index)).size();
}

void FillPangoAttributeIndex(PangoAttribute* attr, PangoLayout* layout,
                             int start, int end) {
  if (start == 0 && end < 0)  // this is the most common case
    return;
  std::u16string text = base::UTF8ToUTF16(pango_layout_get_text(layout));
  attr->start_index = CharIndexToByteIndex(text, start);
  attr->end_index = CharIndexToByteIndex(text, end);
}

// Find and remove the attribute of type.
gboolean FilterAttributeType(PangoAttribute* attr, PangoAttrType type) {
  return attr->klass->type == type;
}

void RemoveFromAttributeList(PangoAttrList* attrs, PangoAttrType type) {
  PangoAttrList* removed = pango_attr_list_filter(
      attrs,
      reinterpret_cast<PangoAttrFilterFunc>(FilterAttributeType),
      reinterpret_cast<gpointer>(type));
  if (removed)
    pango_attr_list_unref(removed);
}

}  // namespace

AttributedText::AttributedText(const std::string& text, TextAttributes att) {
  static PangoContext* context = nullptr;
  if (!context) {
    context = gdk_pango_context_get_for_screen(gdk_screen_get_default());
    pango_context_set_language(context, pango_language_get_default());
  }
  text_ = pango_layout_new(context);

  // Set text.
  pango_layout_set_text(text_, text.c_str(), text.size());

  // Set attributes.
  PangoAttrList* attrs = pango_attr_list_new();
  pango_layout_set_attributes(text_, attrs);
  pango_attr_list_unref(attrs);

  // Set format.
  SetFormat(att.ToTextFormat());
  SetFont(att.font.get());
  SetColor(att.color);
}

AttributedText::~AttributedText() {
  g_object_unref(text_);
}

void AttributedText::PlatformUpdateFormat() {
  pango_layout_set_ellipsize(text_,
                             format_.ellipsis ? PANGO_ELLIPSIZE_END
                                              : PANGO_ELLIPSIZE_NONE);
  if (format_.align == TextAlign::Start)
    pango_layout_set_alignment(text_, PANGO_ALIGN_LEFT);
  else if (format_.align == TextAlign::Center)
    pango_layout_set_alignment(text_, PANGO_ALIGN_CENTER);
  else
    pango_layout_set_alignment(text_, PANGO_ALIGN_RIGHT);

  if (format_.wrap)
    pango_layout_set_wrap(text_, PANGO_WRAP_WORD_CHAR);
  else
    pango_layout_set_width(text_, -1);
}

void AttributedText::PlatformSetFontFor(scoped_refptr<Font> font,
                                        int start, int end) {
  PangoAttrList* attrs = pango_layout_get_attributes(text_);
  if (start == 0 && end == -1)
    RemoveFromAttributeList(attrs, PANGO_ATTR_FONT_DESC);

  PangoAttribute* font_attr = pango_attr_font_desc_new(font->GetNative());
  FillPangoAttributeIndex(font_attr, text_, start, end);
  pango_attr_list_insert(attrs, font_attr);  // ownership taken
}

void AttributedText::PlatformSetColorFor(Color color, int start, int end) {
  PangoAttrList* attrs = pango_layout_get_attributes(text_);
  if (start == 0 && end == -1)
    RemoveFromAttributeList(attrs, PANGO_ATTR_FOREGROUND);

  PangoAttribute* fg_attr = pango_attr_foreground_new(
      color.r() / 255. * 65535,
      color.g() / 255. * 65535,
      color.b() / 255. * 65535);
  FillPangoAttributeIndex(fg_attr, text_, start, end);
  pango_attr_list_insert(attrs, fg_attr);  // ownership taken
}

RectF AttributedText::GetBoundsFor(const SizeF& size) const {
  if (format_.wrap) {
    // Yoga may pass 0 as width to indicate no wrapping.
    if (size.width() == 0 || isnan(size.width()))
      pango_layout_set_width(text_, -1);
    else
      pango_layout_set_width(text_, size.width() * PANGO_SCALE);
    pango_layout_set_height(text_, size.height() * PANGO_SCALE);
  }
  int width, height;
  pango_layout_get_pixel_size(text_, &width, &height);
  return RectF(0, 0, width, height);
}

void AttributedText::SetText(const std::string& text) {
  pango_layout_set_text(text_, text.c_str(), text.length());
}

std::string AttributedText::GetText() const {
  return pango_layout_get_text(text_);
}

}  // namespace nu
