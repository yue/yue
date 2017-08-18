// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/label.h"

#include <gtk/gtk.h>

#include "nativeui/gtk/widget_util.h"

namespace nu {

Label::Label(const std::string& text) {
  TakeOverView(gtk_label_new(text.c_str()));
  SetDefaultStyle(GetPreferredSizeForWidget(GetNative()));
  // Create GdkWindow for label, otherwise it can not receive input events.
  gtk_widget_set_has_window(GetNative(), true);
}

Label::~Label() {
}

void Label::SetText(const std::string& text) {
  gtk_label_set_text(GTK_LABEL(GetNative()), text.c_str());
  SetDefaultStyle(GetPreferredSizeForWidget(GetNative()));
}

std::string Label::GetText() {
  return gtk_label_get_text(GTK_LABEL(GetNative()));
}

}  // namespace nu
