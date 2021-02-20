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
  explicit Label(const std::string& text = "");
  explicit Label(scoped_refptr<AttributedText> text);

  // View class name.
  static const char kClassName[];

  void SetText(const std::string& text);
  std::string GetText() const;
  void SetAlign(TextAlign align);
  void SetVAlign(TextAlign align);

  void SetAttributedText(scoped_refptr<AttributedText> text);
  AttributedText* GetAttributedText() const { return text_.get(); }

  // Internal: Make sure the label is using system text color.
  void UpdateColor();

  // View:
  const char* GetClassName() const override;
  void SetFont(scoped_refptr<Font> font) override;
  void SetColor(Color color) override;

 protected:
  ~Label() override;

 private:
  void Init();

  // Mark the yoga node as dirty.
  void MarkDirty();

  NativeView PlatformCreate();

  scoped_refptr<AttributedText> text_;

  bool use_system_color_;
  Color system_color_;
};

}  // namespace nu

#endif  // NATIVEUI_LABEL_H_
