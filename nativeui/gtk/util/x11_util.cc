// Copyright 2017 Cheng Zhao. All rights reserved.
// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gtk/util/x11_util.h"

#include <X11/Xatom.h>
#include <X11/Xlib.h>

#include "base/logging.h"
#include "nativeui/gtk/util/x11_error_tracker.h"

namespace nu {

namespace {

template <class T, class R, R (*F)(T*)>
struct XObjectDeleter {
  inline void operator()(void* ptr) const { F(static_cast<T*>(ptr)); }
};

template <class T, class D = XObjectDeleter<void, int, XFree>>
using XScopedPtr = std::unique_ptr<T, D>;

// Note: The caller should free the resulting value data.
bool GetProperty(XDisplay* display, XID window,
                 const std::string& property_name, long max_length,
                 XAtom* type, int* format, unsigned long* num_items,
                 unsigned char** property) {
  XAtom property_atom = GetAtom(display, property_name.c_str());
  unsigned long remaining_bytes = 0;
  return XGetWindowProperty(display,
                            window,
                            property_atom,
                            0,          // offset into property data to read
                            max_length, // max length to get
                            False,      // deleted
                            AnyPropertyType,
                            type,
                            format,
                            num_items,
                            &remaining_bytes,
                            property);
}

}  // namespace

bool GetAtomArrayProperty(XDisplay* display,
                          XID window,
                          const std::string& property_name,
                          std::vector<XAtom>* value) {
  XAtom type = None;
  int format = 0;  // size in bits of each item in 'property'
  unsigned long num_items = 0;
  unsigned char* properties = NULL;

  int result = GetProperty(display, window, property_name,
                           (~0L), // (all of them)
                           &type, &format, &num_items, &properties);
  XScopedPtr<unsigned char> scoped_properties(properties);
  if (result != Success)
    return false;

  if (type != XA_ATOM)
    return false;

  XAtom* atom_properties = reinterpret_cast<XAtom*>(properties);
  value->clear();
  value->insert(value->begin(), atom_properties, atom_properties + num_items);
  return true;
}

bool SetAtomArrayProperty(XDisplay* display,
                          XID window,
                          const std::string& name,
                          const std::string& type,
                          const std::vector<XAtom>& value) {
  DCHECK(!value.empty());
  XAtom name_atom = GetAtom(display, name.c_str());
  XAtom type_atom = GetAtom(display, type.c_str());

  // XChangeProperty() expects values of type 32 to be longs.
  std::unique_ptr<XAtom[]> data(new XAtom[value.size()]);
  for (size_t i = 0; i < value.size(); ++i)
    data[i] = value[i];

  X11ErrorTracker err_tracker(display);
  XChangeProperty(display,
                  window,
                  name_atom,
                  type_atom,
                  32,  // size in bits of items in 'value'
                  PropModeReplace,
                  reinterpret_cast<const unsigned char*>(data.get()),
                  value.size());  // num items
  return !err_tracker.FoundNewError();
}

XAtom GetAtom(XDisplay* display, const char* atom_name) {
  return XInternAtom(display, atom_name, false);
}

}  // namespace nu
