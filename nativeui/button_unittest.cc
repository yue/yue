// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/nativeui.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/css-layout/CSSLayout/CSSLayout.h"

class ButtonTest : public testing::Test {
 protected:
  void SetUp() override {
  }

  nu::State state_;
};

TEST_F(ButtonTest, Title) {
  scoped_refptr<nu::Button> button = new nu::Button("title");
  EXPECT_EQ(button->GetTitle(), "title");
  button->SetTitle("title2");
  EXPECT_EQ(button->GetTitle(), "title2");
}

TEST_F(ButtonTest, SetBounds) {
  scoped_refptr<nu::Button> button = new nu::Button("title");
  nu::RectF bounds(0, 0, 300, 400);
  button->SetBounds(bounds);
  EXPECT_EQ(button->GetBounds(), bounds);
}

TEST_F(ButtonTest, UpdateStyle) {
  scoped_refptr<nu::Button> button = new nu::Button("title");
  float width = CSSNodeStyleGetMinWidth(button->node());
  float height = CSSNodeStyleGetMinHeight(button->node());
  button->SetTitle("a long long title");
  EXPECT_LT(width, CSSNodeStyleGetMinWidth(button->node()));
  EXPECT_EQ(height, CSSNodeStyleGetMinHeight(button->node()));
}

TEST_F(ButtonTest, CheckBox) {
  scoped_refptr<nu::Button> button =
      new nu::Button("title", nu::Button::CheckBox);
  EXPECT_FALSE(button->IsChecked());
  button->SetChecked(true);
  EXPECT_TRUE(button->IsChecked());
}

// TODO(zcbenz): Fix the platform differences on radio buttons.
#if !defined(OS_LINUX)
TEST_F(ButtonTest, Radio) {
  scoped_refptr<nu::Button> button =
      new nu::Button("title", nu::Button::Radio);
  EXPECT_FALSE(button->IsChecked());
  button->SetChecked(true);
  EXPECT_TRUE(button->IsChecked());
}
#endif

TEST_F(ButtonTest, RadioGroup) {
  scoped_refptr<nu::Container> container = new nu::Container;
  container->SetBounds(nu::RectF(0, 0, 100, 100));
  auto* r1 = new nu::Button("r1", nu::Button::Radio);
  auto* r2 = new nu::Button("r2", nu::Button::Radio);
  auto* r3 = new nu::Button("r3", nu::Button::Radio);
  container->AddChildView(r1);
  container->AddChildView(r2);
  container->AddChildView(r3);
#if !defined(OS_LINUX)
  EXPECT_FALSE(r1->IsChecked());
  EXPECT_FALSE(r2->IsChecked());
  EXPECT_FALSE(r3->IsChecked());
#endif
  r1->SetChecked(true);
  EXPECT_TRUE(r1->IsChecked());
  EXPECT_FALSE(r2->IsChecked());
  EXPECT_FALSE(r3->IsChecked());
  r2->SetChecked(true);
  EXPECT_FALSE(r1->IsChecked());
  EXPECT_TRUE(r2->IsChecked());
  EXPECT_FALSE(r3->IsChecked());
}
