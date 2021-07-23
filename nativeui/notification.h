// Copyright 2021 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_NOTIFICATION_H_
#define NATIVEUI_NOTIFICATION_H_

#include <memory>
#include <string>
#include <vector>

#include "base/memory/ref_counted.h"
#include "nativeui/nativeui_export.h"
#include "nativeui/types.h"

#if defined(OS_LINUX) || defined(OS_WIN)
#include "base/files/scoped_temp_dir.h"
#include "base/optional.h"
#endif

namespace nu {

class Image;

class NATIVEUI_EXPORT Notification : public base::RefCounted<Notification> {
 public:
  Notification();

  void Show();
  void Close();
  void SetTitle(const std::string& title);
  void SetBody(const std::string& body);
  void SetInfo(const std::string& info);
  std::string GetInfo() const;
  void SetSilent(bool silent);
  void SetImage(scoped_refptr<Image> image);
#if defined(OS_MAC) || defined(OS_WIN)
  void SetHasReplyButton(bool has);
  void SetResponsePlaceholder(const std::string& placeholder);
#endif
#if defined(OS_MAC)
  void SetIdentifier(const std::string& identifier);
  std::string GetIdentifier() const;
#elif defined(OS_WIN)
  void SetImagePlacement(base::Optional<std::wstring> placement);
  void SetXML(base::Optional<std::wstring> xml);
  std::wstring GetXML() const;
#endif

  struct Action {
    std::string title;
    std::string info;
  };
  void SetActions(const std::vector<Action>& actions);

  // Return the native Notification object.
  NativeNotification GetNative() const { return notification_; }

 private:
  friend class base::RefCounted<Notification>;

  ~Notification();

#if defined(OS_LINUX) || defined(OS_WIN)
  bool WriteImageToTempDir(Image* image, base::FilePath* out);
#endif

  void PlatformInit();
  void PlatformDestroy();
  void PlatformSetImage(Image* image);

  scoped_refptr<Image> image_;
#if defined(OS_LINUX) || defined(OS_WIN)
  base::ScopedTempDir image_dir_;
#endif

  NativeNotification notification_ = nullptr;
};

}  // namespace nu

#endif  // NATIVEUI_NOTIFICATION_H_
