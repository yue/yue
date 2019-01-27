// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/clipboard.h"

#include <gtk/gtk.h>

namespace nu {

NativeClipboard Clipboard::PlatformCreate(Type type) {
  switch (type) {
    case Type::CopyPaste:
      return gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    case Type::Selection:
      return gtk_clipboard_get(GDK_SELECTION_PRIMARY);
    default:
      NOTREACHED() << "Type::Count is not a valid clipboard type";
  }
  return gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
}

void Clipboard::PlatformDestroy() {
}

void Clipboard::Clear() {
  gtk_clipboard_clear(clipboard_);
}

void Clipboard::SetText(const std::string& text) {
  gtk_clipboard_set_text(clipboard_, text.c_str(), text.size());
}

std::string Clipboard::GetText() const {
  gchar* data = gtk_clipboard_wait_for_text(clipboard_);
  std::string result(data ? data : "");
  if (data)
    g_free(data);
  return result;
}

}  // namespace nu
