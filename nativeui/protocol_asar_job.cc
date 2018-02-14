// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/protocol_asar_job.h"

#include "nativeui/asar_archive.h"

namespace nu {

namespace {

// The old asar extension name.
const base::FilePath::CharType kOldAsarExt[] = FILE_PATH_LITERAL(".asar");

}  // namespace

ProtocolAsarJob::ProtocolAsarJob(const base::FilePath& asar,
                                 const std::string& path)
    : ProtocolFileJob(asar) {
  // Do nothing if the asar file does not exist.
  if (!file_.IsValid())
    return;

  // Read asar.
  AsarArchive archive(file_.Duplicate(), !asar.MatchesExtension(kOldAsarExt));
  AsarArchive::FileInfo info;
  if (!archive.IsValid() ||
      !archive.GetFileInfo(path, &info)) {
    file_.Close();
    return;
  }

  // Seek to the position of the path.
  file_.Seek(base::File::FROM_BEGIN, info.offset);
  path_ = base::FilePath::FromUTF8Unsafe(path);
  content_length_ = info.size;
}

ProtocolAsarJob::~ProtocolAsarJob() {
}

}  // namespace nu
