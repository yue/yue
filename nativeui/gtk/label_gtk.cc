// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/label.h"

#include <gtk/gtk.h>

#include "nativeui/gtk/widget_util.h"

namespace nu {

Label::Label(const std::string& text) {
  GtkWidget* event_box = gtk_event_box_new();
  GtkWidget* label = gtk_label_new(text.c_str());
  gtk_widget_show(label);
  gtk_container_add(GTK_CONTAINER(event_box), label);
  TakeOverView(event_box);
  UpdateDefaultStyle();
}

Label::~Label() {
}

void Label::PlatformSetText(const std::string& text) {
  auto* label = GTK_LABEL(gtk_bin_get_child(GTK_BIN(GetNative())));
  gtk_label_set_text(label, text.c_str());
}

std::string Label::GetText() const {
  auto* label = GTK_LABEL(gtk_bin_get_child(GTK_BIN(GetNative())));
  return gtk_label_get_text(label);
}

void Label::SetAlign(TextAlign align) {
  auto* label = GTK_LABEL(gtk_bin_get_child(GTK_BIN(GetNative())));
  gfloat xalign, yalign;
  gtk_misc_get_alignment(GTK_MISC(label), &xalign, &yalign);
  if (align == TextAlign::Start)
    xalign = 0.f;
  else if (align == TextAlign::End)
    xalign = 1.f;
  else
    xalign = 0.5f;
  gtk_misc_set_alignment(GTK_MISC(label), xalign, yalign);
}

void Label::SetVAlign(TextAlign align) {
  auto* label = GTK_LABEL(gtk_bin_get_child(GTK_BIN(GetNative())));
  gfloat xalign, yalign;
  gtk_misc_get_alignment(GTK_MISC(label), &xalign, &yalign);
  if (align == TextAlign::Start)
    yalign = 0.f;
  else if (align == TextAlign::End)
    yalign = 1.f;
  else
    yalign = 0.5f;
  gtk_misc_set_alignment(GTK_MISC(label), xalign, yalign);
}

SizeF Label::GetMinimumSize() const {
  return GetPreferredSizeForWidget(GetNative());
}

}  // namespace nu
