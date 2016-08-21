// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/subwin_view.h"

#include "nativeui/win/subwin_holder.h"

namespace nu {

SubwinView::SubwinView(base::StringPiece16 class_name,
                       DWORD window_style, DWORD window_ex_style)
    : WindowImpl(class_name, SubwinHolder::GetInstance()->hwnd(),
                 window_style, window_ex_style) {
}

SubwinView::~SubwinView() {
}

void SubwinView::OnSize(UINT param, const gfx::Size& size) {
}

}  // namespace nu
