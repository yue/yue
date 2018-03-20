// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/protocol_job.h"

#include <string.h>

#include <algorithm>
#include <utility>

namespace nu {

///////////////////////////////////////////////////////////////////////////////
// ProtocolJob implementation.

ProtocolJob::ProtocolJob() {
}

ProtocolJob::~ProtocolJob() {
}

bool ProtocolJob::Start() {
  notify_content_length(0);
  return true;
}

void ProtocolJob::Kill() {
}

void ProtocolJob::Plug(std::function<void(int)> func) {
  notify_content_length = std::move(func);
}

///////////////////////////////////////////////////////////////////////////////
// ProtocolStringJob implementation.

ProtocolStringJob::ProtocolStringJob(const std::string& mime_type,
                                     const std::string& content)
    : mime_type_(mime_type),
      content_(content) {
}

ProtocolStringJob::~ProtocolStringJob() {
}

bool ProtocolStringJob::Start() {
  notify_content_length(static_cast<int>(content_.size()));
  return true;
}

bool ProtocolStringJob::GetMimeType(std::string* mime_type) {
  *mime_type = mime_type_;
  return true;
}

size_t ProtocolStringJob::Read(void* buf, size_t buf_size) {
  if (pos_ == content_.size() || buf_size == 0)
    return 0;
  size_t nread = std::min(buf_size, content_.size() - pos_);
  memcpy(buf, content_.data() + pos_, nread);
  pos_ += nread;
  return nread;
}

}  // namespace nu
