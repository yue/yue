// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_CLIPBOARD_H_
#define NATIVEUI_CLIPBOARD_H_

#include <string>

#include "base/memory/weak_ptr.h"
#include "nativeui/nativeui_export.h"
#include "nativeui/types.h"

namespace nu {

// Native clipboard, can only be obtained from App.
class NATIVEUI_EXPORT Clipboard {
 public:
  ~Clipboard();

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

  void Clear();
  void SetText(const std::string& text);
  std::string GetText() const;

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
