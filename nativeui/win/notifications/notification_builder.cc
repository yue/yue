// Copyright 2021 Cheng Zhao. All rights reserved.
// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#include "nativeui/win/notifications/notification_builder.h"

#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "nativeui/win/notification_win.h"

namespace nu {

namespace {

const wchar_t kEmptyTemplate[] =
    L"<toast>\n"
    L"  <visual>\n"
    L"    <binding template=\"ToastGeneric\">\n"
    L"      <text>{title}</text>\n"
    L"      <text>{body}</text>\n"
    L"    </binding>\n"
    L"  </visual>\n"
    L"  <actions></actions>\n"
    L"</toast>";

mswr::ComPtr<winxml::Dom::IXmlDocument> XMLStringToDocument(
    base::WStringPiece xml_template) {
  ScopedHString ref_class_name =
      ScopedHString::Create(RuntimeClass_Windows_Data_Xml_Dom_XmlDocument);
  mswr::ComPtr<IInspectable> inspectable;
  HRESULT hr =
      base::win::RoActivateInstance(ref_class_name.get(), &inspectable);
  if (FAILED(hr)) {
    DLOG(ERROR) << "Unable to activate the XML Document " << std::hex << hr;
    return nullptr;
  }

  mswr::ComPtr<winxml::Dom::IXmlDocumentIO> document_io;
  hr = inspectable.As(&document_io);
  if (FAILED(hr)) {
    DLOG(ERROR) << "Failed to get XmlDocument as IXmlDocumentIO " << std::hex
                << hr;
    return nullptr;
  }

  ScopedHString ref_template = ScopedHString::Create(xml_template);
  hr = document_io->LoadXml(ref_template.get());
  if (FAILED(hr)) {
    DLOG(ERROR) << "Unable to load the template's XML into the document "
                << std::hex << hr;
    return nullptr;
  }

  mswr::ComPtr<winxml::Dom::IXmlDocument> document;
  hr = document_io.As<winxml::Dom::IXmlDocument>(&document);
  if (FAILED(hr)) {
    DLOG(ERROR) << "Unable to get as XMLDocument " << std::hex << hr;
    return nullptr;
  }

  return document;
}

mswr::ComPtr<winxml::Dom::IXmlNodeList> GetElementsByTagName(
    winxml::Dom::IXmlDocument* document,
    base::WStringPiece tag_str,
    UINT32 expected_length) {
  ScopedHString tag = ScopedHString::Create(tag_str);
  mswr::ComPtr<winxml::Dom::IXmlNodeList> elements;
  HRESULT hr = document->GetElementsByTagName(tag.get(), &elements);
  if (FAILED(hr)) {
    DLOG(ERROR) << "Failed to get <" << tag_str << "> elements from document";
    return nullptr;
  }

  UINT32 length;
  hr = elements->get_Length(&length);
  if (FAILED(hr) || length < expected_length) {
    DLOG(ERROR) << "Not enough <" << tag_str << "> elements in document.";
    return nullptr;
  }

  return elements;
}

mswr::ComPtr<winxml::Dom::IXmlNode> GetNthElementByTagName(
    winxml::Dom::IXmlDocument* document,
    base::WStringPiece tag_str,
    UINT32 index) {
  mswr::ComPtr<winxml::Dom::IXmlNodeList> node_list =
      GetElementsByTagName(document, tag_str, index + 1);
  if (!node_list)
    return nullptr;
  mswr::ComPtr<winxml::Dom::IXmlNode> node;
  if (FAILED(node_list->Item(index, &node)))
    return nullptr;
  return node;
}

HRESULT SetAttribute(winxml::Dom::IXmlDocument* document,
                     winxml::Dom::IXmlNode* node,
                     base::WStringPiece name,
                     base::WStringPiece value) {
  mswr::ComPtr<winxml::Dom::IXmlAttribute> attribute;
  HRESULT hr = document->CreateAttribute(ScopedHString::Create(name).get(),
                                         &attribute);
  if (FAILED(hr))
    return hr;
  mswr::ComPtr<winxml::Dom::IXmlNode> attr_node;
  hr = attribute.As(&attr_node);
  if (FAILED(hr))
    return hr;
  mswr::ComPtr<winxml::Dom::IXmlText> xml_text;
  hr = document->CreateTextNode(ScopedHString::Create(value).get(), &xml_text);
  if (FAILED(hr))
    return hr;
  mswr::ComPtr<winxml::Dom::IXmlNode> value_node;
  hr = xml_text.As(&value_node);
  if (FAILED(hr))
    return hr;
  mswr::ComPtr<winxml::Dom::IXmlNode> append_node;
  hr = attr_node->AppendChild(value_node.Get(), &append_node);
  if (FAILED(hr))
    return hr;
  mswr::ComPtr<winxml::Dom::IXmlNamedNodeMap> attributes;
  hr = node->get_Attributes(&attributes);
  if (FAILED(hr))
    return hr;
  return attributes->SetNamedItem(attr_node.Get(), &append_node);
}

HRESULT AppendNode(winxml::Dom::IXmlDocument* document,
                   winxml::Dom::IXmlNode* parent,
                   base::WStringPiece tag,
                   mswr::ComPtr<winxml::Dom::IXmlNode>* node) {
  mswr::ComPtr<winxml::Dom::IXmlElement> element;
  HRESULT hr = document->CreateElement(ScopedHString::Create(tag).get(),
                                       &element);
  if (FAILED(hr))
    return hr;
  hr = element.As(&(*node));
  if (FAILED(hr))
    return hr;
  mswr::ComPtr<winxml::Dom::IXmlNode> append_node;
  return parent->AppendChild(node->Get(), &append_node);
}

HRESULT AppendActionNode(winxml::Dom::IXmlDocument* document,
                         winxml::Dom::IXmlNode* actions,
                         const Notification::Action& action) {
  mswr::ComPtr<winxml::Dom::IXmlNode> action_node;
  HRESULT hr = AppendNode(document, actions, L"action", &action_node);
  if (FAILED(hr))
    return hr;
  SetAttribute(document, action_node.Get(), L"content",
               base::UTF8ToWide(action.title));
  return SetAttribute(document, action_node.Get(), L"arguments",
                      kNotificationTypeAction + base::UTF8ToWide(action.info));
}

mswr::ComPtr<IToastNotification> CreateToastNotification(
    winxml::Dom::IXmlDocument* document) {
  mswr::ComPtr<winui::Notifications::IToastNotificationFactory> factory;
  HRESULT hr = CreateActivationFactory(
      RuntimeClass_Windows_UI_Notifications_ToastNotification,
      IID_PPV_ARGS(&factory));
  if (FAILED(hr)) {
    DLOG(ERROR) << "Unable to create IToastNotificationFactory "
                << std::hex << hr;
    return nullptr;
  }
  mswr::ComPtr<IToastNotification> toast_notification;
  hr = factory->CreateToastNotification(document, &toast_notification);
  if (FAILED(hr)) {
    DLOG(ERROR) << "Unable to create IToastNotification " << std::hex << hr;
    return nullptr;
  }
  return toast_notification;
}

mswr::ComPtr<winxml::Dom::IXmlDocument> BuildNotificationXMLDocument(
    NotificationImpl* notification) {
  mswr::ComPtr<winxml::Dom::IXmlDocument> document =
      XMLStringToDocument(kEmptyTemplate);
  if (!document)
    return nullptr;

  mswr::ComPtr<winxml::Dom::IXmlNode> toast =
      GetNthElementByTagName(document.Get(), L"toast", 0);
  if (!toast)
    return nullptr;
  SetAttribute(document.Get(), toast.Get(), L"launch",
               kNotificationTypeClick + notification->info);

  mswr::ComPtr<winxml::Dom::IXmlNode> actions =
      GetNthElementByTagName(document.Get(), L"actions", 0);
  if (!actions)
    return nullptr;

  if (notification->has_reply_button) {
    mswr::ComPtr<winxml::Dom::IXmlNode> input;
    if (FAILED(AppendNode(document.Get(), actions.Get(), L"input", &input)))
      return nullptr;
    SetAttribute(document.Get(), input.Get(), L"type", L"text");
    SetAttribute(document.Get(), input.Get(), L"id", kReplyInputId);
    if (!notification->reply_placeholder.empty()) {
      SetAttribute(document.Get(), input.Get(), L"placeHolderContent",
                   notification->reply_placeholder.c_str());
    }
    mswr::ComPtr<winxml::Dom::IXmlNode> button;
    if (FAILED(AppendNode(document.Get(), actions.Get(), L"action", &button)))
      return nullptr;
    SetAttribute(document.Get(), button.Get(), L"content", L"Reply");
    SetAttribute(document.Get(), button.Get(), L"hint-inputId", kReplyInputId);
    SetAttribute(document.Get(), button.Get(), L"arguments",
                 kNotificationTypeReply + notification->info);
  }

  for (const Notification::Action& action : notification->actions) {
    if (FAILED(AppendActionNode(document.Get(), actions.Get(), action)))
      return nullptr;
  }

  if (notification->silent) {
    mswr::ComPtr<winxml::Dom::IXmlNode> node;
    if (FAILED(AppendNode(document.Get(), toast.Get(), L"audio", &node)))
      return nullptr;
    SetAttribute(document.Get(), node.Get(), L"silent", L"true");
  }

  if (notification->image_path) {
    mswr::ComPtr<winxml::Dom::IXmlNode> binding =
        GetNthElementByTagName(document.Get(), L"binding", 0);
    if (!binding)
      return nullptr;
    mswr::ComPtr<winxml::Dom::IXmlNode> node;
    if (FAILED(AppendNode(document.Get(), binding.Get(), L"image", &node)))
      return nullptr;
    SetAttribute(document.Get(), node.Get(), L"src",
                 notification->image_path->c_str());
    if (notification->image_placement) {
      SetAttribute(document.Get(), node.Get(), L"placement",
                   notification->image_placement->c_str());
    }
  }

  return document;
}

}  // namespace

const wchar_t kNotificationTypeClick[] = L"type=click&info=";
const wchar_t kNotificationTypeAction[] = L"type=action&info=";
const wchar_t kNotificationTypeReply[] = L"type=reply&info=";
const wchar_t kReplyInputId[] = L"reply";

std::wstring GetNotificationXMLRepresentation(NotificationImpl* notification) {
  mswr::ComPtr<winxml::Dom::IXmlDocument> document =
      BuildNotificationXMLDocument(notification);
  if (!document)
    return std::wstring();
  HSTRING xml;
  mswr::ComPtr<winxml::Dom::IXmlNodeSerializer> serializer;
  if (FAILED(document.As<winxml::Dom::IXmlNodeSerializer>(&serializer)))
    return std::wstring();
  if (FAILED(serializer->GetXml(&xml)))
    return std::wstring();
  ScopedHString hstr(xml);
  base::WStringPiece str = hstr.Get();
  return std::wstring(str.data(), str.length());
}

mswr::ComPtr<IToastNotification> BuildNotification(
    NotificationImpl* notification) {
  mswr::ComPtr<winxml::Dom::IXmlDocument> document =
      notification->xml ? XMLStringToDocument(*notification->xml)
                        : BuildNotificationXMLDocument(notification);
  if (!document)
    return nullptr;
  return CreateToastNotification(document.Get());
}

mswr::ComPtr<INotificationData> CreateNotificationData() {
  mswr::ComPtr<winui::Notifications::INotificationDataFactory> factory;
  HRESULT hr = CreateActivationFactory(
      RuntimeClass_Windows_UI_Notifications_NotificationData,
      IID_PPV_ARGS(&factory));
  if (FAILED(hr)) {
    DLOG(ERROR) << "Unable to create INotificationDataFactory "
                << std::hex << hr;
    return nullptr;
  }
  mswr::ComPtr<INotificationData> notification_data;
  hr = factory->CreateNotificationDataWithValues(nullptr, &notification_data);
  if (FAILED(hr)) {
    DLOG(ERROR) << "Unable to create INotificationData " << std::hex << hr;
    return nullptr;
  }
  return notification_data;
}

HRESULT NotificationDataInsert(INotificationData* data,
                               base::WStringPiece key,
                               base::WStringPiece value) {
  mswr::ComPtr<winfoundtn::Collections::IMap<HSTRING, HSTRING>> values;
  HRESULT hr = data->get_Values(&values);
  if (FAILED(hr))
    return hr;
  ScopedHString hkey = ScopedHString::Create(key);
  ScopedHString hvalue = ScopedHString::Create(value);
  boolean replaced;
  return values->Insert(hkey.get(), hvalue.get(), &replaced);
}

}  // namespace nu
