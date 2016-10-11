// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_GTK_FONT_GTK_H_
#define NATIVEUI_GFX_GTK_FONT_GTK_H_

#include <string>

#include "nativeui/gfx/font.h"

namespace nu {

NATIVEUI_EXPORT class FontGtk : public Font {
 public:
  explicit FontGtk(PangoFontDescription* font);
  FontGtk(const std::string& font_name, int font_size);

  // Font:
  std::string GetName() const override;
  int GetSize() const override;

 private:
  ~FontGtk() override;

  PangoFontDescription* font_;
};

}  // namespace nu

#endif  // NATIVEUI_GFX_GTK_FONT_GTK_H_
