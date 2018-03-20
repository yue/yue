// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_BUFFER_H_
#define NATIVEUI_BUFFER_H_

#include <functional>
#include <memory>

#include "base/macros.h"

namespace nu {

// A move-only class to manage memory.
class Buffer {
 public:
  using FreeFunc = std::function<void(void*)>;

  // Wrap the memory but do not free it.
  static Buffer Wrap(void* content, size_t size);

  // Take over the memory and free it when done.
  static Buffer TakeOver(void* content, size_t size, FreeFunc free);

  Buffer(Buffer&& other) noexcept;
  Buffer() noexcept;

  ~Buffer();

  void* content() const { return content_; }
  size_t size() const { return size_; }

 private:
  Buffer(void* content, size_t size, FreeFunc free);

  void* content_ = nullptr;
  size_t size_ = 0;
  FreeFunc free_;

  DISALLOW_COPY_AND_ASSIGN(Buffer);
};

}  // namespace nu

#endif  // NATIVEUI_BUFFER_H_
