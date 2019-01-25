// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/label.h"

#include <gtk/gtk.h>

#include "nativeui/gtk/widget_util.h"

namespace nu {

Label::Label(const std::string& text) {
  TakeOverView(gtk_label_new(text.c_str()));
  UpdateDefaultStyle();
}

Label::~Label() {
}

void Label::PlatformSetText(const std::string& text) {
  gtk_label_set_text(GTK_LABEL(GetNative()), text.c_str());
}

std::string Label::GetText() const {
  return gtk_label_get_text(GTK_LABEL(GetNative()));
}

void Label::SetAlign(TextAlign align) {
  gfloat xalign, yalign;
  gtk_misc_get_alignment(GTK_MISC(GetNative()), &xalign, &yalign);
  if (align == TextAlign::Start)
    xalign = 0.f;
  else if (align == TextAlign::End)
    xalign = 1.f;
  else
    xalign = 0.5f;
  gtk_misc_set_alignment(GTK_MISC(GetNative()), xalign, yalign);
}

void Label::SetVAlign(TextAlign align) {
  gfloat xalign, yalign;
  gtk_misc_get_alignment(GTK_MISC(GetNative()), &xalign, &yalign);
  if (align == TextAlign::Start)
    yalign = 0.f;
  else if (align == TextAlign::End)
    yalign = 1.f;
  else
    yalign = 0.5f;
  gtk_misc_set_alignment(GTK_MISC(GetNative()), xalign, yalign);
}

SizeF Label::GetMinimumSize() const {
  return GetPreferredSizeForWidget(GetNative());
}

}  // namespace nu
