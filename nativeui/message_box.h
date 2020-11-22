// Copyright 2020 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_MESSAGE_BOX_H_
#define NATIVEUI_MESSAGE_BOX_H_

#include <string>

#include "base/memory/ref_counted.h"
#include "base/optional.h"
#include "nativeui/signal.h"
#include "nativeui/types.h"

namespace nu {

class Image;
class View;
class Window;

class NATIVEUI_EXPORT MessageBox : public base::RefCounted<MessageBox> {
 public:
  enum class Type {
    None,
    Information,
    Warning,
    Error,
  };

  MessageBox();

  int Run();
  int RunForWindow(Window* window);
#if defined(OS_LINUX) || defined(OS_WIN)
  void Show();
#endif
  void ShowForWindow(Window* window);
  void Close();

  void SetType(Type type);
  void AddButton(const std::string& title, int response);
  void SetDefaultResponse(int response);
  void SetCancelResponse(int response);
  void SetText(const std::string& text);
  void SetInformativeText(const std::string& text);
#if defined(OS_LINUX) || defined(OS_MAC)
  void SetAccessoryView(scoped_refptr<View> view);
  View* GetAccessoryView() const { return accessory_view_.get(); }
#endif
  void SetImage(scoped_refptr<Image> image);
  Image* GetImage() const { return image_.get(); }

  NativeMessageBox GetNative() const { return box_; }

  // Events.
  Signal<void(MessageBox*, int response)> on_response;

  // Private: Called by native implementations to notify response.
  void OnClose(base::Optional<int> response = base::Optional<int>());

 private:
  friend class base::RefCounted<MessageBox>;

  ~MessageBox();

  int PlatformRun();
  int PlatformRunForWindow(Window* window);
  void PlatformShow();
  void PlatformShowForWindow(Window* window);
  void PlatformClose();
  void PlatformSetDefaultResponse();
  void PlatformSetCancelResponse();
  void PlatformSetImage(Image* image);

  base::Optional<int> default_response_;
  int cancel_response_ = -1;
#if defined(OS_LINUX) || defined(OS_MAC)
  scoped_refptr<View> accessory_view_;
#endif
  scoped_refptr<Image> image_;

  bool is_showing_ = false;
  NativeMessageBox box_;
};

}  // namespace nu

#endif  // NATIVEUI_MESSAGE_BOX_H_
