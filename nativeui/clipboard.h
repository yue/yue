// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_CLIPBOARD_H_
#define NATIVEUI_CLIPBOARD_H_

#include <string>
#include <vector>

#include "base/memory/weak_ptr.h"
#include "nativeui/gfx/image.h"
#include "nativeui/message_loop.h"
#include "nativeui/signal.h"

#if defined(OS_LINUX)
// X11 headers define macros for these function names which screw with us.
#if defined(None)
#undef None
#endif
#endif

namespace nu {

// Native clipboard, can only be obtained from App.
class NATIVEUI_EXPORT Clipboard {
 public:
  ~Clipboard();

  // Types of clipboard.
  enum class Type {
    CopyPaste,
#if defined(OS_MAC)
    Drag,
    Find,
    Font,
#elif defined(OS_LINUX)
    Selection,
#endif
    Count,
  };

  static Clipboard* Get();
  static Clipboard* FromType(Type type);

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
    explicit Data(scoped_refptr<Image> image);
    explicit Data(std::vector<base::FilePath> file_paths);

    Data& operator=(Data&& that);

    ~Data();

    Data Clone() const;

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

#if defined(OS_MAC)
  // Initializing from existing draggingPasteboard.
  explicit Clipboard(NativeClipboard clipboard);
#endif

  void Clear();
  void SetText(const std::string& text);
  std::string GetText() const;

  bool IsDataAvailable(Data::Type type) const;
  Data GetData(Data::Type type) const;
  void SetData(std::vector<Data> objects);

  void StartWatching();
  void StopWatching();

  NativeClipboard GetNative() const { return clipboard_; }

  base::WeakPtr<Clipboard> GetWeakPtr() { return weak_factory_.GetWeakPtr(); }

  // Events.
  Signal<void(Clipboard*)> on_change;

 private:
  friend class State;

  explicit Clipboard(Type type);

  NativeClipboard PlatformCreate(Type type);
  void PlatformDestroy();
  void PlatformStartWatching();
  void PlatformStopWatching();

#if defined(OS_MAC)
  void OnTimer();
#endif

  Type type_;
  NativeClipboard clipboard_;

  bool is_watching_ = false;
#if defined(OS_MAC)
  // Timer-based clipboard watching.
  MessageLoop::TimerId timer_ = 0;
  int change_count_ = 0;
#elif defined(OS_LINUX)
  // Signal-based clipboard watching.
  ulong signal_ = 0;
#endif

  base::WeakPtrFactory<Clipboard> weak_factory_;
};

}  // namespace nu

#endif  // NATIVEUI_CLIPBOARD_H_
