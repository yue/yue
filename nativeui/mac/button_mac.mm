// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/button.h"

#include "base/strings/sys_string_conversions.h"
#include "nativeui/gfx/font.h"
#include "nativeui/gfx/image.h"
#include "nativeui/gfx/geometry/insets_f.h"
#include "nativeui/mac/nu_private.h"
#include "nativeui/mac/nu_view.h"

namespace {

nu::InsetsF GetButtonInsets(NSButton* button) {
  if ([button bezelStyle] == NSSmallSquareBezelStyle) {
    return nu::InsetsF(1, 0, 1, 0);
  } else if ([button bezelStyle] == NSRoundedBezelStyle) {
    NSControlSize size = [[button cell] controlSize];
    switch (size) {
#if defined(MAC_OS_VERSION_11_0)
      case NSControlSizeLarge:
#endif
      case NSRegularControlSize: return nu::InsetsF(4, 6, 7, 6);
      case NSSmallControlSize: return nu::InsetsF(4, 5, 6, 5);
      case NSMiniControlSize: return nu::InsetsF(0, 1, 1, 1);
    }
  }
  return nu::InsetsF();
}

}  // namespace

@interface NUButton : NSButton<NUViewMethods> {
 @private
  nu::Button* shell_;
  nu::NUViewPrivate private_;
}
- (id)initWithShell:(nu::Button*)shell;
- (IBAction)onClick:(id)sender;
@end

@implementation NUButton

- (id)initWithShell:(nu::Button*)shell {
  if ((self = [super init])) {
    shell_ = shell;
    [self setTarget:self];
    [self setAction:@selector(onClick:)];
  }
  return self;
}

- (IBAction)onClick:(id)sender {
  shell_->on_click.Emit(shell_);
}

- (nu::NUViewPrivate*)nuPrivate {
  return &private_;
}

- (void)setNUFont:(nu::Font*)font {
  [self.cell setFont:font->GetNative()];
}

- (void)setNUColor:(nu::Color)color {
  base::scoped_nsobject<NSMutableAttributedString> colored_title(
      [[NSMutableAttributedString alloc]
          initWithAttributedString:[self attributedTitle]]);
  [colored_title addAttribute:NSForegroundColorAttributeName
                        value:color.ToNSColor()
                        range:NSMakeRange(0, [colored_title length])];
  [self setAttributedTitle:colored_title];
}

- (void)setNUBackgroundColor:(nu::Color)color {
  [self.cell setBackgroundColor:color.ToNSColor()];
}

- (void)setNUEnabled:(BOOL)enabled {
  [self setEnabled:enabled];
}

- (BOOL)isNUEnabled {
  return [self isEnabled];
}

// The default NSButton includes the button's shadow area as its frame, and it
// gives us wrong coordinates to calculate layout. This method overrides
// setFrame: frame: to ignore the shadow frame.
- (void)setFrame:(NSRect)frame {
  nu::InsetsF border(GetButtonInsets(self));
  nu::RectF bounds(frame);
  bounds.Inset(-border);
  [super setFrame:bounds.ToCGRect()];
}

- (NSRect)frame {
  nu::InsetsF border(GetButtonInsets(self));
  nu::RectF bounds([super frame]);
  bounds.Inset(border);
  return bounds.ToCGRect();
}

@end

namespace nu {

Button::Button(const std::string& title, Type type) {
  NSButton* button = [[NUButton alloc] initWithShell:this];
  if (type == Type::Normal)
    [button setBezelStyle:NSRoundedBezelStyle];
  else if (type == Type::Checkbox)
    [button setButtonType:NSSwitchButton];
  else if (type == Type::Radio)
    [button setButtonType:NSRadioButton];
  TakeOverView(button);
  SetTitle(title);
}

void Button::MakeDefault() {
  NSButton* button = static_cast<NSButton*>(GetNative());
  [button setKeyEquivalent:@"\r"];
}

void Button::PlatformSetTitle(const std::string& title) {
  static_cast<NSButton*>(GetNative()).title = base::SysUTF8ToNSString(title);
}

std::string Button::GetTitle() const {
  return base::SysNSStringToUTF8(static_cast<NSButton*>(GetNative()).title);
}

void Button::SetChecked(bool checked) {
  static_cast<NSButton*>(GetNative()).state = checked ? NSOnState : NSOffState;
}

bool Button::IsChecked() const {
  return static_cast<NSButton*>(GetNative()).state == NSOnState;
}

void Button::PlatformSetImage(Image* image) {
  static_cast<NSButton*>(GetNative()).image = image->GetNative();
}

void Button::SetButtonStyle(Style style) {
  auto* button = static_cast<NSButton*>(GetNative());
  button.bezelStyle = static_cast<NSBezelStyle>(style);
  UpdateDefaultStyle();
}

void Button::SetHasBorder(bool yes) {
  static_cast<NSButton*>(GetNative()).bordered = yes;
  UpdateDefaultStyle();
}

bool Button::HasBorder() const {
  return static_cast<NSButton*>(GetNative()).bordered;
}

SizeF Button::GetMinimumSize() const {
  auto* button = static_cast<NSButton*>(GetNative());
  RectF bounds = RectF(SizeF([[button cell] cellSize]));
  bounds.Inset(GetButtonInsets(button));
  return bounds.size();
}

}  // namespace nu
