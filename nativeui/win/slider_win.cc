// Copyright 2019 Cheng Zhao. All rights reserved.
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
    set_focusable(true);
    SetTransparentBackground();
    ::SendMessage(hwnd(), TBM_SETRANGE, TRUE, MAKELPARAM(0, base_));
    ::SendMessage(hwnd(), TBM_SETPAGESIZE, TRUE, base_ / max_);
  }

  void SetValue(float value) {
    int pos = value * (base_ / (max_ - min_));
    ::SendMessage(hwnd(), TBM_SETPOS, TRUE, pos);
  }

  float GetValue() const {
    int pos = ::SendMessage(hwnd(), TBM_GETPOS, 0, 0L);
    return pos * ((max_ - min_) / base_);
  }

  void SetStep(float step) {
    step_ = step;
    ::SendMessage(hwnd(), TBM_SETPAGESIZE, TRUE,
                  step_ * (base_ / (max_ - min_)));
  }

  float GetStep() const {
    return step_;
  }

  void SetRange(float min, float max) {
    min_ = min;
    max_ = max;
  }

  std::tuple<float, float> GetRange() const {
    return std::make_tuple(min_, max_);
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

 private:
  float min_ = 0.;
  float max_ = 100.l;
  float step_ = 1.;

  // Use a large range since Windows does not support discrete range.
  const int base_ = 10000;
};

}  // namespace

NativeView Slider::PlatformCreate() {
  return new SliderImpl(this);
}

void Slider::SetValue(float value) {
  auto* slider = static_cast<SliderImpl*>(GetNative());
  slider->SetValue(value);
}

float Slider::GetValue() const {
  auto* slider = static_cast<SliderImpl*>(GetNative());
  return slider->GetValue();
}

void Slider::SetStep(float step) {
  auto* slider = static_cast<SliderImpl*>(GetNative());
  slider->SetStep(step);
}

float Slider::GetStep() const {
  auto* slider = static_cast<SliderImpl*>(GetNative());
  return slider->GetStep();
}

void Slider::SetRange(float min, float max) {
  auto* slider = static_cast<SliderImpl*>(GetNative());
  slider->SetRange(min, max);
}

std::tuple<float, float> Slider::GetRange() const {
  auto* slider = static_cast<SliderImpl*>(GetNative());
  return slider->GetRange();
}

SizeF Slider::GetMinimumSize() const {
  return SizeF(0, 20);
}

}  // namespace nu
