// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_PROTOCOL_ASAR_JOB_H_
#define NATIVEUI_PROTOCOL_ASAR_JOB_H_

#include <string>

#include "nativeui/protocol_file_job.h"
#include "nativeui/util/aes.h"

namespace nu {

class AES;

class NATIVEUI_EXPORT ProtocolAsarJob : public ProtocolFileJob {
 public:
  ProtocolAsarJob(const base::FilePath& asar, const std::string& path);

  bool SetDecipher(const std::string& key, const std::string& iv);

 protected:
  ~ProtocolAsarJob() override;

  // ProtocolJob:
  bool Start() override;
  size_t Read(void* buf, size_t buf_size) override;

  AES aes_;

  // Buffer used to store remaining encrypted data.
  uint8_t buffer_[AES_BLOCKLEN];
  size_t remaining_ = 0;
};

}  // namespace nu

#endif  // NATIVEUI_PROTOCOL_ASAR_JOB_H_
