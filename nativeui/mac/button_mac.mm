// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/button.h"

#include "base/strings/sys_string_conversions.h"
#include "nativeui/gfx/font.h"
#include "nativeui/gfx/geometry/insets_f.h"
#include "nativeui/mac/nu_private.h"
#include "nativeui/mac/nu_view.h"

namespace {

// The insets numbers are from wxWidgets:
// wxWidgets/src/osx/cocoa/button.mm
nu::InsetsF GetButtonInsets(NSButton* button) {
  if ([button bezelStyle] == NSSmallSquareBezelStyle) {
    return nu::InsetsF(1, 0, 1, 0);
  } else if ([button bezelStyle] == NSRoundedBezelStyle) {
    NSControlSize size = [[button cell] controlSize];
    switch (size) {
      case NSRegularControlSize: return nu::InsetsF(4, 6, 7, 6);
      case NSSmallControlSize: return nu::InsetsF(4, 5, 6, 5);
      case NSMiniControlSize: return nu::InsetsF(0, 1, 1, 1);
    }
  }
  return nu::InsetsF();
}

}  // namespace

@interface NUButton : NSButton<NUView> {
 @private
  nu::NUPrivate private_;
}
- (nu::NUPrivate*)nuPrivate;
- (void)setNUFont:(nu::Font*)font;
- (void)setNUColor:(nu::Color)color;
- (void)setNUBackgroundColor:(nu::Color)color;
@end

@implementation NUButton

- (nu::NUPrivate*)nuPrivate {
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

// The button delegate to catch click events.
@interface NUButtonDelegate : NSObject {
 @private
  nu::Button* shell_;
}
- (id)initWithShell:(nu::Button*)shell;
- (IBAction)onClick:(id)sender;
@end

@implementation NUButtonDelegate

- (id)initWithShell:(nu::Button*)shell {
  if ((self = [super init]))
    shell_ = shell;
  return self;
}

- (IBAction)onClick:(id)sender {
  shell_->on_click.Emit();
}

@end

namespace nu {

Button::Button(const std::string& title, Type type) {
  NSButton* button = [[NUButton alloc] init];
  if (type == Type::Normal)
    [button setBezelStyle:NSRoundedBezelStyle];
  else if (type == Type::Checkbox)
    [button setButtonType:NSSwitchButton];
  else if (type == Type::Radio)
    [button setButtonType:NSRadioButton];

  [button setTarget:[[NUButtonDelegate alloc] initWithShell:this]];
  [button setAction:@selector(onClick:)];
  TakeOverView(button);

  SetTitle(title);
}

Button::~Button() {
  NSButton* button = static_cast<NSButton*>(GetNative());
  [button.target release];
  [button setTarget:nil];
}

void Button::SetTitle(const std::string& title) {
  NSButton* button = static_cast<NSButton*>(GetNative());
  button.title = base::SysUTF8ToNSString(title);

  // Calculate the preferred size by creating a new button.
  RectF bounds = RectF(SizeF([[button cell] cellSize]));
  bounds.Inset(GetButtonInsets(button));
  SetDefaultStyle(bounds.size());
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

}  // namespace nu
