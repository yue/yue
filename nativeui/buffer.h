// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_BUFFER_H_
#define NATIVEUI_BUFFER_H_

#include <functional>
#include <memory>

#include "build/build_config.h"
#include "nativeui/nativeui_export.h"

#if defined(OS_MAC)
#ifdef __OBJC__
@class NSData;
#else
class NSData;
#endif
#endif

namespace nu {

// A move-only class to manage memory.
// Note: Currently for language bindings we are assuming the buffer passed to
// APIs are consumed immediately, so memory passed from language bindings are
// NOT copied. Keep this in mind when designing new APIs.
class NATIVEUI_EXPORT Buffer {
 public:
  using FreeFunc = std::function<void(void*)>;

  // Wrap the memory but do not free it.
  static Buffer Wrap(const void* content, size_t size);

  // Take over the memory and free it when done.
  static Buffer TakeOver(void* content, size_t size, FreeFunc free);

  Buffer(Buffer&& other) noexcept;
  Buffer() noexcept;

  ~Buffer();

  Buffer& operator=(Buffer&& other) noexcept;
  Buffer& operator=(const Buffer&) = delete;
  Buffer(const Buffer&) = delete;

  void* content() const { return content_; }
  size_t size() const { return size_; }

#if defined(OS_MAC)
  // Return an autoreleased NSData which does not manage the memory.
  NSData* ToNSData() const;
#endif

 private:
  Buffer(void* content, size_t size, FreeFunc free);

  void* content_ = nullptr;
  size_t size_ = 0;
  FreeFunc free_;
};

}  // namespace nu

#endif  // NATIVEUI_BUFFER_H_
