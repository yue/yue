// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/nativeui.h"
#include "testing/gtest/include/gtest/gtest.h"

class ClipboardTest : public testing::Test {
 protected:
  void SetUp() override {
    app_ = nu::App::GetCurrent();
    clipboard_ = app_->GetClipboard();
  }

  nu::Lifetime lifetime_;
  nu::State state_;

  nu::App* app_;
  nu::Clipboard* clipboard_;
};

TEST_F(ClipboardTest, Types) {
  EXPECT_TRUE(app_->GetClipboard(nu::Clipboard::Type::CopyPaste)->GetNative());
#if defined(OS_MACOSX)
  EXPECT_TRUE(app_->GetClipboard(nu::Clipboard::Type::Drag)->GetNative());
  EXPECT_TRUE(app_->GetClipboard(nu::Clipboard::Type::Find)->GetNative());
  EXPECT_TRUE(app_->GetClipboard(nu::Clipboard::Type::Font)->GetNative());
#elif defined(OS_LINUX)
  EXPECT_TRUE(app_->GetClipboard(nu::Clipboard::Type::Selection)->GetNative());
#endif
}

TEST_F(ClipboardTest, Clear) {
  clipboard_->SetText("some text");
  clipboard_->Clear();
  EXPECT_EQ(clipboard_->GetText(), "");
}

TEST_F(ClipboardTest, Text) {
  clipboard_->SetText("some text");
  EXPECT_EQ(clipboard_->GetText(), "some text");
}
