// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/label.h"

namespace nu {

Label::Label(const std::string& text) {
  set_view(gtk_label_new(text.c_str()));
  g_object_ref_sink(view());
  gtk_widget_show(view());
}

Label::~Label() {
}

void Label::SetText(const std::string& text) {
  gtk_label_set_text(GTK_LABEL(view()), text.c_str());
}

std::string Label::GetText() {
  return gtk_label_get_text(GTK_LABEL(view()));
}

}  // namespace nu
