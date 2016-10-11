// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_FONT_H_
#define NATIVEUI_GFX_FONT_H_

#include <string>

#include "base/memory/ref_counted.h"
#include "nativeui/nativeui_export.h"
#include "nativeui/types.h"

namespace nu {

NATIVEUI_EXPORT class Font : public base::RefCounted<Font> {
 public:
  // Creates an appropriate Font implementation.
  static Font* CreateDefault();
  // Creates a Font implementation with the specified |font_name|
  // (encoded in UTF-8) and |font_size| in pixels.
  static Font* CreateFromNameAndSize(const std::string& font_name,
                                     int font_size);

  // Returns the specified font name in UTF-8.
  virtual std::string GetName() const = 0;

  // Returns the font size in pixels.
  virtual int GetSize() const = 0;

  // Returns the native font handle.
  virtual NativeFont GetNative() const = 0;

 protected:
  Font() {}
  virtual ~Font() {}

 private:
  friend class base::RefCounted<Font>;

  DISALLOW_COPY_AND_ASSIGN(Font);
};

}  // namespace nu

#endif  // NATIVEUI_GFX_FONT_H_
