// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/app.h"

#include "nativeui/gfx/font.h"
#include "nativeui/menu_bar.h"
#include "nativeui/state.h"

namespace nu {

// static
App* App::GetCurrent() {
  return State::GetCurrent()->GetApp();
}

App::App() : weak_factory_(this) {
}

App::~App() = default;

Color App::GetColor(ThemeColor name) {
  return Color::Get(name);
}

Font* App::GetDefaultFont() {
  return Font::Default();
}

Clipboard* App::GetClipboard(Clipboard::Type type) {
  return Clipboard::FromType(type);
}

}  // namespace nu
