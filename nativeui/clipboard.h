// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_CLIPBOARD_H_
#define NATIVEUI_CLIPBOARD_H_

#include <string>
#include <vector>

#include "base/memory/weak_ptr.h"
#include "nativeui/gfx/image.h"

namespace nu {

// Native clipboard, can only be obtained from App.
class NATIVEUI_EXPORT Clipboard {
 public:
  ~Clipboard();

  // Types of clipboard.
  enum class Type {
    CopyPaste,
#if defined(OS_MACOSX)
    Drag,
    Find,
    Font,
#elif defined(OS_LINUX)
    Selection,
#endif
    Count,
  };

  // Abstraction of data in clipboard.
  class NATIVEUI_EXPORT Data {
   public:
    // Types of the data.
    enum class Type {
      None,
      Text,
      HTML,
      Image,
      FilePaths,
    };

    Data();
    Data(Data&& that);

    Data(Type type, std::string str);
    explicit Data(Image* image);
    explicit Data(std::vector<base::FilePath> file_paths);

    Data& operator=(Data&& that);

    ~Data();

    // Getters for data.
    Type type() const { return type_; }
    std::string& str() {
      CHECK(type_ == Type::Text || type_ == Type::HTML);
      return str_;
    }
    const std::string& str() const {
      CHECK(type_ == Type::Text || type_ == Type::HTML);
      return str_;
    }
    Image* image() const {
      CHECK_EQ(type_, Type::Image);
      return image_.get();
    }
    std::vector<base::FilePath>& file_paths() {
      CHECK_EQ(type_, Type::FilePaths);
      return file_paths_;
    }
    const std::vector<base::FilePath>& file_paths() const {
      CHECK_EQ(type_, Type::FilePaths);
      return file_paths_;
    }

   private:
    void InternalCleanup();
    void InternalMoveConstructFrom(Data&& that);

    Type type_;

    union {
      std::string str_;
      scoped_refptr<Image> image_;
      std::vector<base::FilePath> file_paths_;
    };
  };

  void Clear();
  void SetText(const std::string& text);
  std::string GetText() const;

  bool IsDataAvailable(Data::Type type) const;
  Data GetData(Data::Type type) const;
  void SetData(std::vector<Data> objects);

  NativeClipboard GetNative() const { return clipboard_; }

  base::WeakPtr<Clipboard> GetWeakPtr() { return weak_factory_.GetWeakPtr(); }

 private:
  friend class App;

  explicit Clipboard(Type type);

  NativeClipboard PlatformCreate(Type type);
  void PlatformDestroy();

  Type type_;
  NativeClipboard clipboard_;

  base::WeakPtrFactory<Clipboard> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(Clipboard);
};

}  // namespace nu

#endif  // NATIVEUI_CLIPBOARD_H_
