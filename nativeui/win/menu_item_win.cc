// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/menu_item_win.h"

#include <richedit.h>

#include "base/strings/utf_string_conversions.h"
#include "nativeui/menu.h"
#include "nativeui/menu_bar.h"
#include "nativeui/menu_item.h"
#include "nativeui/state.h"
#include "nativeui/win/drag_drop/clipboard_util.h"
#include "nativeui/win/subwin_view.h"
#include "nativeui/win/view_win.h"
#include "nativeui/win/window_win.h"

namespace nu {

namespace {

void OnRoleClick(MenuItem* self) {
  // Get the window.
  MenuBase* menu = self->FindTopLevelMenu();
  if (!menu || menu->GetClassName() != MenuBar::kClassName)
    return;
  Window* window = static_cast<MenuBar*>(menu)->GetWindow();
  if (!window)
    return;
  // Window roles.
  switch (self->GetRole()) {
    case MenuItem::Role::Minimize:
      window->Minimize();
      return;
    case MenuItem::Role::Maximize:
      window->Maximize();
      return;
    case MenuItem::Role::CloseWindow:
      window->Close();
      return;
    default:
      break;
  }
  // Get the focused widget.
  ViewImpl* view = window->GetNative()->focus_manager()->focused_view();
  if (!view)
    return;
  // Edit roles.
  if (view->type() != ControlType::Subwin)
    return;
  HWND hwnd = static_cast<SubwinView*>(view)->hwnd();
  switch (self->GetRole()) {
    case MenuItem::Role::Copy:
      ::SendMessage(hwnd, WM_COPY, 0, 0L);
      break;
    case MenuItem::Role::Cut:
      ::SendMessage(hwnd, WM_CUT, 0, 0L);
      break;
    case MenuItem::Role::Paste:
      ::SendMessage(hwnd, WM_PASTE, 0, 0L);
      break;
    case MenuItem::Role::SelectAll:
      ::SendMessage(hwnd, EM_SETSEL, 0, -1);
      break;
    case MenuItem::Role::Undo:
      ::SendMessage(hwnd, EM_UNDO, 0, 0L);
      break;
    case MenuItem::Role::Redo:
      ::SendMessage(hwnd, EM_REDO, 0, 0L);
      break;
    default:
      break;
  }
}

}  // namespace

MenuItemData::MenuItemData() {}
MenuItemData::~MenuItemData() {}

void MenuItem::Click() {
  EmitClick();
}

void MenuItem::SetLabel(const std::string& label) {
  menu_item_->label = label;
  menu_item_->actual_label = base::UTF8ToWide(label);
  if (!menu_item_->accelerator.empty()) {
    menu_item_->actual_label += L'\t';
    menu_item_->actual_label += menu_item_->accelerator;
  }
  if (menu_) {
    MENUITEMINFO mii = {0};
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_STRING;
    mii.dwTypeData = const_cast<wchar_t*>(menu_item_->actual_label.c_str());
    mii.cch = static_cast<UINT>(menu_item_->actual_label.size());
    SetMenuItemInfo(menu_->GetNative(), menu_item_->id, FALSE, &mii);
  }
}

std::string MenuItem::GetLabel() const {
  return menu_item_->label;
}

void MenuItem::SetChecked(bool checked) {
  menu_item_->checked = checked;
  if (menu_) {
    if (checked && type_ == Type::Radio)
      FlipRadioMenuItems(menu_, this);
    UINT flags = MF_BYCOMMAND;
    flags |= checked ? MF_CHECKED : MF_UNCHECKED;
    CheckMenuItem(menu_->GetNative(), menu_item_->id, flags);
  }
}

bool MenuItem::IsChecked() const {
  return menu_item_->checked;
}

void MenuItem::SetEnabled(bool enabled) {
  menu_item_->enabled = enabled;
  if (menu_) {
    UINT flags = MF_BYCOMMAND;
    flags |= enabled ? MF_ENABLED : MF_DISABLED;
    EnableMenuItem(menu_->GetNative(), menu_item_->id, flags);
  }
}

bool MenuItem::IsEnabled() const {
  return menu_item_->enabled;
}

void MenuItem::SetVisible(bool visible) {
  menu_item_->visible = visible;
}

bool MenuItem::IsVisible() const {
  return menu_item_->visible;
}

void MenuItem::PlatformInit() {
  menu_item_ = new MenuItemData;
  menu_item_->id = State::GetCurrent()->GetNextCommandID();
  if (role_ < Role::ItemCount)
    on_click.Connect(&OnRoleClick);
}

void MenuItem::PlatformDestroy() {
  delete menu_item_;
}

void MenuItem::PlatformSetSubmenu(Menu* submenu) {
  if (menu_) {
    MENUITEMINFO mii = {0};
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_SUBMENU;
    mii.hSubMenu = submenu->GetNative();
    SetMenuItemInfo(menu_->GetNative(), menu_item_->id, FALSE, &mii);
  }
}

void MenuItem::PlatformSetImage(Image* image) {
  bitmap_image_.reset(GetBitmapFromImage(image));
  if (menu_) {
    MENUITEMINFO mii = {0};
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_BITMAP;
    mii.hbmpItem = bitmap_image_.get();
    SetMenuItemInfo(menu_->GetNative(), menu_item_->id, FALSE, &mii);
  }
}

}  // namespace nu
