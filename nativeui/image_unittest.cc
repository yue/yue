// Copyright 2023 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "base/files/file_path.h"
#include "base/path_service.h"
#include "nativeui/nativeui.h"
#include "testing/gtest/include/gtest/gtest.h"

class ImageTest : public testing::Test {
 protected:
  void SetUp() override {
    base::FilePath exe_path;
    base::PathService::Get(base::FILE_EXE, &exe_path);
    base::FilePath dir = exe_path.DirName().DirName().DirName()
                                 .Append(FILE_PATH_LITERAL("nativeui"))
                                 .Append(FILE_PATH_LITERAL("test"))
                                 .Append(FILE_PATH_LITERAL("fixtures"));
    static_img_ = new nu::Image(dir.Append(FILE_PATH_LITERAL("static.png")));
    hidpi_img_ = new nu::Image(dir.Append(FILE_PATH_LITERAL("hidpi@2x.png")));
  }

  nu::State state_;
  scoped_refptr<nu::Image> static_img_;
  scoped_refptr<nu::Image> hidpi_img_;
};

TEST_F(ImageTest, HiDPI) {
  EXPECT_EQ(hidpi_img_->GetSize(), nu::SizeF(5, 5));
  EXPECT_EQ(hidpi_img_->GetScaleFactor(), 2.f);
}

TEST_F(ImageTest, Resize) {
  scoped_refptr<nu::Image> r1 = static_img_->Resize(nu::SizeF(10, 10), 1);
  EXPECT_EQ(r1->GetSize(), nu::SizeF(10, 10));
  EXPECT_EQ(r1->GetScaleFactor(), 1);
  scoped_refptr<nu::Image> r2 = hidpi_img_->Resize(nu::SizeF(20, 20), 2);
  EXPECT_EQ(r2->GetSize(), nu::SizeF(20, 20));
  EXPECT_EQ(r2->GetScaleFactor(), 2);
}

TEST_F(ImageTest, ToPNG) {
  scoped_refptr<nu::Image> png = new nu::Image(hidpi_img_->ToPNG(), 1);
  EXPECT_EQ(png->GetSize(), nu::SizeF(10, 10));
  EXPECT_EQ(png->GetScaleFactor(), 1);
}

TEST_F(ImageTest, ToJPEG) {
  scoped_refptr<nu::Image> jpg = new nu::Image(hidpi_img_->ToJPEG(80), 1);
  EXPECT_EQ(jpg->GetSize(), nu::SizeF(10, 10));
  EXPECT_EQ(jpg->GetScaleFactor(), 1);
}

TEST_F(ImageTest, Clear) {
  EXPECT_FALSE(static_img_->IsEmpty());
  static_img_->Clear();
  EXPECT_TRUE(static_img_->IsEmpty());
}
