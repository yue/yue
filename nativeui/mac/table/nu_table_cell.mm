// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/mac/table/nu_table_cell.h"

#include "base/mac/scoped_nsobject.h"
#include "base/strings/sys_string_conversions.h"
#include "base/values.h"
#include "nativeui/gfx/mac/painter_mac.h"
#include "nativeui/mac/table/nu_wrapped_value.h"
#include "nativeui/mac/value_conversion.h"
#include "nativeui/table_model.h"

@interface NUCustomTableCellView : NSView {
 @private
  nu::Table::ColumnOptions options_;
  base::Value value_;
}
- (id)initWithColumnOptions:(const nu::Table::ColumnOptions&)options;
- (void)setValue:(base::Value)value;
@end

@implementation NUCustomTableCellView

- (id)initWithColumnOptions:(const nu::Table::ColumnOptions&)options {
  if ((self = [super init]))
    options_ = options;
  return self;
}

- (void)setValue:(base::Value)value {
  value_ = std::move(value);
}

- (void)drawRect:(NSRect)dirtyRect {
  if (options_.on_draw) {
    nu::PainterMac painter(self);
    options_.on_draw(&painter, nu::RectF(dirtyRect), value_);
  }
}

@end

@implementation NUTableCell

- (id)initWithColumnOptions:(const nu::Table::ColumnOptions&)options {
  if ((self = [super init])) {
    type_ = options.type;
    model_ = nullptr;

    switch (type_) {
      case nu::Table::ColumnType::Text:
      case nu::Table::ColumnType::Edit: {
        base::scoped_nsobject<NSTextField> textField(
            [[NSTextField alloc] initWithFrame:NSZeroRect]);
        if (type_ == nu::Table::ColumnType::Edit) {
          [textField setTarget:self];
          [textField setAction:@selector(onEditDone:)];
        } else {
          [textField setEditable:NO];
        }
        [textField setDrawsBackground:NO];
        [textField setBezeled:NO];
        [textField setSelectable:YES];
        [self setTextField:textField];
        [self addSubview:textField];
        // Remove default layout.
        [textField setTranslatesAutoresizingMaskIntoConstraints:NO];
        // Make the textField has vertical center alignment.
        [NSLayoutConstraint activateConstraints:@[
          // Vertical center.
          [NSLayoutConstraint constraintWithItem:textField attribute:NSLayoutAttributeCenterY relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeCenterY multiplier:1 constant:0],
          // Take full width.
          [NSLayoutConstraint constraintWithItem:textField attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeWidth multiplier:1 constant:0],
        ]];
        break;
      }

      case nu::Table::ColumnType::Custom: {
        base::scoped_nsobject<NUCustomTableCellView> customView(
            [[NUCustomTableCellView alloc] initWithColumnOptions:options]);
        [customView setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
        [self addSubview:customView];
        break;
      }
    }
  }
  return self;
}

- (void)setTableModel:(nu::TableModel*)model
               column:(uint32_t)column
                  row:(uint32_t)row {
  model_ = model;
  column_ = column;
  row_ = row;
}

- (void)setObjectValue:(id)obj {
  [super setObjectValue:obj];

  // A nil value may be passed to do clear work.
  if (!obj)
    return;

  // Note: we have to store a copy of the value, the reference will be away
  // after current stack ends.
  base::Value value = [static_cast<NUWrappedValue*>(obj) pass];
  switch (type_) {
    case nu::Table::ColumnType::Text:
    case nu::Table::ColumnType::Edit: {
      if (value.is_string())
        self.textField.stringValue = base::SysUTF8ToNSString(value.GetString());
      break;
    }

    case nu::Table::ColumnType::Custom: {
      auto* customView = static_cast<NUCustomTableCellView*>(
          [[self subviews] firstObject]);
      [customView setValue:std::move(value)];
      [customView setNeedsDisplay:YES];
      break;
    }
  }
}

- (void)onEditDone:(id)sender {
  if (!model_)
    return;
  model_->SetValue(column_, row_,
                   base::Value(base::SysNSStringToUTF8([sender stringValue])));
}

@end
