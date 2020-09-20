// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GROUP_H_
#define NATIVEUI_GROUP_H_

#include <string>

#include "nativeui/view.h"

namespace nu {

class NATIVEUI_EXPORT Group : public View {
 public:
  explicit Group(const std::string& title);

  // View class name.
  static const char kClassName[];

  // View:
  const char* GetClassName() const override;
  SizeF GetMinimumSize() const override;

  void SetContentView(scoped_refptr<View> view);
  View* GetContentView() const;

  void SetTitle(const std::string& title);
  std::string GetTitle() const;

 protected:
  ~Group() override;

  // Get the spaces taken by the border of group.
  SizeF GetBorderSize() const;

  // Following platform implementations should only be called by wrappers.
  void PlatformInit();
  void PlatformSetContentView(View* container);

 private:
  scoped_refptr<View> content_view_;
};

}  // namespace nu

#endif  // NATIVEUI_GROUP_H_
