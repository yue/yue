// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#ifndef NATIVEUI_GTK_UTIL_SCOPED_GOBJECT_H_
#define NATIVEUI_GTK_UTIL_SCOPED_GOBJECT_H_

namespace nu {

// Similar in spirit to a std::unique_ptr.
template <typename T>
class ScopedGObject {
 public:
  ScopedGObject() : obj_(nullptr) {}

  explicit ScopedGObject(T* obj) : obj_(obj) { Ref(); }

  ScopedGObject(const ScopedGObject<T>& other) = delete;

  ScopedGObject(ScopedGObject<T>&& other) : obj_(other.obj_) {
    other.obj_ = nullptr;
  }

  ~ScopedGObject() { reset(); }

  ScopedGObject<T>& operator=(const ScopedGObject<T>& other) = delete;

  ScopedGObject<T>& operator=(ScopedGObject<T>&& other) {
    reset();
    obj_ = other.obj_;
    other.obj_ = nullptr;
    return *this;
  }

  T* get() { return obj_; }

  operator T*() { return obj_; }

  void reset(T* obj = nullptr) {
    Unref();
    obj_ = obj;
    Ref();
  }

 private:
  void Ref() {
    // Remove the floating reference from |obj_| if it has one.
    if (obj_ && g_object_is_floating(obj_))
      g_object_ref_sink(obj_);
  }

  // This function is necessary so that gtk can overload it in
  // the case of T = GtkStyleContext.
  void Unref() {
    if (obj_)
      g_object_unref(obj_);
  }

  T* obj_;
};

}  // namespace nu

#endif  // NATIVEUI_GTK_UTIL_SCOPED_GOBJECT_H_
