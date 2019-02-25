// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/label.h"

#include <gtk/gtk.h>

#include "nativeui/gfx/attributed_text.h"

namespace nu {

namespace {

inline float AlignToFloat(TextAlign align) {
  if (align == TextAlign::Start)
    return 0.f;
  else if (align == TextAlign::End)
    return 1.f;
  else
    return 0.5f;
}

}  // namespace

NativeView Label::PlatformCreate() {
  GtkWidget* event_box = gtk_event_box_new();
  GtkWidget* label = gtk_label_new(nullptr);
  gtk_widget_show(label);
  gtk_container_add(GTK_CONTAINER(event_box), label);
  return event_box;
}

void Label::PlatformSetAttributedText(AttributedText* text) {
  auto* label = GTK_LABEL(gtk_bin_get_child(GTK_BIN(GetNative())));

  // Text and styles.
  PangoLayout* layout = text->GetNative();
  gtk_label_set_text(label, pango_layout_get_text(layout));
  gtk_label_set_attributes(label, pango_layout_get_attributes(layout));

  // Format.
  const auto& format = text->GetFormat();
  gtk_label_set_line_wrap(label, format.wrap);
  gtk_label_set_ellipsize(GTK_LABEL(label),
                          format.ellipsis ? PANGO_ELLIPSIZE_END
                                          : PANGO_ELLIPSIZE_NONE);
  gtk_misc_set_alignment(GTK_MISC(label),
                         AlignToFloat(format.align),
                         AlignToFloat(format.valign));
}

}  // namespace nu
