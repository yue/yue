// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/toolbar.h"

#import <Cocoa/Cocoa.h>

#include <map>
#include <utility>

#include "base/mac/scoped_nsobject.h"
#include "base/strings/sys_string_conversions.h"
#include "nativeui/gfx/image.h"
#include "nativeui/view.h"

@interface NUToolbarDelegate : NSObject<NSToolbarDelegate> {
 @private
  nu::Toolbar* shell_;
  base::scoped_nsobject<NSMutableArray> default_identifiers_;
  base::scoped_nsobject<NSMutableArray> allowed_identifiers_;
  std::map<std::string,
           std::pair<nu::Toolbar::Item,
                     base::scoped_nsobject<NSToolbarItem>>> items_;
}
- (id)initWithShell:(nu::Toolbar*)shell;
- (void)setDefaultItemIdentifiers:(const std::vector<std::string>&)identifiers;
- (void)setAllowedItemIdentifiers:(const std::vector<std::string>&)identifiers;
@end

@implementation NUToolbarDelegate

- (id)initWithShell:(nu::Toolbar*)shell {
  if ((self = [super init])) {
    shell_ = shell;
    default_identifiers_.reset([NSMutableArray array]);
    allowed_identifiers_.reset([NSMutableArray array]);
  }
  return self;
}

- (void)setDefaultItemIdentifiers:(const std::vector<std::string>&)identifiers {
  default_identifiers_.reset([NSMutableArray arrayWithCapacity:identifiers.size()]);
  for (const auto& ident : identifiers)
    [default_identifiers_ addObject:[self translateIdentifier:ident]];
}

- (void)setAllowedItemIdentifiers:(const std::vector<std::string>&)identifiers {
  allowed_identifiers_.reset([NSMutableArray arrayWithCapacity:identifiers.size()]);
  for (const auto& ident : identifiers)
    [allowed_identifiers_ addObject:[self translateIdentifier:ident]];
}

- (NSToolbarItem*)toolbar:(NSToolbar*)toolbar
    itemForItemIdentifier:(NSString*)identifier
willBeInsertedIntoToolbar:(BOOL)flag {
  // Return the previous item if it has been requested before.
  std::string ident = base::SysNSStringToUTF8(identifier);
  auto it = items_.find(ident);
  if (it != items_.end())
    return it->second.second;

  // Checks.
  if (!shell_->get_item)
    return nil;
  auto config = shell_->get_item(shell_, ident);
  if (config.label.empty()) {
    // Empty label means undefined.
    return nil;
  }

  // Normal item or group item.
  NSToolbarItem* item;
  if (config.subitems.empty()) {
    item = [[NSToolbarItem alloc] initWithItemIdentifier:identifier];
  } else {
    NSToolbarItemGroup* group =
        [[NSToolbarItemGroup alloc] initWithItemIdentifier:identifier];
    // Get subitems.
    base::scoped_nsobject<NSMutableArray> subitems(
        [NSMutableArray arrayWithCapacity:config.subitems.size()]);
    for (const std::string& sub_ident : config.subitems) {
      [subitems addObject:[self toolbar:toolbar
                  itemForItemIdentifier:base::SysUTF8ToNSString(sub_ident)
              willBeInsertedIntoToolbar:flag]];
    }
    [group setSubitems:subitems];
    item = group;
  }

  // Read config.
  item.target = self;
  item.action = @selector(onClick:);
  item.label = base::SysUTF8ToNSString(config.label);
  if (config.image)
    item.image = config.image->GetNative();
  if (!config.max_size.IsEmpty())
    item.maxSize = config.max_size.ToCGSize();
  if (!config.min_size.IsEmpty())
    item.minSize = config.min_size.ToCGSize();
  if (config.view) {
    item.view = config.view->GetNative();
    // Align the items.
    item.view.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
  }
  // Save to map.
  items_.emplace(ident,
                 std::make_pair(
                     std::move(config),
                     base::scoped_nsobject<NSToolbarItem>(item)));
  return item;
}

- (NSArray<NSString*>*)toolbarDefaultItemIdentifiers:(NSToolbar*)toolbar {
  return default_identifiers_.get();
}

- (NSArray<NSString*>*)toolbarAllowedItemIdentifiers:(NSToolbar*)toolbar {
  return allowed_identifiers_.get();
}

- (void)onClick:(NSToolbarItem*)item {
  // Find the item.
  std::string ident = base::SysNSStringToUTF8(item.itemIdentifier);
  auto it = items_.find(ident);
  if (it == items_.end())
    return;

  // Emit.
  if (it->second.first.on_click)
    it->second.first.on_click(shell_, ident);
}

- (NSString*)translateIdentifier:(const std::string&)identifier {
  if (identifier == nu::Toolbar::kFlexibleSpaceItemIdentifier)
    return NSToolbarFlexibleSpaceItemIdentifier;
  else if (identifier == nu::Toolbar::kSpaceItemIdentifier)
    return NSToolbarSpaceItemIdentifier;
  else
    return base::SysUTF8ToNSString(identifier);
}

@end

namespace nu {

// static
const char Toolbar::kFlexibleSpaceItemIdentifier[] = "FlexibleSpaceItem";
const char Toolbar::kSpaceItemIdentifier[] = "SpaceItem";

Toolbar::Item::Item() = default;
Toolbar::Item::Item(Item&&) = default;
Toolbar::Item::~Item() = default;
Toolbar::Item& Toolbar::Item::operator=(Item&&) = default;

Toolbar::Toolbar(const std::string& identifier)
    : toolbar_([[NSToolbar alloc]
                   initWithIdentifier:base::SysUTF8ToNSString(identifier)]) {
  toolbar_.delegate = [[NUToolbarDelegate alloc] initWithShell:this];
}

Toolbar::~Toolbar() {
  [toolbar_.delegate release];
  [toolbar_ release];
}

void Toolbar::SetDefaultItemIdentifiers(
    const std::vector<std::string>& identifiers) {
  auto* delegate = static_cast<NUToolbarDelegate*>(toolbar_.delegate);
  [delegate setDefaultItemIdentifiers:identifiers];
}

void Toolbar::SetAllowedItemIdentifiers(
    const std::vector<std::string>& identifiers) {
  auto* delegate = static_cast<NUToolbarDelegate*>(toolbar_.delegate);
  [delegate setAllowedItemIdentifiers:identifiers];
}

void Toolbar::SetAllowCustomization(bool allow) {
  toolbar_.allowsUserCustomization = allow;
}

void Toolbar::SetDisplayMode(DisplayMode mode) {
  toolbar_.displayMode = static_cast<NSToolbarDisplayMode>(mode);
}

void Toolbar::SetVisible(bool visible) {
  toolbar_.visible = visible;
}

bool Toolbar::IsVisible() const {
  return toolbar_.visible;
}

std::string Toolbar::GetIdentifier() const {
  return base::SysNSStringToUTF8(toolbar_.identifier);
}

}  // namespace nu
