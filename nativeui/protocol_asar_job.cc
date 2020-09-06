// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/protocol_asar_job.h"

#include <string.h>

#include "base/logging.h"
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

bool ProtocolAsarJob::SetDecipher(const std::string& key,
                                  const std::string& iv) {
  return aes_.Init(key, iv);
}

bool ProtocolAsarJob::Start() {
  if (!aes_.IsValid())
    return ProtocolFileJob::Start();
  if (!file_.IsValid())
    return false;
  // Don't pass content length when stream is encrypted, since the decrypted
  // size might be smaller.
  notify_content_length(-1);
  return true;
}

size_t ProtocolAsarJob::Read(void* buf, size_t buf_size) {
  if (!aes_.IsValid())
    return ProtocolFileJob::Read(buf, buf_size);

  if (buf_size < remaining_)
    return 0;  // this is unlikely to happen

  // Read as much as we can.
  size_t nread = ProtocolFileJob::Read(static_cast<char*>(buf) + remaining_,
                                       buf_size - remaining_);
  if (nread == 0) {
    if (remaining_ != 0) {
      LOG(ERROR) << "The encrypted stream stored in asar is not aligned to "
                 << AES_BLOCKLEN << "bytes";
    }
    return 0;
  }

  // Put the remaining data of last read into buf, as if it was read this time.
  if (remaining_ > 0) {
    memcpy(buf, buffer_, remaining_);
    nread += remaining_;
  }

  // Handle the data in alignment of 16 bytes.
  uint8_t* in = static_cast<uint8_t*>(buf);
  uint8_t* out = static_cast<uint8_t*>(buf);
  remaining_ = nread;
  while (remaining_ >= AES_BLOCKLEN) {
    // Fill buffer with 16 bytes and move pointers forward.
    memcpy(buffer_, in, AES_BLOCKLEN);
    in += AES_BLOCKLEN;
    remaining_ -= AES_BLOCKLEN;
    // Write to output buffer with decrypted data.
    aes_.CBCDecryptBuffer(buffer_, AES_BLOCKLEN);
    memcpy(out, buffer_, AES_BLOCKLEN);
    out += AES_BLOCKLEN;
  }

  // Determine the padding when all data has been read.
  if (content_length_ == 0) {
    int paddings = buffer_[AES_BLOCKLEN - 1];
    if (nread < paddings + remaining_)
      return 0;  // likely a corrupted padding value
    // We should probably do some verification, but we don't really care when
    // the encryption is corrupted.
    nread -= paddings;
  }

  // Still have some data left, leave it to the next read.
  DCHECK(remaining_ < AES_BLOCKLEN);
  memcpy(buffer_, in, remaining_);

  // Return the bytes we decrypted.
  // FIXME(zcbenz): The stream would end when we can not get 16 bytes in one
  // read, we should probably improve our API to fix this.
  return nread - remaining_;
}

}  // namespace nu
