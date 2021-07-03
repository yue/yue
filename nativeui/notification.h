// Copyright 2021 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_NOTIFICATION_H_
#define NATIVEUI_NOTIFICATION_H_

#include <string>
#include <vector>

#include "base/memory/ref_counted.h"
#include "nativeui/signal.h"
#include "nativeui/types.h"

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
#if defined(OS_MAC)
  void SetHasReplyButton(bool has);
  void SetResponsePlaceholder(const std::string& placeholder);
  void SetIdentifier(const std::string& identifier);
  std::string GetIdentifier() const;
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

  void PlatformInit();
  void PlatformDestroy();
  void PlatformSetImage(Image* image);

  scoped_refptr<Image> image_;

  NativeNotification notification_;
};

}  // namespace nu

#endif  // NATIVEUI_NOTIFICATION_H_
