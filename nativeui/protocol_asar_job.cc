// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/protocol_asar_job.h"

#include "nativeui/asar_archive.h"

namespace nu {

namespace {

const base::FilePath::CharType kAsarExtension[] = FILE_PATH_LITERAL(".asar");

bool GetAsarArchivePath(const base::FilePath& full_path,
                        base::FilePath* asar_path,
                        base::FilePath* relative_path) {
  base::FilePath iter = full_path;
  while (true) {
    base::FilePath dirname = iter.DirName();
    if (iter.MatchesExtension(kAsarExtension))
      break;
    else if (iter == dirname)
      return false;
    iter = dirname;
  }

  base::FilePath tail;
  if (!iter.AppendRelativePath(full_path, &tail))
    return false;

  *asar_path = iter;
  *relative_path = tail;
  return true;
}

}  // namespace

ProtocolAsarJob::ProtocolAsarJob(const base::FilePath& path)
    : ProtocolFileJob(path) {
  // Do nothing if it is actually a real path.
  if (file_.IsValid())
    return;

  // Read asar.
  base::FilePath asar_path, relative_path;
  if (!GetAsarArchivePath(path, &asar_path, &relative_path))
    return;
  scoped_refptr<AsarArchive> archive = AsarArchive::Create(asar_path);
  if (!archive)
    return;
  AsarArchive::FileInfo info;
  if (!archive->GetFileInfo(relative_path, &info))
    return;

  // Redirect the file to read from asar.
  file_.Initialize(asar_path, base::File::FLAG_OPEN | base::File::FLAG_READ);
  if (file_.IsValid()) {
    file_.Seek(base::File::FROM_BEGIN, info.offset);
    content_length_ = info.size;
  }
}

ProtocolAsarJob::~ProtocolAsarJob() {
}

}  // namespace nu
