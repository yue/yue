// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_PROTOCOL_ASAR_JOB_H_
#define NATIVEUI_PROTOCOL_ASAR_JOB_H_

#include "nativeui/protocol_file_job.h"

namespace nu {

class NATIVEUI_EXPORT ProtocolAsarJob : public ProtocolFileJob {
 public:
  explicit ProtocolAsarJob(const base::FilePath& path);

 protected:
  ~ProtocolAsarJob() override;
};

}  // namespace nu

#endif  // NATIVEUI_PROTOCOL_ASAR_JOB_H_
