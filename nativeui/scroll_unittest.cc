// Copyright 2021 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/nativeui.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

nu::SizeF kScrollSize = { 400, 400 };
nu::SizeF kContentSize = { 800, 800 };

}  // namespace

class ScrollTest : public testing::Test {
 protected:
  void SetUp() override {
    window_ = new nu::Window(nu::Window::Options());
    scroll_ = new nu::Scroll();
#if defined(OS_WIN)
    scroll_->SetScrollbarPolicy(nu::Scroll::Policy::Never,
                                nu::Scroll::Policy::Never);
#endif
    scroll_->SetContentSize(kContentSize);
    window_->SetContentView(scroll_);
    window_->SetContentSize(kScrollSize);
  }

  nu::Lifetime lifetime_;
  nu::State state_;
  scoped_refptr<nu::Window> window_;
  scoped_refptr<nu::Scroll> scroll_;
};

TEST_F(ScrollTest, ContentSize) {
  EXPECT_EQ(scroll_->GetContentSize(), kContentSize);
  auto range = scroll_->GetMaximumScrollPosition();
  EXPECT_EQ(std::get<0>(range), kContentSize.width() - kScrollSize.width());
  EXPECT_EQ(std::get<1>(range), kContentSize.height() - kScrollSize.height());
}

TEST_F(ScrollTest, Scroll) {
  scroll_->SetScrollPosition(0, 0);
  EXPECT_EQ(scroll_->GetScrollPosition(), std::make_tuple(0, 0));
  scroll_->SetScrollPosition(100, 100);
  EXPECT_EQ(scroll_->GetScrollPosition(), std::make_tuple(100, 100));
  auto range = scroll_->GetMaximumScrollPosition();
  scroll_->SetScrollPosition(std::get<0>(range), std::get<1>(range));
  EXPECT_EQ(scroll_->GetScrollPosition(), range);
}
