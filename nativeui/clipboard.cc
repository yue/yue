// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/clipboard.h"

#include <utility>

namespace nu {

Clipboard::Data::Data() : type_(Type::None) {}

Clipboard::Data::Data(Data&& that) {
  InternalMoveConstructFrom(std::move(that));
}

Clipboard::Data::Data(Type t, std::string s) : type_(t), str_(std::move(s)) {
  if (type_ != Type::Text && type_ != Type::HTML) {
    type_ = Type::Text;  // fix wrong type to make destructor work
    NOTREACHED() << "String data must be string type";
  }
}

Clipboard::Data::Data(Image* i) : type_(Type::Image), image_(i) {}

Clipboard::Data::Data(std::vector<base::FilePath> f)
    : type_(Type::FilePaths), file_paths_(std::move(f)) {}

Clipboard::Data::~Data() {
  InternalCleanup();
}

Clipboard::Data& Clipboard::Data::operator=(Data&& that) {
  InternalCleanup();
  InternalMoveConstructFrom(std::move(that));
  return *this;
}

void Clipboard::Data::InternalCleanup() {
  switch (type_) {
    case Type::None:
      break;
    case Type::Text:
    case Type::HTML:
      str_.~basic_string();
      break;
    case Type::Image:
      image_.~scoped_refptr<Image>();
      break;
    case Type::FilePaths:
      file_paths_.~vector<base::FilePath>();
      break;
  }
  type_ = Type::None;
}

void Clipboard::Data::InternalMoveConstructFrom(Data&& that) {
  type_ = that.type_;
  switch (type_) {
    case Type::None:
      break;
    case Type::Text:
    case Type::HTML:
      new (&str_) std::string(std::move(that.str_));
      break;
    case Type::Image:
      new (&image_) scoped_refptr<Image>(std::move(that.image_));
      break;
    case Type::FilePaths:
      new (&file_paths_) std::vector<base::FilePath>(
          std::move(that.file_paths_));
      break;
  }
}

Clipboard::Clipboard(Type type)
    : type_(type), clipboard_(PlatformCreate(type)), weak_factory_(this) {}

Clipboard::~Clipboard() {
  PlatformDestroy();
}

void Clipboard::Clear() {
  SetData({});
}

void Clipboard::SetText(const std::string& text) {
  std::vector<Data> objects;
  objects.emplace_back(Data::Type::Text, text);
  SetData(std::move(objects));
}

std::string Clipboard::GetText() const {
  Data data = GetData(Data::Type::Text);
  return data.type() == Data::Type::Text ? data.str() : std::string();
}

}  // namespace nu
