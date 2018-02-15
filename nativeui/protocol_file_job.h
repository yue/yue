// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_PROTOCOL_FILE_JOB_H_
#define NATIVEUI_PROTOCOL_FILE_JOB_H_

#include <string>

#include "base/files/file.h"
#include "base/files/file_path.h"
#include "nativeui/protocol_job.h"

namespace nu {

// Serve file for the protocol request.
class NATIVEUI_EXPORT ProtocolFileJob : public ProtocolJob {
 public:
  explicit ProtocolFileJob(const base::FilePath& path);

  // ProtocolJob:
  bool Start() override;
  void Kill() override;
  bool GetMimeType(std::string* mime_type) override;
  size_t Read(void* buf, size_t buf_size) override;

 protected:
  ~ProtocolFileJob() override;

  base::FilePath path_;
  base::File file_;
  int64_t content_length_ = 0;
};

}  // namespace nu

#endif  // NATIVEUI_PROTOCOL_FILE_JOB_H_
