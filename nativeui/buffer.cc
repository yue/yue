// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/buffer.h"

#include "base/logging.h"

namespace nu {

// static
Buffer Buffer::Wrap(void* content, size_t size) {
  return Buffer(content, size, nullptr);
}

// static
Buffer Buffer::TakeOver(void* content, size_t size, FreeFunc free) {
  CHECK(free) << "Must pass a valid free function to TakeOver";
  return Buffer(content, size, free);
}

Buffer::Buffer(void* content, size_t size, FreeFunc free)
    : content_(content), size_(size), free_(free) {
}

Buffer::Buffer(Buffer&& other) noexcept
    : content_(other.content_), size_(other.size_), free_(other.free_) {
  other.content_ = nullptr;
  other.size_ = 0;
  other.free_ = nullptr;
}

Buffer::Buffer() noexcept {
}

Buffer::~Buffer() {
  if (free_)
    free_(content_);
}

}  // namespace nu
