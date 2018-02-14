// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_ASAR_ARCHIVE_H_
#define NATIVEUI_ASAR_ARCHIVE_H_

#include <string>

#include "base/files/file.h"
#include "base/values.h"
#include "nativeui/nativeui_export.h"

namespace nu {

class NATIVEUI_EXPORT AsarArchive {
 public:
  struct FileInfo {
    uint32_t size = 0;
    uint64_t offset = 0;
  };

  AsarArchive(base::File file, bool extended_format);
  virtual ~AsarArchive();

  bool IsValid() const;
  bool GetFileInfo(const std::string& path, FileInfo* info);

 private:
  bool ReadExtendedMeta();

  base::File file_;
  base::Value header_;
  uint64_t content_offset_ = 0;
};

}  // namespace nu

#endif  // NATIVEUI_ASAR_ARCHIVE_H_
