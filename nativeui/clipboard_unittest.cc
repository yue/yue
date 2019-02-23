// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "base/files/file_path.h"
#include "base/path_service.h"
#include "nativeui/nativeui.h"
#include "testing/gtest/include/gtest/gtest.h"

class ClipboardTest : public testing::Test {
 protected:
  using Data = nu::Clipboard::Data;

  void SetUp() override {
    clipboard_ = nu::Clipboard::Get();
    clipboard_->Clear();

    base::FilePath exe_path;
    PathService::Get(base::FILE_EXE, &exe_path);
    image_path_ = exe_path.DirName().DirName().DirName()
                          .Append(FILE_PATH_LITERAL("nativeui"))
                          .Append(FILE_PATH_LITERAL("test"))
                          .Append(FILE_PATH_LITERAL("fixtures"))
                          .Append(FILE_PATH_LITERAL("static.png"));
  }

  nu::Lifetime lifetime_;
  nu::State state_;

  nu::Clipboard* clipboard_;
  base::FilePath image_path_;
};

TEST_F(ClipboardTest, Types) {
  EXPECT_TRUE(nu::Clipboard::Get(nu::Clipboard::Type::CopyPaste)->GetNative());
#if defined(OS_MACOSX)
  EXPECT_TRUE(nu::Clipboard::Get(nu::Clipboard::Type::Drag)->GetNative());
  EXPECT_TRUE(nu::Clipboard::Get(nu::Clipboard::Type::Find)->GetNative());
  EXPECT_TRUE(nu::Clipboard::Get(nu::Clipboard::Type::Font)->GetNative());
#elif defined(OS_LINUX)
  EXPECT_TRUE(nu::Clipboard::Get(nu::Clipboard::Type::Selection)->GetNative());
#endif
}

TEST_F(ClipboardTest, DataMovable) {
  Data text(Data::Type::Text, "text");
  EXPECT_EQ(text.type(), Data::Type::Text);
  EXPECT_EQ(text.str(), "text");

  Data other(std::move(text));
  EXPECT_EQ(text.type(), Data::Type::Text);
  EXPECT_EQ(text.str(), "");
  EXPECT_EQ(other.type(), Data::Type::Text);
  EXPECT_EQ(other.str(), "text");

  Data image(new nu::Image);
  EXPECT_EQ(image.type(), Data::Type::Image);

  other = std::move(image);
  EXPECT_EQ(other.type(), Data::Type::Image);
  EXPECT_TRUE(other.image());
  EXPECT_EQ(image.type(), Data::Type::Image);
  EXPECT_FALSE(image.image());
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

TEST_F(ClipboardTest, HTML) {
  std::string html = "<strong>text 文字</strong>";
  std::vector<Data> objects;
  objects.emplace_back(Data::Type::HTML, html);
  clipboard_->SetData(std::move(objects));
  EXPECT_TRUE(clipboard_->IsDataAvailable(Data::Type::HTML));

  Data data = clipboard_->GetData(Data::Type::HTML);
  EXPECT_EQ(data.type(), Data::Type::HTML);
  EXPECT_EQ(data.str(), html);
}

TEST_F(ClipboardTest, Image) {
  scoped_refptr<nu::Image> image = new nu::Image(image_path_);

  std::vector<Data> objects;
  objects.emplace_back(image.get());
  clipboard_->SetData(std::move(objects));
  EXPECT_TRUE(clipboard_->IsDataAvailable(Data::Type::Image));

  Data data = clipboard_->GetData(Data::Type::Image);
  EXPECT_EQ(data.type(), Data::Type::Image);
  EXPECT_EQ(data.image()->GetSize(), image->GetSize());
}

TEST_F(ClipboardTest, FilePaths) {
  base::FilePath exe_path;
  PathService::Get(base::FILE_EXE, &exe_path);
  base::FilePath root_dir = exe_path.DirName().DirName().DirName();
  std::vector<base::FilePath> paths = {
    exe_path,
    image_path_,
    root_dir.Append(FILE_PATH_LITERAL("base")),
    root_dir.Append(FILE_PATH_LITERAL("nativeui")),
    root_dir.Append(FILE_PATH_LITERAL("v8binding")),
  };

  std::vector<Data> objects;
  objects.emplace_back(paths);
  clipboard_->SetData(std::move(objects));
  EXPECT_TRUE(clipboard_->IsDataAvailable(Data::Type::FilePaths));

  Data data = clipboard_->GetData(Data::Type::FilePaths);
  EXPECT_EQ(data.type(), Data::Type::FilePaths);
  EXPECT_EQ(data.file_paths(), paths);
}

TEST_F(ClipboardTest, Multiple) {
  std::vector<Data> objects;
  objects.emplace_back(Data::Type::HTML, "<div></div>");
  objects.emplace_back(Data::Type::Text, "text");
  objects.emplace_back(new nu::Image(image_path_));
  clipboard_->SetData(std::move(objects));
  EXPECT_TRUE(clipboard_->IsDataAvailable(Data::Type::HTML));
  EXPECT_TRUE(clipboard_->IsDataAvailable(Data::Type::Text));
  EXPECT_TRUE(clipboard_->IsDataAvailable(Data::Type::Image));
  EXPECT_FALSE(clipboard_->IsDataAvailable(Data::Type::FilePaths));
}

TEST_F(ClipboardTest, NotAvailable) {
  Data data = clipboard_->GetData(Data::Type::Text);
  EXPECT_EQ(data.type(), Data::Type::None);
  data = clipboard_->GetData(Data::Type::HTML);
  EXPECT_EQ(data.type(), Data::Type::None);
  data = clipboard_->GetData(Data::Type::Image);
  EXPECT_EQ(data.type(), Data::Type::None);
  data = clipboard_->GetData(Data::Type::FilePaths);
  EXPECT_EQ(data.type(), Data::Type::None);

  clipboard_->SetText("some text");
  data = clipboard_->GetData(Data::Type::HTML);
  EXPECT_EQ(data.type(), Data::Type::None);
  data = clipboard_->GetData(Data::Type::Image);
  EXPECT_EQ(data.type(), Data::Type::None);
  data = clipboard_->GetData(Data::Type::FilePaths);
  EXPECT_EQ(data.type(), Data::Type::None);
}
