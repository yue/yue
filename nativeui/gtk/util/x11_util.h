// Copyright 2017 Cheng Zhao. All rights reserved.
// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GTK_UTIL_X11_UTIL_H_
#define NATIVEUI_GTK_UTIL_X11_UTIL_H_

#include <memory>
#include <vector>

#include "nativeui/gtk/util/x11_types.h"

namespace nu {

// Get the value of an int, int array, atom array or string property.  On
// success, true is returned and the value is stored in |value|.
bool GetAtomArrayProperty(XDisplay* display,
                          XID window,
                          const std::string& property_name,
                          std::vector<XAtom>* value);

// These setters all make round trips.
bool SetAtomArrayProperty(XDisplay* display,
                          XID window,
                          const std::string& name,
                          const std::string& type,
                          const std::vector<XAtom>& value);

// Gets the X atom for default display corresponding to atom_name.
XAtom GetAtom(XDisplay* display, const char* atom_name);

}  // namespace nu

#endif  // NATIVEUI_GTK_UTIL_X11_UTIL_H_
