// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GTK_UTIL_CLIPBOARD_UTIL_H_
#define NATIVEUI_GTK_UTIL_CLIPBOARD_UTIL_H_

#include <gtk/gtk.h>

#include <string>

#include "nativeui/clipboard.h"

namespace nu {

// Return GdkAtom for the type.
// Note that one type may have multiple possible GdkAtom, so the return value
// can only be used to set target types instead of comparing.
GdkAtom GetAtomForType(Clipboard::Data::Type type);

// Return whether data type is available in clipboard.
bool IsDataAvailableInClipboard(GtkClipboard* clipboard,
                                Clipboard::Data::Type type);

// Fill the GtkTargetList according to data type.
void FillTargetList(GtkTargetList* targets, Clipboard::Data::Type type, int f);

// Fill the GtkSelectionData according to data type.
void FillSelection(GtkSelectionData* selection, const Clipboard::Data& data);

// Get data from GtkSelectionData.
Clipboard::Data GetDataFromSelection(GtkSelectionData* selection,
                                     Clipboard::Data::Type type);

// Get data from GtkClipboard.
Clipboard::Data GetDataFromClipboard(GtkClipboard* clipboard,
                                     Clipboard::Data::Type type);

}  // namespace nu

#endif  // NATIVEUI_GTK_UTIL_CLIPBOARD_UTIL_H_
