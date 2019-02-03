// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/clipboard.h"

#include <gtk/gtk.h>

#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"

namespace nu {

namespace {

const char kMarkupPrefix[] =
    "<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\">";

// Cached atom for markup type format.
GdkAtom& MarkupAtom() {
  static GdkAtom atom = gdk_atom_intern_static_string("text/html");
  return atom;
}

// Read markup string from selection data.
std::string ReadMarkupFromSelectionData(GtkSelectionData* selection) {
  const char* data =
      reinterpret_cast<const char*>(gtk_selection_data_get_data(selection));
  size_t size = gtk_selection_data_get_length(selection);

  // If the data starts with 0xFEFF, i.e., Byte Order Mark, assume it is
  // UTF-16, otherwise assume UTF-8.
  std::string markup;
  if (size >= 2 &&
      reinterpret_cast<const uint16_t*>(data)[0] == 0xFEFF) {
    base::UTF16ToUTF8(reinterpret_cast<const uint16_t*>(data) + 1,
                      (size / 2) - 1, &markup);
  } else {
    markup.assign(reinterpret_cast<const char*>(data), size);
  }

  // If there is a terminating NULL, drop it.
  if (!markup.empty() && markup.at(markup.length() - 1) == '\0')
    markup.resize(markup.length() - 1);

  // GTK adds some prefix in markup result, remove it have consistent result.
  if (base::StartsWith(markup, kMarkupPrefix, base::CompareCase::SENSITIVE))
    return markup.substr(arraysize(kMarkupPrefix) - 1,
                         markup.length() - (arraysize(kMarkupPrefix) - 1));

  return markup;
}

void ClipboardGet(GtkClipboard* clipboard,
                  GtkSelectionData* selection,
                  guint info,
                  gpointer user_data) {
  using Data = Clipboard::Data;
  auto* objects = static_cast<std::vector<Data>*>(user_data);
  if (info >= objects->size())
    return;

  const Data& data = (*objects)[info];
  switch (data.type()) {
    case Data::Type::Text:
      gtk_selection_data_set_text(selection,
                                  data.str().c_str(), data.str().size());
      break;
    case Data::Type::HTML: {
      // Some apps are expecting the meta prefix.
      std::string html = kMarkupPrefix + data.str();
      gtk_selection_data_set(selection, MarkupAtom(), 8,
                             reinterpret_cast<const guchar*>(html.c_str()),
                             html.size());
      break;
    }
    case Data::Type::Image:
      gtk_selection_data_set_pixbuf(
          selection,
          gdk_pixbuf_animation_get_static_image(data.image()->GetNative()));
      break;
    case Data::Type::FilePaths:
      if (gchar** uris = g_new(gchar*, data.file_paths().size() + 1)) {
        size_t i;
        for (i = 0; i < data.file_paths().size(); ++i) {
          uris[i] = g_filename_to_uri(data.file_paths()[i].value().c_str(),
                                      nullptr, nullptr);
        }
        uris[i] = nullptr;
        gtk_selection_data_set_uris(selection, uris);
        g_strfreev(uris);
      }
      break;
    default:
      NOTREACHED() << "Can not set clipboard data without type";
  }
}

void ClipboardClear(GtkClipboard* clipboard, gpointer user_data) {
  auto* objects = static_cast<std::vector<Clipboard::Data>*>(user_data);
  delete objects;
}

}  // namespace

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

bool Clipboard::IsDataAvailable(Data::Type type) const {
  switch (type) {
    case Data::Type::Text:
      return gtk_clipboard_wait_is_text_available(clipboard_);
    case Data::Type::HTML:
      return gtk_clipboard_wait_is_target_available(clipboard_, MarkupAtom());
    case Data::Type::Image:
      return gtk_clipboard_wait_is_image_available(clipboard_);
    case Data::Type::FilePaths:
      return gtk_clipboard_wait_is_uris_available(clipboard_);
    default:
      NOTREACHED() << "Can not get clipboard data without type";
      return false;
  }
}

Clipboard::Data Clipboard::GetData(Data::Type type) const {
  switch (type) {
    case Data::Type::Text:
      if (gchar* text = gtk_clipboard_wait_for_text(clipboard_)) {
        std::string str(text);
        g_free(text);
        return Data(Data::Type::Text, std::move(str));
      }
      return Data();
    case Data::Type::HTML:
      if (GtkSelectionData* selection =
              gtk_clipboard_wait_for_contents(clipboard_, MarkupAtom())) {
        std::string markup = ReadMarkupFromSelectionData(selection);
        gtk_selection_data_free(selection);
        return Data(Data::Type::HTML, std::move(markup));
      }
      return Data();
    case Data::Type::Image:
      if (GdkPixbuf* pixbuf = gtk_clipboard_wait_for_image(clipboard_)) {
        GdkPixbufSimpleAnim* image = gdk_pixbuf_simple_anim_new(
            gdk_pixbuf_get_width(pixbuf), gdk_pixbuf_get_height(pixbuf), 1.f);
        gdk_pixbuf_simple_anim_add_frame(image, pixbuf);
        g_object_unref(pixbuf);
        return Data(new Image(GDK_PIXBUF_ANIMATION(image)));
      }
      return Data();
    case Data::Type::FilePaths:
      if (gchar** uris = gtk_clipboard_wait_for_uris(clipboard_)) {
        std::vector<base::FilePath> result;
        for (int i = 0; uris[i]; i++) {
          if (gchar* path = g_filename_from_uri(uris[i], nullptr, nullptr)) {
            result.emplace_back(path);
            g_free(path);
          }
        }
        g_strfreev(uris);
        return Data(std::move(result));
      }
      return Data();
    default:
      NOTREACHED() << "Can not get clipboard data without type";
      return Data();
  }
}

void Clipboard::SetData(std::vector<Data> objects) {
  GtkTargetList* targets = gtk_target_list_new(0, 0);
  for (size_t i = 0; i < objects.size(); ++i) {
    const Data& data = objects[i];
    switch (data.type()) {
      case Data::Type::Text:
        gtk_target_list_add_text_targets(targets, i);
        break;
      case Data::Type::HTML:
        gtk_target_list_add(targets, MarkupAtom(), 0, i);
        break;
      case Data::Type::Image:
        gtk_target_list_add_image_targets(targets, i, true);
        break;
      case Data::Type::FilePaths:
        gtk_target_list_add_uri_targets(targets, i);
        break;
      default:
        NOTREACHED() << "Can not set clipboard data without type";
    }
  }

  int number = 0;
  GtkTargetEntry* table = gtk_target_table_new_from_list(targets, &number);
  if (table && number > 0) {
    gtk_clipboard_set_with_data(clipboard_, table, number,
                                ClipboardGet, ClipboardClear,
                                new std::vector<Data>(std::move(objects)));
    gtk_clipboard_set_can_store(clipboard_, nullptr, 0);
  } else {
    gtk_clipboard_clear(clipboard_);
  }

  if (table)
    gtk_target_table_free(table, number);
  gtk_target_list_unref(targets);
}

}  // namespace nu
