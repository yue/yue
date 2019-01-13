// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/slider.h"

#include <commctrl.h>

#include "nativeui/win/subwin_view.h"

namespace nu {

namespace {

class SliderImpl : public SubwinView {
 public:
  explicit SliderImpl(Slider* delegate)
      : SubwinView(delegate, TRACKBAR_CLASS, WS_CHILD | WS_VISIBLE) {
    SetTransparentBackground();
  }

 protected:
  // SubwinView:
  LRESULT OnNotify(int, LPNMHDR pnmh) override {
    Slider* slider = static_cast<Slider*>(delegate());
    if (pnmh->code == NM_RELEASEDCAPTURE)
      slider->on_sliding_complete.Emit(slider);
    return 0;
  }

  void OnHScroll(UINT code, UINT pos) override {
    Slider* slider = static_cast<Slider*>(delegate());
    slider->on_value_change.Emit(slider);
  }
};

}  // namespace

Slider::Slider() {
  auto* slider = new SliderImpl(this);
  ::SendMessage(slider->hwnd(), TBM_SETRANGE, TRUE, MAKELPARAM(0, 100));
  ::SendMessage(slider->hwnd(), TBM_SETPAGESIZE, TRUE, 1);
  TakeOverView(slider);
  UpdateDefaultStyle();
}

Slider::~Slider() {
}

void Slider::SetValue(float value) {
  auto* slider = static_cast<SliderImpl*>(GetNative());
  ::SendMessage(slider->hwnd(), TBM_SETPOS, TRUE, value);
}

float Slider::GetValue() const {
  auto* slider = static_cast<SliderImpl*>(GetNative());
  return static_cast<float>(
      ::SendMessage(slider->hwnd(), TBM_GETPOS, 0, 0L));
}

void Slider::SetStep(float step) {
  auto* slider = static_cast<SliderImpl*>(GetNative());
  ::SendMessage(slider->hwnd(), TBM_SETPAGESIZE, TRUE, step);
}

float Slider::GetStep() const {
  auto* slider = static_cast<SliderImpl*>(GetNative());
  return static_cast<float>(
      ::SendMessage(slider->hwnd(), TBM_GETPAGESIZE, 0, 0L));
}

void Slider::SetMaximumValue(float max) {
  auto* slider = static_cast<SliderImpl*>(GetNative());
  ::SendMessage(slider->hwnd(), TBM_SETRANGEMAX, TRUE, max);
}

float Slider::GetMaximumValue() const {
  auto* slider = static_cast<SliderImpl*>(GetNative());
  return static_cast<float>(
      ::SendMessage(slider->hwnd(), TBM_GETRANGEMAX, 0, 0L));
}

void Slider::SetMinimumValue(float min) {
  auto* slider = static_cast<SliderImpl*>(GetNative());
  ::SendMessage(slider->hwnd(), TBM_SETRANGEMIN, TRUE, min);
}

float Slider::GetMinimumValue() const {
  auto* slider = static_cast<SliderImpl*>(GetNative());
  return static_cast<float>(
      ::SendMessage(slider->hwnd(), TBM_GETRANGEMIN, 0, 0L));
}

SizeF Slider::GetMinimumSize() const {
  return SizeF(0, 20);
}

}  // namespace nu
