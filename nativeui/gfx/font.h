// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_FONT_H_
#define NATIVEUI_GFX_FONT_H_

#include <string>

#include "base/memory/ref_counted.h"
#include "base/strings/string_piece.h"
#include "nativeui/nativeui_export.h"
#include "nativeui/types.h"

namespace nu {

class NATIVEUI_EXPORT Font : public base::RefCounted<Font> {
 public:
  // Creates an appropriate Font implementation.
  Font();
  // Creates a Font implementation with the specified |font_name|
  // (encoded in UTF-8) and |font_size| in pixels.
  Font(const std::string& font_name, int font_size);

  // Returns the specified font name in UTF-8.
  std::string GetName() const;

  // Returns the font size in pixels.
  int GetSize() const;

  // Returns the native font handle.
  NativeFont GetNative() const;

 protected:
  virtual ~Font();

 private:
  friend class base::RefCounted<Font>;

  NativeFont font_;
};

}  // namespace nu

#endif  // NATIVEUI_GFX_FONT_H_
