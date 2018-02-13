// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_ASAR_ARCHIVE_H_
#define NATIVEUI_ASAR_ARCHIVE_H_

#include <string>

#include "base/files/file.h"
#include "base/memory/ref_counted.h"
#include "base/values.h"
#include "nativeui/nativeui_export.h"

namespace nu {

class AsarArchive : public base::RefCounted<AsarArchive> {
 public:
  static scoped_refptr<AsarArchive> Create(const base::FilePath& path);

  struct FileInfo {
    uint32_t size = 0;
    uint64_t offset = 0;
  };

  bool GetFileInfo(const base::FilePath& path, FileInfo* info);

  const base::FilePath& path() const { return path_; }
  const base::Value& header() const { return header_; }

 protected:
  explicit AsarArchive(const base::FilePath& path);
  virtual ~AsarArchive();

  bool IsValid() const;

 private:
  friend class base::RefCounted<AsarArchive>;

  base::FilePath path_;
  base::File file_;
  base::Value header_;
  uint64_t content_offset_ = 0;
};

}  // namespace nu

#endif  // NATIVEUI_ASAR_ARCHIVE_H_
