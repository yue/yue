// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_LABEL_H_
#define NATIVEUI_LABEL_H_

#include <string>

#include "nativeui/gfx/text.h"
#include "nativeui/view.h"

namespace nu {

class AttributedText;

class NATIVEUI_EXPORT Label : public View {
 public:
  explicit Label(const std::string& text);
  explicit Label(AttributedText* text);

  // View class name.
  static const char kClassName[];

  void SetText(const std::string& text);
  std::string GetText() const;

  void SetAttributedText(AttributedText* text);
  AttributedText* GetAttributedText() const { return text_.get(); }

  // View:
  const char* GetClassName() const override;
  void SetFont(Font* font) override;
  void SetColor(Color color) override;

 protected:
  Label();
  ~Label() override;

 private:
  // Mark the yoga node as dirty.
  void MarkDirty();

  NativeView PlatformCreate();
  void PlatformSetAttributedText(AttributedText* text);

  // False if user has passed an AttributedText.
  bool is_simple_text_ = false;

  // Stored current font and color when showing simple text.
  Color color_;
  scoped_refptr<Font> font_;

  scoped_refptr<AttributedText> text_;
};

}  // namespace nu

#endif  // NATIVEUI_LABEL_H_
