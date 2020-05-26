// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/nativeui.h"
#include "testing/gtest/include/gtest/gtest.h"

class WindowTest : public testing::Test {
 protected:
  void SetUp() override {
    window_ = new nu::Window(nu::Window::Options());
  }

  nu::Lifetime lifetime_;
  nu::State state_;
  scoped_refptr<nu::Window> window_;
};

TEST_F(WindowTest, Bounds) {
  nu::SizeF size(124, 456);
  window_->SetContentSize(size);
  EXPECT_EQ(window_->GetContentSize(), size);
  nu::RectF bounds = window_->GetBounds();
  window_->SetBounds(bounds);
  EXPECT_EQ(window_->GetBounds(), bounds);
}

TEST_F(WindowTest, FramelessWindowBounds) {
  nu::Window::Options options;
  options.frame = false;
  window_ = new nu::Window(options);
  nu::SizeF size(123, 456);
  window_->SetContentSize(size);
  EXPECT_EQ(window_->GetContentSize(), size);
  nu::RectF bounds = window_->GetBounds();
  window_->SetBounds(bounds);
  EXPECT_EQ(window_->GetBounds(), bounds);
}

TEST_F(WindowTest, TransparentWindow) {
  nu::Window::Options options;
  options.frame = false;
  options.transparent = true;
  window_ = new nu::Window(options);
  EXPECT_EQ(window_->IsResizable(), false);
  EXPECT_EQ(window_->IsMaximizable(), false);
}

TEST_F(WindowTest, ContentView) {
  EXPECT_NE(window_->GetContentView(), nullptr);
  scoped_refptr<nu::Container> view(new nu::Container);
  window_->SetContentView(view.get());
  EXPECT_EQ(window_->GetContentView(), view.get());
}

TEST_F(WindowTest, ContentViewWindow) {
  scoped_refptr<nu::Container> view(new nu::Container);
  window_->SetContentView(view.get());
  EXPECT_EQ(view->GetWindow(), window_.get());
  EXPECT_EQ(view->GetParent(), nullptr);
  window_ = nullptr;
  EXPECT_EQ(view->GetWindow(), nullptr);
  EXPECT_EQ(view->GetParent(), nullptr);
}

TEST_F(WindowTest, OnClose) {
  bool closed = false;
  window_->on_close.Connect([&closed](nu::Window*) { closed = true; });
  window_->Close();
  EXPECT_EQ(closed, true);
}

TEST_F(WindowTest, ShouldClose) {
  bool closed = false;
  window_->on_close.Connect([&closed](nu::Window*) { closed = true; });
  window_->should_close = [](nu::Window*) { return false; };
  window_->Close();
  EXPECT_EQ(closed, false);
  window_->should_close = nullptr;
  window_->Close();
  EXPECT_EQ(closed, true);
}

TEST_F(WindowTest, Visible) {
  window_->SetVisible(true);
  ASSERT_TRUE(window_->IsVisible());
  window_->SetVisible(false);
  ASSERT_FALSE(window_->IsVisible());
}

TEST_F(WindowTest, Resizable) {
  nu::SizeF size(124, 456);
  window_->SetContentSize(size);
  nu::RectF bounds = window_->GetBounds();
  EXPECT_EQ(window_->IsResizable(), true);
  window_->SetResizable(false);
  EXPECT_EQ(window_->IsResizable(), false);
  EXPECT_EQ(window_->GetBounds(), bounds);
  EXPECT_EQ(window_->GetContentSize(), size);
  window_->SetResizable(true);
  EXPECT_EQ(window_->GetBounds(), bounds);
  EXPECT_EQ(window_->GetContentSize(), size);
}

TEST_F(WindowTest, VisibleWindowResizable) {
  nu::SizeF size(124, 456);
  window_->SetContentSize(size);
  EXPECT_EQ(window_->GetContentSize(), size);
  window_->SetResizable(false);
  EXPECT_EQ(window_->GetContentSize(), size);
  window_->SetResizable(true);
  EXPECT_EQ(window_->GetContentSize(), size);
}

TEST_F(WindowTest, FramelessWindowResizable) {
  nu::Window::Options options;
  options.frame = false;
  window_ = new nu::Window(options);
  nu::SizeF size(123, 456);
  window_->SetContentSize(size);
  EXPECT_EQ(window_->GetContentSize(), size);
  window_->SetResizable(false);
  EXPECT_EQ(window_->GetContentSize(), size);
  window_->SetResizable(true);
  EXPECT_EQ(window_->GetContentSize(), size);
}

TEST_F(WindowTest, TransparentWindowResizable) {
#if defined(OS_LINUX)
  // This test is weird on Azure Pipelines.
  if (::getenv("AZURE_PIPELINES")) return;
#endif
  nu::Window::Options options;
  options.frame = false;
  options.transparent = true;
  window_ = new nu::Window(options);
  nu::SizeF size(123, 456);
  window_->SetContentSize(size);
  EXPECT_EQ(window_->GetContentSize(), size);
  window_->SetResizable(false);
  EXPECT_EQ(window_->GetContentSize(), size);
  window_->SetResizable(true);
  EXPECT_EQ(window_->GetContentSize(), size);
}

TEST_F(WindowTest, ChildWindow) {
  EXPECT_TRUE(window_->GetChildWindows().empty());
  scoped_refptr<nu::Window> child = new nu::Window(nu::Window::Options());
  EXPECT_EQ(child->GetParentWindow(), nullptr);
  window_->AddChildWindow(child.get());
  EXPECT_EQ(child->GetParentWindow(), window_.get());
  EXPECT_FALSE(window_->GetChildWindows().empty());
  window_->RemoveChildWindow(child.get());
  EXPECT_EQ(child->GetParentWindow(), nullptr);
  EXPECT_TRUE(window_->GetChildWindows().empty());
}

TEST_F(WindowTest, ParentWindowCloseChild) {
  scoped_refptr<nu::Window> child = new nu::Window(nu::Window::Options());
  window_->AddChildWindow(child.get());
  bool closed = false;
  child->on_close.Connect([&closed](nu::Window*) { closed = true; });
  window_->Close();
  EXPECT_EQ(closed, true);
}
