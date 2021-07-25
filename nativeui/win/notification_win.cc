// Copyright 2021 Cheng Zhao. All rights reserved.
// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#include "nativeui/win/notification_win.h"

#include <utility>

#include "base/logging.h"
#include "base/no_destructor.h"
#include "base/strings/utf_string_conversions.h"
#include "nativeui/app.h"
#include "nativeui/notification_center.h"
#include "nativeui/state.h"
#include "nativeui/win/notifications/notification_builder.h"

namespace nu {

using ToastDismissedHandler = ABI::Windows::Foundation::ITypedEventHandler<
    winui::Notifications::ToastNotification*,
    winui::Notifications::ToastDismissedEventArgs*>;

namespace {

winui::Notifications::IToastNotifier* GetNotifier() {
  if (!State::GetCurrent()->InitializeWinRT())
    return nullptr;

  static base::NoDestructor<mswr::ComPtr<winui::Notifications::IToastNotifier>>
      notifier;
  if (!(*notifier)) {
    mswr::ComPtr<winui::Notifications::IToastNotificationManagerStatics>
        toast_manager;
    HRESULT hr = CreateActivationFactory(
        RuntimeClass_Windows_UI_Notifications_ToastNotificationManager,
        IID_PPV_ARGS(&toast_manager));
    if (FAILED(hr)) {
      LOG(ERROR) << "Unable to create the ToastNotificationManager "
                 << std::hex << hr;
      return nullptr;
    }

    App* app = App::GetCurrent();
    if (app->IsRunningAsUWP()) {
      hr = toast_manager->CreateToastNotifier(&(*notifier));
    } else {
      std::wstring app_user_model_id = app->GetAppUserModelID();
      if (app_user_model_id.empty()) {
        LOG(ERROR) << "AppUserModelID is not set";
        return false;
      }
      hr = toast_manager->CreateToastNotifierWithId(
          ScopedHString::Create(app_user_model_id.c_str()).get(),
          &(*notifier));
    }
    if (FAILED(hr)) {
      LOG(ERROR) << "Unable to create the ToastNotifier " << std::hex << hr;
      return nullptr;
    }
  }
  return (*notifier).Get();
}

}  // namespace

class ToastEventHandler {
 public:
  explicit ToastEventHandler(Notification* notification)
      : notification_(notification) {}

  HRESULT OnDismissed(winui::Notifications::IToastNotification*,
                      winui::Notifications::IToastDismissedEventArgs*) {
    auto* center = NotificationCenter::GetCurrent();
    center->on_notification_close.Emit(notification_->GetInfo());
    return S_OK;
  }

 private:
  Notification* notification_;

  DISALLOW_COPY_AND_ASSIGN(ToastEventHandler);
};

void Notification::Show() {
  auto* notifier = GetNotifier();
  if (!notifier) {
    LOG(ERROR) << "Unable to initialize toast notifier";
    return;
  }

  notification_->toast = BuildNotification(notification_);

  notification_->event_handler.reset(new ToastEventHandler(this));
  auto handler = mswr::Callback<ToastDismissedHandler>(
      notification_->event_handler.get(), &ToastEventHandler::OnDismissed);
  EventRegistrationToken token;
  notification_->toast->add_Dismissed(handler.Get(), &token);

  HRESULT hr = notifier->Show(notification_->toast.Get());
  if (FAILED(hr)) {
    LOG(ERROR) << "Unable to display the notification " << std::hex << hr;
    return;
  }

  auto* center = NotificationCenter::GetCurrent();
  center->on_notification_show.Emit(GetInfo());
}

void Notification::Close() {
  auto* notifier = GetNotifier();
  if (notifier && notification_->toast)
    notifier->Hide(notification_->toast.Get());
}

void Notification::SetTitle(const std::string& title) {
  notification_->title = base::UTF8ToWide(title);
}

void Notification::SetBody(const std::string& body) {
  notification_->body = base::UTF8ToWide(body);
}

void Notification::SetInfo(const std::string& info) {
  notification_->info = base::UTF8ToWide(info);
}

std::string Notification::GetInfo() const {
  return base::WideToUTF8(notification_->info);
}

void Notification::SetSilent(bool silent) {
  notification_->silent = silent;
}

void Notification::SetImagePath(const base::FilePath& path) {
  notification_->image.emplace(path.value());
}

void Notification::SetActions(const std::vector<Action>& actions) {
  notification_->actions = actions;
}

void Notification::SetHasReplyButton(bool has) {
  notification_->has_reply_button = has;
}

void Notification::SetResponsePlaceholder(const std::string& placeholder) {
  notification_->reply_placeholder = base::UTF8ToWide(placeholder);
}

void Notification::SetImagePlacement(base::Optional<std::wstring> placement) {
  notification_->image_placement = std::move(placement);
}

void Notification::SetXML(base::Optional<std::wstring> xml) {
  notification_->xml = std::move(xml);
}

std::wstring Notification::GetXML() const {
  return GetNotificationXMLRepresentation(notification_);
}

void Notification::PlatformInit() {
  notification_ = new NotificationImpl();
}

void Notification::PlatformDestroy() {
  delete notification_;
}

}  // namespace nu
