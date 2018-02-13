// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/asar_archive.h"

#include <memory>
#include <utility>
#include <vector>

#include "base/json/json_reader.h"
#include "base/pickle.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_split.h"

namespace nu {

namespace {

std::vector<std::string> FilePathToValuePath(const base::FilePath& path) {
  std::string str = path.AsUTF8Unsafe();
  std::vector<std::string> components = base::SplitString(
      str, "/\\", base::KEEP_WHITESPACE, base::SPLIT_WANT_NONEMPTY);
  std::vector<std::string> result;
  result.reserve(components.size() * 2);
  for (std::string& c : components) {
    result.push_back("files");
    result.push_back(std::move(c));
  }
  return result;
}

}  // namespace

// static
scoped_refptr<AsarArchive> AsarArchive::Create(const base::FilePath& path) {
  scoped_refptr<AsarArchive> archive(new AsarArchive(path));
  return archive->IsValid() ? archive : nullptr;
}

AsarArchive::AsarArchive(const base::FilePath& path)
    : path_(path),
      file_(path, base::File::FLAG_OPEN | base::File::FLAG_READ) {
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
  std::unique_ptr<base::Value> value = base::JSONReader::Read(header);
  if (!value || !value->is_dict())
    return;
  content_offset_ = 8 + size;
  header_ = std::move(*value.release());
}

AsarArchive::~AsarArchive() {
}

bool AsarArchive::GetFileInfo(const base::FilePath& path, FileInfo* info) {
  if (!header_.is_dict())
    return false;

  std::vector<std::string> key = FilePathToValuePath(path);
  std::vector<base::StringPiece> pk(key.begin(), key.end());
  const base::Value* node = header_.FindPath(pk);
  if (!node)
    return false;

  const base::Value* link = node->FindKey("link");
  if (link && link->is_string())
    return GetFileInfo(base::FilePath::FromUTF8Unsafe(link->GetString()), info);

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

bool AsarArchive::IsValid() const {
  return file_.IsValid() && header_.is_dict();
}

}  // namespace nu
