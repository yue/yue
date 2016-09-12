// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/nativeui.h"
#include "testing/gtest/include/gtest/gtest.h"

class BoxLayoutTest : public testing::Test {
 protected:
  void SetUp() override {
    window_ = new nu::Window(nu::Window::Options());
    layout_ = new nu::BoxLayout(nu::BoxLayout::Vertical);
    content_ = window_->GetContentView();
    content_->SetLayoutManager(layout_.get());
  }

  nu::State state_;
  scoped_refptr<nu::BoxLayout> layout_;
  scoped_refptr<nu::Window> window_;
  nu::Container* content_;
};

nu::View* CreateFixedSizeView(int width, int height) {
  nu::Container* container = new nu::Container;
  container->SetPreferredSize(nu::Size(width, height));
  return container;
}

TEST_F(BoxLayoutTest, MainStartCrossStart) {
  layout_->set_main_axis_alignment(nu::BoxLayout::Start);
  layout_->set_cross_axis_alignment(nu::BoxLayout::Start);
  content_->AddChildView(CreateFixedSizeView(100, 100));
  content_->AddChildView(CreateFixedSizeView(50, 50));
  window_->SetContentBounds(nu::Rect(0, 0, 200, 200));
  EXPECT_EQ(content_->preferred_size(), nu::Size(100, 150));
  EXPECT_EQ(content_->child_at(0)->GetBounds(), nu::Rect(50, 0, 100, 100));
  EXPECT_EQ(content_->child_at(1)->GetBounds(), nu::Rect(50, 100, 50, 50));
}

TEST_F(BoxLayoutTest, MainStartCrossCenter) {
  layout_->set_main_axis_alignment(nu::BoxLayout::Start);
  layout_->set_cross_axis_alignment(nu::BoxLayout::Center);
  content_->AddChildView(CreateFixedSizeView(100, 100));
  content_->AddChildView(CreateFixedSizeView(50, 50));
  window_->SetContentBounds(nu::Rect(0, 0, 200, 200));
  EXPECT_EQ(content_->preferred_size(), nu::Size(100, 150));
  EXPECT_EQ(content_->child_at(0)->GetBounds(), nu::Rect(50, 0, 100, 100));
  EXPECT_EQ(content_->child_at(1)->GetBounds(), nu::Rect(75, 100, 50, 50));
}

TEST_F(BoxLayoutTest, MainStartCrossEnd) {
  layout_->set_main_axis_alignment(nu::BoxLayout::Start);
  layout_->set_cross_axis_alignment(nu::BoxLayout::End);
  content_->AddChildView(CreateFixedSizeView(100, 100));
  content_->AddChildView(CreateFixedSizeView(50, 50));
  window_->SetContentBounds(nu::Rect(0, 0, 200, 200));
  EXPECT_EQ(content_->preferred_size(), nu::Size(100, 150));
  EXPECT_EQ(content_->child_at(0)->GetBounds(), nu::Rect(50, 0, 100, 100));
  EXPECT_EQ(content_->child_at(1)->GetBounds(), nu::Rect(100, 100, 50, 50));
}

TEST_F(BoxLayoutTest, MainStartCrossStretch) {
  layout_->set_main_axis_alignment(nu::BoxLayout::Start);
  layout_->set_cross_axis_alignment(nu::BoxLayout::Stretch);
  content_->AddChildView(CreateFixedSizeView(100, 100));
  content_->AddChildView(CreateFixedSizeView(50, 50));
  window_->SetContentBounds(nu::Rect(0, 0, 200, 200));
  EXPECT_EQ(content_->preferred_size(), nu::Size(100, 150));
  EXPECT_EQ(content_->child_at(0)->GetBounds(), nu::Rect(0, 0, 200, 100));
  EXPECT_EQ(content_->child_at(1)->GetBounds(), nu::Rect(0, 100, 200, 50));
}

TEST_F(BoxLayoutTest, MainCenterCrossStart) {
  layout_->set_main_axis_alignment(nu::BoxLayout::Center);
  layout_->set_cross_axis_alignment(nu::BoxLayout::Start);
  content_->AddChildView(CreateFixedSizeView(100, 100));
  content_->AddChildView(CreateFixedSizeView(50, 50));
  window_->SetContentBounds(nu::Rect(0, 0, 200, 200));
  EXPECT_EQ(content_->preferred_size(), nu::Size(100, 150));
  EXPECT_EQ(content_->child_at(0)->GetBounds(), nu::Rect(50, 25, 100, 100));
  EXPECT_EQ(content_->child_at(1)->GetBounds(), nu::Rect(50, 125, 50, 50));
}

TEST_F(BoxLayoutTest, MainEndCrossStart) {
  layout_->set_main_axis_alignment(nu::BoxLayout::End);
  layout_->set_cross_axis_alignment(nu::BoxLayout::Start);
  content_->AddChildView(CreateFixedSizeView(100, 100));
  content_->AddChildView(CreateFixedSizeView(50, 50));
  window_->SetContentBounds(nu::Rect(0, 0, 200, 200));
  EXPECT_EQ(content_->preferred_size(), nu::Size(100, 150));
  EXPECT_EQ(content_->child_at(0)->GetBounds(), nu::Rect(50, 50, 100, 100));
  EXPECT_EQ(content_->child_at(1)->GetBounds(), nu::Rect(50, 150, 50, 50));
}

TEST_F(BoxLayoutTest, MainStretchCrossStart) {
  layout_->set_main_axis_alignment(nu::BoxLayout::Stretch);
  layout_->set_cross_axis_alignment(nu::BoxLayout::Start);
  content_->AddChildView(CreateFixedSizeView(100, 100));
  content_->AddChildView(CreateFixedSizeView(50, 50));
  window_->SetContentBounds(nu::Rect(0, 0, 200, 200));
  EXPECT_EQ(content_->preferred_size(), nu::Size(100, 150));
  EXPECT_EQ(content_->child_at(0)->GetBounds(), nu::Rect(50, 0, 100, 100));
  EXPECT_EQ(content_->child_at(1)->GetBounds(), nu::Rect(50, 100, 50, 100));
}

TEST_F(BoxLayoutTest, MainStretchCrossStretch) {
  layout_->set_main_axis_alignment(nu::BoxLayout::Stretch);
  layout_->set_cross_axis_alignment(nu::BoxLayout::Stretch);
  content_->AddChildView(CreateFixedSizeView(100, 100));
  content_->AddChildView(CreateFixedSizeView(50, 50));
  window_->SetContentBounds(nu::Rect(0, 0, 200, 200));
  EXPECT_EQ(content_->preferred_size(), nu::Size(100, 150));
  EXPECT_EQ(content_->child_at(0)->GetBounds(), nu::Rect(0, 0, 200, 100));
  EXPECT_EQ(content_->child_at(1)->GetBounds(), nu::Rect(0, 100, 200, 100));
}

TEST_F(BoxLayoutTest, HorizontalMainStartCrossStart) {
  layout_->set_orientation(nu::BoxLayout::Horizontal);
  layout_->set_main_axis_alignment(nu::BoxLayout::Start);
  layout_->set_cross_axis_alignment(nu::BoxLayout::Start);
  content_->AddChildView(CreateFixedSizeView(100, 100));
  content_->AddChildView(CreateFixedSizeView(50, 50));
  window_->SetContentBounds(nu::Rect(0, 0, 200, 200));
  EXPECT_EQ(content_->preferred_size(), nu::Size(150, 100));
  EXPECT_EQ(content_->child_at(0)->GetBounds(), nu::Rect(0, 50, 100, 100));
  EXPECT_EQ(content_->child_at(1)->GetBounds(), nu::Rect(100, 50, 50, 50));
}

TEST_F(BoxLayoutTest, ChildSpacing) {
  layout_->set_child_spacing(10);
  layout_->set_main_axis_alignment(nu::BoxLayout::Center);
  layout_->set_cross_axis_alignment(nu::BoxLayout::Center);
  content_->AddChildView(CreateFixedSizeView(100, 100));
  content_->AddChildView(CreateFixedSizeView(50, 50));
  window_->SetContentBounds(nu::Rect(0, 0, 200, 200));
  EXPECT_EQ(content_->preferred_size(), nu::Size(100, 160));
  EXPECT_EQ(content_->child_at(0)->GetBounds(), nu::Rect(50, 20, 100, 100));
  EXPECT_EQ(content_->child_at(1)->GetBounds(), nu::Rect(75, 130, 50, 50));
}

TEST_F(BoxLayoutTest, ChildSpacingSingle) {
  layout_->set_child_spacing(10);
  layout_->set_main_axis_alignment(nu::BoxLayout::Center);
  layout_->set_cross_axis_alignment(nu::BoxLayout::Center);
  content_->AddChildView(CreateFixedSizeView(50, 50));
  window_->SetContentBounds(nu::Rect(0, 0, 200, 200));
  EXPECT_EQ(content_->preferred_size(), nu::Size(50, 50));
  EXPECT_EQ(content_->child_at(0)->GetBounds(), nu::Rect(75, 75, 50, 50));
}

TEST_F(BoxLayoutTest, InnerPaddingMainCenterCrossCenter) {
  layout_->set_inner_padding(nu::Insets(1, 2, 3, 4));
  layout_->set_main_axis_alignment(nu::BoxLayout::Center);
  layout_->set_cross_axis_alignment(nu::BoxLayout::Center);
  content_->AddChildView(CreateFixedSizeView(100, 100));
  content_->AddChildView(CreateFixedSizeView(50, 50));
  window_->SetContentBounds(nu::Rect(0, 0, 200, 200));
  EXPECT_EQ(content_->preferred_size(), nu::Size(106, 154));
  EXPECT_EQ(content_->child_at(0)->GetBounds(), nu::Rect(49, 24, 100, 100));
  EXPECT_EQ(content_->child_at(1)->GetBounds(), nu::Rect(74, 124, 50, 50));
}

TEST_F(BoxLayoutTest, InnerPaddingMainEndCrossEnd) {
  layout_->set_inner_padding(nu::Insets(1, 2, 3, 4));
  layout_->set_main_axis_alignment(nu::BoxLayout::End);
  layout_->set_cross_axis_alignment(nu::BoxLayout::End);
  content_->AddChildView(CreateFixedSizeView(100, 100));
  content_->AddChildView(CreateFixedSizeView(50, 50));
  window_->SetContentBounds(nu::Rect(0, 0, 200, 200));
  EXPECT_EQ(content_->preferred_size(), nu::Size(106, 154));
  EXPECT_EQ(content_->child_at(0)->GetBounds(), nu::Rect(49, 47, 100, 100));
  EXPECT_EQ(content_->child_at(1)->GetBounds(), nu::Rect(99, 147, 50, 50));
}

TEST_F(BoxLayoutTest, Flex) {
  layout_->set_main_axis_alignment(nu::BoxLayout::Start);
  layout_->set_cross_axis_alignment(nu::BoxLayout::Start);
  content_->AddChildView(CreateFixedSizeView(100, 100));
  content_->AddChildView(CreateFixedSizeView(50, 50));
  layout_->SetFlexForView(content_->child_at(1), 1);
  window_->SetContentBounds(nu::Rect(0, 0, 200, 200));
  EXPECT_EQ(content_->preferred_size(), nu::Size(100, 150));
  EXPECT_EQ(content_->child_at(0)->GetBounds(), nu::Rect(50, 0, 100, 100));
  EXPECT_EQ(content_->child_at(1)->GetBounds(), nu::Rect(50, 100, 50, 100));
}

TEST_F(BoxLayoutTest, ChildRemoved) {
  layout_->set_main_axis_alignment(nu::BoxLayout::Start);
  layout_->set_cross_axis_alignment(nu::BoxLayout::Start);
  content_->AddChildView(CreateFixedSizeView(100, 100));
  content_->AddChildView(CreateFixedSizeView(50, 50));
  layout_->SetFlexForView(content_->child_at(1), 1);
  window_->SetContentBounds(nu::Rect(0, 0, 200, 200));
  content_->RemoveChildView(content_->child_at(1));
  EXPECT_EQ(content_->preferred_size(), nu::Size(100, 100));
  EXPECT_EQ(content_->child_at(0)->GetBounds(), nu::Rect(50, 0, 100, 100));
}

TEST_F(BoxLayoutTest, IgnoreHiddenChild) {
  layout_->set_main_axis_alignment(nu::BoxLayout::Center);
  layout_->set_cross_axis_alignment(nu::BoxLayout::Center);
  content_->AddChildView(CreateFixedSizeView(100, 100));
  content_->AddChildView(CreateFixedSizeView(50, 50));
  content_->child_at(1)->SetVisible(false);
  window_->SetContentBounds(nu::Rect(0, 0, 200, 200));
  EXPECT_EQ(content_->preferred_size(), nu::Size(100, 100));
  EXPECT_EQ(content_->child_at(0)->GetBounds(), nu::Rect(50, 50, 100, 100));
}
