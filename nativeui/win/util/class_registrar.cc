// Copyright 2016 Cheng Zhao. All rights reserved.
// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#include "nativeui/win/util/class_registrar.h"

#include "base/strings/string_number_conversions_win.h"
#include "base/win/win_util.h"
#include "base/win/wrapped_window_proc.h"
#include "nativeui/win/util/win32_window.h"

namespace nu {

namespace {

// static
const wchar_t* const kBaseClassName = L"Yue_";

}  // namespace

ClassRegistrar::ClassRegistrar() : registered_count_(0) {}

ClassRegistrar::~ClassRegistrar() {
  UnregisterClasses();
}

void ClassRegistrar::UnregisterClasses() {
  for (RegisteredClasses::iterator i = registered_classes_.begin();
        i != registered_classes_.end(); ++i) {
    if (!UnregisterClass(MAKEINTATOM(i->atom), i->instance)) {
      LOG(ERROR) << "Failed to unregister class " << i->name
                 << ". Error = " << GetLastError();
    }
  }
}

ATOM ClassRegistrar::RetrieveClassAtom(const ClassInfo& class_info) {
  base::AutoLock auto_lock(lock_);
  for (RegisteredClasses::const_iterator i = registered_classes_.begin();
       i != registered_classes_.end(); ++i) {
    if (class_info.Equals(i->info))
      return i->atom;
  }

  // No class found, need to register one.
  std::wstring name = std::wstring(kBaseClassName) +
      base::NumberToWString(registered_count_++);

  WNDCLASSEX window_class;
  base::win::InitializeWindowClass(
      name.c_str(), &base::win::WrappedWindowProc<Win32Window::WndProc>,
      class_info.style, 0, 0, LoadCursorW(NULL, IDC_ARROW),
      reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1), NULL,
      class_info.icon, class_info.small_icon, &window_class);
  HMODULE instance = window_class.hInstance;
  ATOM atom = RegisterClassEx(&window_class);
  CHECK(atom) << GetLastError();

  registered_classes_.push_back(RegisteredClass(
      class_info, name, atom, instance));

  return atom;
}

ClassRegistrar::RegisteredClass::RegisteredClass(const ClassInfo& info,
                                                 const std::wstring& name,
                                                 ATOM atom,
                                                 HMODULE instance)
    : info(info),
      name(name),
      atom(atom),
      instance(instance) {}

}  // namespace nu
