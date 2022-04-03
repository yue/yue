// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gtk/util/clipboard_util.h"

#include <utility>
#include <vector>

#include "base/notreached.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"

namespace nu {

using Data = Clipboard::Data;

namespace {

const char kMarkupPrefix[] =
    "<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\">";

GdkAtom& MarkupAtom() {
  static GdkAtom atom = gdk_atom_intern_static_string("text/html");
  return atom;
}

// Read markup string from selection data.
std::string ReadMarkupFromSelectionData(GtkSelectionData* selection) {
  const char* data =
      reinterpret_cast<const char*>(gtk_selection_data_get_data(selection));
  size_t size = gtk_selection_data_get_length(selection);
  if (!data || size == 0)
    return std::string();

  // If the data starts with 0xFEFF, i.e., Byte Order Mark, assume it is
  // UTF-16, otherwise assume UTF-8.
  std::string markup;
  if (size >= 2 &&
      reinterpret_cast<const char16_t*>(data)[0] == 0xFEFF) {
    base::UTF16ToUTF8(reinterpret_cast<const char16_t*>(data) + 1,
                      (size / 2) - 1, &markup);
  } else {
    markup.assign(reinterpret_cast<const char*>(data), size);
  }

  // If there is a terminating NULL, drop it.
  if (!markup.empty() && markup.at(markup.length() - 1) == '\0')
    markup.resize(markup.length() - 1);

  // GTK adds some prefix in markup result, remove it have consistent result.
  if (base::StartsWith(markup, kMarkupPrefix, base::CompareCase::SENSITIVE))
    return markup.substr(std::size(kMarkupPrefix) - 1,
                         markup.length() - (std::size(kMarkupPrefix) - 1));

  return markup;
}

Data GetDataFromGType(gchar* text) {
  if (!text)
    return Data();
  std::string str(text);
  g_free(text);
  return Data(Data::Type::Text, std::move(str));
}

Data GetDataFromGType(GdkPixbuf* pixbuf) {
  if (!pixbuf)
    return Data();
  GdkPixbufSimpleAnim* image = gdk_pixbuf_simple_anim_new(
      gdk_pixbuf_get_width(pixbuf), gdk_pixbuf_get_height(pixbuf), 1.f);
  gdk_pixbuf_simple_anim_add_frame(image, pixbuf);
  g_object_unref(pixbuf);
  return Data(new Image(GDK_PIXBUF_ANIMATION(image)));
}

Data GetDataFromGType(gchar** uris) {
  if (!uris)
    return Data();
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

}  // namespace

GdkAtom GetAtomForType(Clipboard::Data::Type type) {
  switch (type) {
    case Data::Type::Text:
      return gdk_atom_intern("text/plain;charset=utf-8", false);
    case Data::Type::HTML:
      return MarkupAtom();
    case Data::Type::Image:
      return gdk_atom_intern("image/png", false);
    case Data::Type::FilePaths:
      return gdk_atom_intern("text/uri-list", false);
    case Data::Type::None:
      return nullptr;
  }
  NOTREACHED() << "Invalid type: " << static_cast<int>(type);
  return nullptr;
}

bool IsDataAvailableInClipboard(GtkClipboard* clipboard,
                                Clipboard::Data::Type type) {
  switch (type) {
    case Data::Type::Text:
      return gtk_clipboard_wait_is_text_available(clipboard);
    case Data::Type::HTML:
      return gtk_clipboard_wait_is_target_available(clipboard, MarkupAtom());
    case Data::Type::Image:
      return gtk_clipboard_wait_is_image_available(clipboard);
    case Data::Type::FilePaths:
      return gtk_clipboard_wait_is_uris_available(clipboard);
    case Data::Type::None:
      return false;
  }
  NOTREACHED() << "Invalid type: " << static_cast<int>(type);
  return false;
}

void FillTargetList(GtkTargetList* targets, Clipboard::Data::Type type, int f) {
  switch (type) {
    case Data::Type::Text:
      gtk_target_list_add_text_targets(targets, f);
      break;
    case Data::Type::HTML:
      gtk_target_list_add(targets, MarkupAtom(), 0, f);
      break;
    case Data::Type::Image:
      gtk_target_list_add_image_targets(targets, f, true);
      break;
    case Data::Type::FilePaths:
      gtk_target_list_add_uri_targets(targets, f);
      break;
    case Data::Type::None:
      break;
    default:
      NOTREACHED() << "Invalid type: " << static_cast<int>(type);
  }
}

void FillSelection(GtkSelectionData* selection, const Clipboard::Data& data) {
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
    case Data::Type::None:
      break;
    default:
      NOTREACHED() << "Invalid type: " << static_cast<int>(data.type());
  }
}

Data GetDataFromSelection(GtkSelectionData* selection,
                          Clipboard::Data::Type type) {
  if (!selection)
    return Data();
  switch (type) {
    case Data::Type::Text:
      return GetDataFromGType(
          reinterpret_cast<gchar*>(gtk_selection_data_get_text(selection)));
    case Data::Type::HTML:
      return Data(Data::Type::HTML, ReadMarkupFromSelectionData(selection));
    case Data::Type::Image:
      return GetDataFromGType(gtk_selection_data_get_pixbuf(selection));
    case Data::Type::FilePaths:
      return GetDataFromGType(gtk_selection_data_get_uris(selection));
    case Data::Type::None:
      return Data();
  }
  NOTREACHED() << "Invalid type: " << static_cast<int>(type);
  return Data();
}

Data GetDataFromClipboard(GtkClipboard* clipboard,
                          Clipboard::Data::Type type) {
  switch (type) {
    case Data::Type::Text:
      return GetDataFromGType(gtk_clipboard_wait_for_text(clipboard));
    case Data::Type::HTML:
      if (GtkSelectionData* selection =
              gtk_clipboard_wait_for_contents(clipboard, MarkupAtom())) {
        std::string markup = ReadMarkupFromSelectionData(selection);
        gtk_selection_data_free(selection);
        return Data(Data::Type::HTML, std::move(markup));
      }
      return Data();
    case Data::Type::Image:
      return GetDataFromGType(gtk_clipboard_wait_for_image(clipboard));
    case Data::Type::FilePaths:
      return GetDataFromGType(gtk_clipboard_wait_for_uris(clipboard));
    case Data::Type::None:
      return Data();
  }
  NOTREACHED() << "Invalid type: " << static_cast<int>(type);
  return Data();
}

}  // namespace nu
