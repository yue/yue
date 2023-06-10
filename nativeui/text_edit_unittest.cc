// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/nativeui.h"
#include "testing/gtest/include/gtest/gtest.h"

class TextEditTest : public testing::Test {
 protected:
  void SetUp() override {
    edit_ = new nu::TextEdit;
    // On macOS a window is required for undoManager to work.
    window_ = new nu::Window(nu::Window::Options());
    window_->SetContentView(edit_.get());
  }

  nu::Lifetime lifetime_;
  nu::State state_;
  scoped_refptr<nu::TextEdit> edit_;
  scoped_refptr<nu::Window> window_;
};

TEST_F(TextEditTest, Disable) {
  edit_->SetEnabled(false);
  EXPECT_EQ(edit_->IsEnabled(), false);
}

TEST_F(TextEditTest, SetText) {
  edit_->SetText("test");
  EXPECT_EQ(edit_->GetText(), "test");
}

TEST_F(TextEditTest, EmptySelection) {
  edit_->SetText("test");
  EXPECT_EQ(edit_->GetSelectionRange(), std::make_tuple(4, 4));
}

TEST_F(TextEditTest, SelectRange) {
  edit_->SetText("abcdedfghijklmn");
  edit_->SelectRange(1, 4);
  EXPECT_EQ(edit_->GetSelectionRange(), std::make_tuple(1, 4));
  EXPECT_EQ(edit_->GetTextInRange(1, 4), "bcd");
}

TEST_F(TextEditTest, InsertText) {
  edit_->SetText("d");
  edit_->SelectRange(0, 0);
  edit_->InsertText("ab");
  EXPECT_EQ(edit_->GetText(), "abd");
  edit_->InsertTextAt("c", 2);
  EXPECT_EQ(edit_->GetText(), "abcd");
  EXPECT_EQ(edit_->GetSelectionRange(), std::make_tuple(3, 3));
}

TEST_F(TextEditTest, Clipboard) {
  edit_->SetText("abcde");
  edit_->SelectRange(0, 1);
  edit_->Cut();
  EXPECT_EQ(edit_->GetText(), "bcde");
  edit_->SelectRange(4, 4);
  edit_->Paste();
  EXPECT_EQ(edit_->GetText(), "bcdea");
  edit_->SelectRange(0, 1);
  edit_->Copy();
  EXPECT_EQ(edit_->GetText(), "bcdea");
  edit_->SelectRange(5, 5);
  edit_->Paste();
  EXPECT_EQ(edit_->GetText(), "bcdeab");
}

TEST_F(TextEditTest, Delete) {
  edit_->SetText("abcde");
  edit_->SelectRange(0, 1);
  edit_->Delete();
  EXPECT_EQ(edit_->GetText(), "bcde");
  edit_->DeleteRange(0, 1);
  EXPECT_EQ(edit_->GetText(), "cde");
}

TEST_F(TextEditTest, Undo) {
  EXPECT_EQ(edit_->CanUndo(), false);
  EXPECT_EQ(edit_->CanRedo(), false);
  edit_->InsertTextAt("a", 0);
  EXPECT_EQ(edit_->CanUndo(), true);
  EXPECT_EQ(edit_->CanRedo(), false);
  edit_->InsertTextAt("b", 0);
  edit_->Undo();
#if !defined(OS_MAC)
  // On macOS an undo operation would undo all.
  EXPECT_EQ(edit_->GetText(), "a");
  EXPECT_EQ(edit_->CanUndo(), true);
#endif
  EXPECT_EQ(edit_->CanRedo(), true);
  edit_->Redo();
  EXPECT_EQ(edit_->GetText(), "ba");
  EXPECT_EQ(edit_->CanUndo(), true);
  EXPECT_EQ(edit_->CanRedo(), false);
}
