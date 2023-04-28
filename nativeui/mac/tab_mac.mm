// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/tab.h"

#include "base/mac/scoped_nsobject.h"
#include "base/strings/sys_string_conversions.h"
#include "nativeui/gfx/font.h"
#include "nativeui/mac/nu_private.h"
#include "nativeui/mac/nu_view.h"

@interface NUTabDelegate : NSObject<NSTabViewDelegate> {
 @private
  nu::Tab* shell_;
}
- (id)initWithShell:(nu::Tab*)shell;
@end

@implementation NUTabDelegate

- (id)initWithShell:(nu::Tab*)shell {
  if ((self = [super init]))
    shell_ = shell;
  return self;
}

- (void)tabView:(NSTabView*)tabView
    didSelectTabViewItem:(NSTabViewItem*)tabViewItem {
  shell_->on_selected_page_change.Emit(shell_);
}

@end


@interface NUTab : NSTabView<NUViewMethods> {
 @private
  base::scoped_nsobject<NUTabDelegate> delegate_;
  nu::NUViewPrivate private_;
}
- (id)initWithShell:(nu::Tab*)shell;
@end

@implementation NUTab

- (id)initWithShell:(nu::Tab*)shell {
  if ((self = [super init])) {
    delegate_.reset([[NUTabDelegate alloc] initWithShell:shell]);
    [self setDelegate:delegate_];
  }
  return self;
}

- (nu::NUViewPrivate*)nuPrivate {
  return &private_;
}

- (void)setNUFont:(nu::Font*)font {
  [self setFont:font->GetNative()];
  [self setNeedsDisplay:YES];
}

- (void)setNUColor:(nu::Color)color {
}

- (void)setNUBackgroundColor:(nu::Color)color {
}

- (void)setNUEnabled:(BOOL)enabled {
}

- (BOOL)isNUEnabled {
  return YES;
}

@end

namespace nu {

NativeView Tab::PlatformCreate() {
  return [[NUTab alloc] initWithShell:this];
}

void Tab::PlatformAddPage(const std::string& title, View* view) {
  auto* tab = static_cast<NUTab*>(GetNative());
  base::scoped_nsobject<NSTabViewItem> item([[NSTabViewItem alloc] init]);
  [item setLabel:base::SysUTF8ToNSString(title)];
  [item setView:view->GetNative()];
  [tab addTabViewItem:item];
}

void Tab::PlatformRemovePage(int index, View* view) {
  auto* tab = static_cast<NUTab*>(GetNative());
  [tab removeTabViewItem:[tab tabViewItemAtIndex:index]];
}

void Tab::SelectPageAt(int index) {
  auto* tab = static_cast<NUTab*>(GetNative());
  if (index >= 0 && index < [tab numberOfTabViewItems])
    [tab selectTabViewItemAtIndex:index];
}

int Tab::GetSelectedPageIndex() const {
  auto* tab = static_cast<NUTab*>(GetNative());
  if ([tab selectedTabViewItem])
    return [tab indexOfTabViewItem:[tab selectedTabViewItem]];
  else
    return -1;
}

SizeF Tab::GetMinimumSize() const {
  auto* tab = static_cast<NUTab*>(GetNative());
  return SizeF([tab minimumSize]);
}

}  // namespace nu
