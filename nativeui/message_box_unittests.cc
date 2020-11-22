// Copyright 2020 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/nativeui.h"
#include "testing/gtest/include/gtest/gtest.h"

class MessageBoxTest : public testing::Test {
 protected:
  void SetUp() override {
    window_ = new nu::Window(nu::Window::Options());
    window_->SetVisible(true);
    box_ = new nu::MessageBox();
    box_->AddButton("OK", 1);
    box_->AddButton("Cancel", 2);
  }

  nu::Lifetime lifetime_;
  nu::State state_;
  scoped_refptr<nu::Window> window_;
  scoped_refptr<nu::MessageBox> box_;
};

TEST_F(MessageBoxTest, Run) {
  nu::MessageLoop::PostTask([&]() {
    box_->Close();
  });
  int response = box_->Run();
  EXPECT_EQ(response, -1);
}

TEST_F(MessageBoxTest, SetCancelResponse) {
  box_->SetCancelResponse(2);
  nu::MessageLoop::PostTask([&]() {
    box_->Close();
  });
  int response = box_->Run();
  EXPECT_EQ(response, 2);
}

TEST_F(MessageBoxTest, ReleaseBeforeResponse) {
  box_->SetCancelResponse(2);
  int res = 0;
  box_->on_response.Connect([&res](nu::MessageBox*, int response) {
    res = response;
    nu::MessageLoop::Quit();
  });
  box_->ShowForWindow(window_.get());
  nu::MessageBox* raw_ref = box_.get();
  box_ = nullptr;
  nu::MessageLoop::PostTask([&]() {
    raw_ref->Close();
  });
  nu::MessageLoop::Run();
  EXPECT_EQ(res, 2);
}
