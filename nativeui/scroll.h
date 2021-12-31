// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_SCROLL_H_
#define NATIVEUI_SCROLL_H_

#include <tuple>

#include "nativeui/view.h"

#if defined(OS_LINUX)
// X11 headers define macros for these function names which screw with us.
#if defined(Always)
#undef Always
#endif
#endif

namespace nu {

class NATIVEUI_EXPORT Scroll : public View {
 public:
  Scroll();

  // View class name.
  static const char kClassName[];

  void SetContentView(scoped_refptr<View> view);
  View* GetContentView() const;

  void SetContentSize(const SizeF& size);
  SizeF GetContentSize() const;
  void SetScrollPosition(float horizon, float vertical);
  std::tuple<float, float> GetScrollPosition() const;
  std::tuple<float, float> GetMaximumScrollPosition() const;

#if !defined(OS_WIN)
  void SetOverlayScrollbar(bool overlay);
  bool IsOverlayScrollbar() const;
#endif

  enum class Policy {
    Always,
    Never,
    Automatic,
  };
  void SetScrollbarPolicy(Policy h_policy, Policy v_policy);
  std::tuple<Policy, Policy> GetScrollbarPolicy() const;

#if defined(OS_MACOSX)
  enum class Elasticity {
    Automatic = 0,  // NSScrollElasticityAutomatic = 0
    None = 1,       // NSScrollElasticityNone = 1
    Allowed = 2,    // NSScrollElasticityAllowed = 2
  };
  void SetScrollElasticity(Elasticity h, Elasticity v);
  std::tuple<Elasticity, Elasticity> GetScrollElasticity() const;
#endif

  // View:
  const char* GetClassName() const override;

  // Events.
  Signal<bool(Scroll*)> on_scroll;

 protected:
  ~Scroll() override;

  // Following platform implementations should only be called by wrappers.
  void PlatformInit();
  void PlatformSetContentView(View* container);

  enum { kOnScroll };

  // SignalDelegate:
  void OnConnect(int identifier) override;

 private:
  void SubscribeOnScroll();

#if defined(OS_LINUX)
  ulong h_signal_ = 0;
  ulong v_signal_ = 0;
#endif

  scoped_refptr<View> content_view_;
};

}  // namespace nu

#endif  // NATIVEUI_SCROLL_H_
