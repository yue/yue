// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_SCROLL_H_
#define NATIVEUI_SCROLL_H_

#include <tuple>

#include "nativeui/view.h"

namespace nu {

class NATIVEUI_EXPORT Scroll : public View {
 public:
  Scroll();

  // View class name.
  static const char kClassName[];

  void SetContentView(View* view);
  View* GetContentView() const;

  void SetContentSize(const SizeF& size);
  SizeF GetContentSize() const;

  enum class Policy {
    Always,
    Never,
    Automatic,
  };
  void SetScrollbarPolicy(Policy h_policy, Policy v_policy);
  std::tuple<Policy, Policy> GetScrollbarPolicy() const;

  // View:
  const char* GetClassName() const override;

 protected:
  ~Scroll() override;

  // Following platform implementations should only be called by wrappers.
  void PlatformInit();
  void PlatformSetContentView(View* container);

 private:
  scoped_refptr<View> content_view_;
};

}  // namespace nu

#endif  // NATIVEUI_SCROLL_H_
