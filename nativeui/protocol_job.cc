// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/protocol_job.h"

#include <algorithm>
#include <utility>

namespace nu {

///////////////////////////////////////////////////////////////////////////////
// ProtocolJob implementation.

ProtocolJob::ProtocolJob() {
}

ProtocolJob::~ProtocolJob() {
}

void ProtocolJob::Start() {
  notify_data_available(0);
}

void ProtocolJob::Kill() {
}

void ProtocolJob::Plug(std::function<void(size_t)> data_available) {
  notify_data_available = std::move(data_available);
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

void ProtocolStringJob::Start() {
  notify_data_available(content_.size());
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
