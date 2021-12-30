// Copyright 2021 Cheng Zhao. All rights reserved.
// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#include "nativeui/win/notification_win.h"

#include <functional>
#include <utility>

#include "base/logging.h"
#include "base/no_destructor.h"
#include "base/strings/string_number_conversions_win.h"
#include "base/strings/utf_string_conversions.h"
#include "nativeui/app.h"
#include "nativeui/message_loop.h"
#include "nativeui/notification_center.h"
#include "nativeui/state.h"
#include "nativeui/win/notifications/notification_builder.h"

namespace nu {

namespace {

using ToastActivatedHandler = ABI::Windows::Foundation::ITypedEventHandler<
    winui::Notifications::ToastNotification*,
    IInspectable*>;
using ToastDismissedHandler = ABI::Windows::Foundation::ITypedEventHandler<
    winui::Notifications::ToastNotification*,
    winui::Notifications::ToastDismissedEventArgs*>;
using ToastFailedHandler = ABI::Windows::Foundation::ITypedEventHandler<
    winui::Notifications::ToastNotification*,
    winui::Notifications::ToastFailedEventArgs*>;

const wchar_t kNotificationGroup[] = L"YueNotification";

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

bool UpdateToastNotification(winui::Notifications::IToastNotifier* notifier,
                             NotificationImpl* notification) {
  if (notification->first_time) {
    notification->first_time = false;
    return false;
  }

  mswr::ComPtr<winui::Notifications::IToastNotifier2> notifier2;
  HRESULT hr = notifier->QueryInterface(IID_PPV_ARGS(&notifier2));
  if (FAILED(hr)) {
    DLOG(ERROR) << "Failed to get IToastNotifier2 " << std::hex << hr;
    return false;
  }

  winui::Notifications::NotificationUpdateResult result =
      winui::Notifications::NotificationUpdateResult_Succeeded;
  hr = notifier2->UpdateWithTagAndGroup(
      notification->data.Get(),
      ScopedHString::Create(notification->id).get(),
      ScopedHString::Create(kNotificationGroup).get(),
      &result);
  if (FAILED(hr)) {
    LOG(ERROR) << "Failed to update notification " << result << " "
               << std::hex << hr;
    return false;
  }

  return result == winui::Notifications::NotificationUpdateResult_Succeeded;
}

}  // namespace

class ToastEventHandler {
 public:
  explicit ToastEventHandler(Notification* notification)
      : notification_(notification) {
    IToastNotification* toast = notification_->GetNative()->toast.Get();
    toast->add_Activated(mswr::Callback<ToastActivatedHandler>(
                             this, &ToastEventHandler::OnActivated).Get(),
                         &activate_token_);
    toast->add_Dismissed(mswr::Callback<ToastDismissedHandler>(
                             this, &ToastEventHandler::OnDismissed).Get(),
                         &dismiss_token_);
    toast->add_Failed(mswr::Callback<ToastFailedHandler>(
                          this, &ToastEventHandler::OnFailed).Get(),
                      &fail_token_);
    // There is no event emitted if the notification is removed by system, so
    // we have to delete the handler after some timeout, otherwise it may be
    // leaked forever.
    timer_ = MessageLoop::SetTimeout(
        10 * 60 * 1000, std::bind(&ToastEventHandler::OnTimeout, this));
  }

  ~ToastEventHandler() {
    IToastNotification* toast = notification_->GetNative()->toast.Get();
    toast->remove_Activated(activate_token_);
    toast->remove_Dismissed(dismiss_token_);
    toast->remove_Failed(fail_token_);
    if (timer_ != 0)
      MessageLoop::ClearTimeout(timer_);
  }

  ToastEventHandler& operator=(const ToastEventHandler&) = delete;
  ToastEventHandler(const ToastEventHandler&) = delete;

  HRESULT OnActivated(winui::Notifications::IToastNotification*,
                      IInspectable*) {
    DeleteThis();
    return S_OK;
  }

  HRESULT OnDismissed(winui::Notifications::IToastNotification*,
                      winui::Notifications::IToastDismissedEventArgs*) {
    auto* center = NotificationCenter::GetCurrent();
    center->on_notification_close.Emit(notification_->GetInfo());
    DeleteThis();
    return S_OK;
  }

  HRESULT OnFailed(winui::Notifications::IToastNotification*,
                   winui::Notifications::IToastFailedEventArgs*) {
    DeleteThis();
    return S_OK;
  }

  void OnTimeout() {
    timer_ = 0;
    DeleteThis();
  }

 private:
  void DeleteThis() {
    delete this;
  }

  scoped_refptr<Notification> notification_;

  EventRegistrationToken activate_token_;
  EventRegistrationToken dismiss_token_;
  EventRegistrationToken fail_token_;

  MessageLoop::TimerId timer_;
};

void Notification::Show() {
  if (!notification_)
    return;

  auto* notifier = GetNotifier();
  if (!notifier) {
    LOG(ERROR) << "Unable to initialize toast notifier";
    return;
  }

  // First try to update the notification with existing tag.
  if (!UpdateToastNotification(notifier, notification_)) {
    // Then create a new toast notification.
    notification_->toast = BuildNotification(notification_);
    if (!notification_->toast) {
      LOG(ERROR) << "Unable to create toast notification";
      return;
    }

    mswr::ComPtr<winui::Notifications::IToastNotification2> toast2;
    HRESULT hr = notification_->toast->QueryInterface(IID_PPV_ARGS(&toast2));
    if (FAILED(hr)) {
      DLOG(ERROR) << "Failed to get IToastNotification2 " << std::hex << hr;
      return;
    }
    toast2->put_Group(ScopedHString::Create(kNotificationGroup).get());
    toast2->put_Tag(ScopedHString::Create(notification_->id).get());

    mswr::ComPtr<winui::Notifications::IToastNotification4> toast4;
    hr = notification_->toast->QueryInterface(IID_PPV_ARGS(&toast4));
    if (FAILED(hr)) {
      DLOG(ERROR) << "Failed to get IToastNotification4 " << std::hex << hr;
      return;
    }
    toast4->put_Data(notification_->data.Get());

    // Listen to dismiss events.
    new ToastEventHandler(this);

    hr = notifier->Show(notification_->toast.Get());
    if (FAILED(hr)) {
      LOG(ERROR) << "Unable to display the notification " << std::hex << hr;
      return;
    }
  }

  auto* center = NotificationCenter::GetCurrent();
  center->on_notification_show.Emit(GetInfo());
}

void Notification::Close() {
  if (!notification_)
    return;
  auto* notifier = GetNotifier();
  if (notifier && notification_->toast)
    notifier->Hide(notification_->toast.Get());
}

void Notification::SetTitle(const std::string& title) {
  if (!notification_)
    return;
  NotificationDataInsert(notification_->data.Get(), L"title",
                         base::UTF8ToWide(title));
}

void Notification::SetBody(const std::string& body) {
  if (!notification_)
    return;
  NotificationDataInsert(notification_->data.Get(), L"body",
                         base::UTF8ToWide(body));
}

void Notification::SetInfo(const std::string& info) {
  if (!notification_)
    return;
  notification_->info = base::UTF8ToWide(info);
}

std::string Notification::GetInfo() const {
  if (!notification_)
    return std::string();
  return base::WideToUTF8(notification_->info);
}

void Notification::SetSilent(bool silent) {
  if (!notification_)
    return;
  notification_->silent = silent;
}

void Notification::SetImagePath(const base::FilePath& path) {
  if (!notification_)
    return;
  notification_->image_path.emplace(path.value());
}

void Notification::SetActions(const std::vector<Action>& actions) {
  if (!notification_)
    return;
  notification_->actions = actions;
}

void Notification::SetHasReplyButton(bool has) {
  if (!notification_)
    return;
  notification_->has_reply_button = has;
}

void Notification::SetResponsePlaceholder(const std::string& placeholder) {
  if (!notification_)
    return;
  notification_->reply_placeholder = base::UTF8ToWide(placeholder);
}

void Notification::SetIdentifier(const std::string& identifier) {
  notification_->id = base::UTF8ToWide(identifier);
  notification_->first_time = false;
}

std::string Notification::GetIdentifier() const {
  return base::WideToUTF8(notification_->id);
}

void Notification::SetImagePlacement(absl::optional<std::wstring> placement) {
  if (!notification_)
    return;
  notification_->image_placement = std::move(placement);
}

void Notification::SetXML(absl::optional<std::wstring> xml) {
  if (!notification_)
    return;
  notification_->xml = std::move(xml);
}

std::wstring Notification::GetXML() const {
  if (!notification_)
    return std::wstring();
  return GetNotificationXMLRepresentation(notification_);
}

void Notification::PlatformInit() {
  if (!State::GetCurrent()->InitializeWinRT())
    return;
  auto data = CreateNotificationData();
  if (!data)
    return;
  static int next_id = 0;
  notification_ = new NotificationImpl();
  notification_->id = base::NumberToWString(++next_id);
  notification_->data = std::move(data);
}

void Notification::PlatformDestroy() {
  if (notification_)
    delete notification_;
}

}  // namespace nu
