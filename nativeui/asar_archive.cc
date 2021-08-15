// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/asar_archive.h"

#include <memory>
#include <utility>
#include <vector>

#include "base/json/json_reader.h"
#include "base/pickle.h"
#include "base/stl_util.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_split.h"
#include "base/strings/utf_string_conversions.h"

namespace nu {

namespace {

// The version of asar format we supports.
const uint8_t kSupportedAsarVersion = 2;

}  // namespace

AsarArchive::AsarArchive(base::File file, bool extended_format)
    : file_(std::move(file)) {
  if (!file_.IsValid())
    return;

  // If it is an extended type of asar, search from the end of file.
  if (extended_format && !ReadExtendedMeta())
    return;

  // Read size.
  char size_buf[8];
  if (file_.ReadAtCurrentPos(size_buf, 8) != 8)
    return;
  uint32_t size;
  if (!base::PickleIterator(base::Pickle(size_buf, 8)).ReadUInt32(&size))
    return;

  // Read header.
  std::vector<char> header_buf(size);
  if (file_.ReadAtCurrentPos(header_buf.data(), size) != static_cast<int>(size))
    return;
  std::string header;
  if (!base::PickleIterator(
          base::Pickle(header_buf.data(), size)).ReadString(&header))
    return;

  // Parse header.
  absl::optional<base::Value> value = base::JSONReader::Read(header);
  if (!value || !value->is_dict())
    return;
  content_offset_ += 8 + size;
  header_ = std::move(*value);
}

AsarArchive::~AsarArchive() {
}

bool AsarArchive::IsValid() const {
  return file_.IsValid() && header_.is_dict();
}

bool AsarArchive::GetFileInfo(const std::string& path, FileInfo* info) {
  if (!header_.is_dict())
    return false;

  // Convert file path to value search path.
  // path/to/image.jpg => { files, path, files, to, files, image.jpg }
  std::vector<base::StringPiece> components = base::SplitStringPiece(
      path, "/\\", base::KEEP_WHITESPACE, base::SPLIT_WANT_NONEMPTY);
  std::vector<base::StringPiece> key;
  key.reserve(components.size() * 2);
  for (const base::StringPiece& c : components) {
    key.push_back("files");
    key.push_back(c);
  }

  // Search for the node representing path.
  const base::Value* node = header_.FindPath(key);
  if (!node)
    return false;

  // Read file information.
  const base::Value* link = node->FindKey("link");
  if (link && link->is_string())
    return GetFileInfo(link->GetString(), info);

  const base::Value* size = node->FindKey("size");
  if (!size || !size->is_int())
    return false;
  info->size = size->GetInt();

  const base::Value* offset = node->FindKey("offset");
  if (!offset || !offset->is_string() ||
      !base::StringToUint64(offset->GetString(), &info->offset))
    return false;

  info->offset += content_offset_;
  return true;
}

bool AsarArchive::ReadExtendedMeta() {
  // Read last 13 bytes, which are | size(8) | version(1) | magic(4) |.
  char magic[5] = { 0 };
  if (file_.Seek(base::File::FROM_END, -4) == -1 ||
      file_.ReadAtCurrentPos(magic, 4) != 4 ||
      base::StringPiece(magic) != "ASAR")
    return false;
  uint8_t version;
  if (file_.Seek(base::File::FROM_END, -5) == -1 ||
      file_.ReadAtCurrentPos(reinterpret_cast<char*>(&version), 1) != 1 ||
      version != kSupportedAsarVersion)
    return false;
  double size;
  if (file_.Seek(base::File::FROM_END, -13) == -1 ||
      file_.ReadAtCurrentPos(reinterpret_cast<char*>(&size), 8) != 8)
    return false;
  content_offset_ = file_.GetLength() - static_cast<uint64_t>(size);
  file_.Seek(base::File::FROM_BEGIN, content_offset_);
  return true;
}

}  // namespace nu
