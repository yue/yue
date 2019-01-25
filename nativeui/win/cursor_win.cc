// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/cursor.h"

namespace nu {

Cursor::Cursor(Type type) {
  switch (type) {
    case Type::Default:
      cursor_ = ::LoadCursorW(NULL, IDC_ARROW);
      break;
    case Type::Hand:
      cursor_ = ::LoadCursorW(NULL, IDC_HAND);
      break;
    case Type::Crosshair:
      cursor_ = ::LoadCursorW(NULL, IDC_CROSS);
      break;
    case Type::Progress:
      cursor_ = ::LoadCursorW(NULL, IDC_APPSTARTING);
      break;
    case Type::Text:
      cursor_ = ::LoadCursorW(NULL, IDC_IBEAM);
      break;
    case Type::NotAllowed:
      cursor_ = ::LoadCursorW(NULL, IDC_NO);
      break;
    case Type::Help:
      cursor_ = ::LoadCursorW(NULL, IDC_HELP);
      break;
    case Type::Move:
      cursor_ = ::LoadCursorW(NULL, IDC_SIZEALL);
      break;
    case Type::ResizeEW:
      cursor_ = ::LoadCursorW(NULL, IDC_SIZEWE);
      break;
    case Type::ResizeNS:
      cursor_ = ::LoadCursorW(NULL, IDC_SIZENS);
      break;
    case Type::ResizeNESW:
      cursor_ = ::LoadCursorW(NULL, IDC_SIZENESW);
      break;
    case Type::ResizeNWSE:
      cursor_ = ::LoadCursorW(NULL, IDC_SIZENWSE);
      break;
  }
}

Cursor::~Cursor() {
  // No need to free shared cursors.
}

}  // namespace nu
