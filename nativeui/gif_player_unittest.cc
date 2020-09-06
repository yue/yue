// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "base/files/file_path.h"
#include "base/path_service.h"
#include "nativeui/nativeui.h"
#include "testing/gtest/include/gtest/gtest.h"

class GifPlayerTest : public testing::Test {
 protected:
  void SetUp() override {
    gif_ = new nu::GifPlayer();
    base::FilePath exe_path;
    base::PathService::Get(base::FILE_EXE, &exe_path);
    base::FilePath dir = exe_path.DirName().DirName().DirName()
                                 .Append(FILE_PATH_LITERAL("nativeui"))
                                 .Append(FILE_PATH_LITERAL("test"))
                                 .Append(FILE_PATH_LITERAL("fixtures"));
    animated_img_ = new nu::Image(
        dir.Append(FILE_PATH_LITERAL("animated.gif")));
    static_img_ = new nu::Image(
        dir.Append(FILE_PATH_LITERAL("static.png")));
  }

  nu::Lifetime lifetime_;
  nu::State state_;
  scoped_refptr<nu::GifPlayer> gif_;
  scoped_refptr<nu::Image> animated_img_;
  scoped_refptr<nu::Image> static_img_;
};

TEST_F(GifPlayerTest, NullImage) {
  EXPECT_FALSE(gif_->IsAnimating());
  gif_->SetImage(nullptr);
  EXPECT_FALSE(gif_->IsAnimating());
}

TEST_F(GifPlayerTest, StaticImage) {
  gif_->SetImage(static_img_.get());
  EXPECT_FALSE(gif_->IsAnimating());
#if defined(OS_WIN) || defined(OS_LINUX)
  EXPECT_FALSE(gif_->IsPlaying());
  EXPECT_FALSE(gif_->CanAnimate());
#endif
  gif_->SetAnimating(true);
  EXPECT_FALSE(gif_->IsAnimating());
}

TEST_F(GifPlayerTest, AnimatedImage) {
  gif_->SetImage(animated_img_.get());
  EXPECT_TRUE(gif_->IsAnimating());
#if defined(OS_WIN) || defined(OS_LINUX)
  EXPECT_TRUE(gif_->IsPlaying());
  EXPECT_TRUE(gif_->CanAnimate());
#endif
  gif_->SetAnimating(false);
  EXPECT_FALSE(gif_->IsAnimating());
}

#if defined(OS_WIN) || defined(OS_LINUX)
TEST_F(GifPlayerTest, StopPlaying) {
  gif_->SetImage(animated_img_.get());
  gif_->SetAnimating(false);
  EXPECT_FALSE(gif_->IsPlaying());
}

TEST_F(GifPlayerTest, HiddenView) {
  gif_->SetImage(animated_img_.get());
  gif_->SetVisible(false);
  EXPECT_FALSE(gif_->IsPlaying());
  gif_->SetVisible(true);
  EXPECT_TRUE(gif_->IsPlaying());
}
#endif
