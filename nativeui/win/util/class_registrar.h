// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_UTIL_CLASS_REGISTRAR_H_
#define NATIVEUI_WIN_UTIL_CLASS_REGISTRAR_H_

#include <list>
#include <string>

#include "base/synchronization/lock.h"

namespace nu {

// Win32Window class information used for registering unique windows.
struct ClassInfo {
  UINT style;
  HICON icon;
  HICON small_icon;

  ClassInfo(int style, HICON icon, HICON small_icon)
      : style(style), icon(icon), small_icon(small_icon) {}

  // Compares two ClassInfos. Returns true if all members match.
  bool Equals(const ClassInfo& other) const {
    return (other.style == style && other.icon == icon &&
            other.small_icon == small_icon);
  }
};

// WARNING: this class may be used on multiple threads.
class ClassRegistrar {
 public:
  ClassRegistrar();
  ~ClassRegistrar();

  void UnregisterClasses();

  // Returns the atom identifying the class matching |class_info|,
  // creating and registering a new class if the class is not yet known.
  ATOM RetrieveClassAtom(const ClassInfo& class_info);

 private:
  // Represents a registered window class.
  struct RegisteredClass {
    RegisteredClass(const ClassInfo& info,
                    const std::wstring& name,
                    ATOM atom,
                    HINSTANCE instance);

    // Info used to create the class.
    ClassInfo info;

    // The name given to the window class
    std::wstring name;

    // The atom identifying the window class.
    ATOM atom;

    // The handle of the module containing the window proceedure.
    HMODULE instance;
  };

  typedef std::list<RegisteredClass> RegisteredClasses;
  RegisteredClasses registered_classes_;

  // Counter of how many classes have been registered so far.
  int registered_count_;

  base::Lock lock_;
};

}  // namespace nu

#endif  // NATIVEUI_WIN_UTIL_CLASS_REGISTRAR_H_
