// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/table.h"

#include "base/mac/scoped_nsobject.h"
#include "nativeui/gfx/font.h"
#include "nativeui/mac/nu_private.h"
#include "nativeui/mac/nu_table_cell.h"
#include "nativeui/mac/nu_table_column.h"
#include "nativeui/mac/nu_table_data_source.h"
#include "nativeui/mac/nu_view.h"

@interface NUTableDelegate : NSObject<NSTableViewDelegate> {
 @private
  nu::Table* shell_;
}
- (id)initWithShell:(nu::Table*)shell;
@end

@implementation NUTableDelegate

- (id)initWithShell:(nu::Table*)shell {
  if ((self = [super init]))
    shell_ = shell;
  return self;
}

- (NSView*)tableView:(NSTableView*)tableView
  viewForTableColumn:(NSTableColumn*)nsTableColumn
                 row:(NSInteger)row {
  auto* tableColumn = static_cast<NUTableColumn*>(nsTableColumn);
  auto* reuse = [tableView makeViewWithIdentifier:[tableColumn identifier]
                                            owner:self];
  NUTableCell* tableCell = nullptr;
  if (reuse)
    tableCell = static_cast<NUTableCell*>(reuse);
  else
    tableCell = [[[NUTableCell alloc]
        initWithColumnOptions:[tableColumn options]] autorelease];
  [tableCell setTableModel:shell_->GetModel()
                    column:[tableColumn columnInModel]
                       row:row];
  return tableCell;
}

@end

@interface NUTable : NSScrollView<NUViewMethods> {
 @private
  base::scoped_nsobject<NSTableView> tableView_;
  base::scoped_nsobject<NUTableDelegate> delegate_;
  base::scoped_nsobject<NUTableDataSource> dataSource_;
  base::scoped_nsobject<NSTableHeaderView> headerView_;
  nu::NUViewPrivate private_;
}
- (id)initWithShell:(nu::Table*)shell;
- (void)setColumnsVisible:(bool)visible;
@end

@implementation NUTable

- (id)initWithShell:(nu::Table*)shell {
  if ((self = [super init])) {
    tableView_.reset([[NSTableView alloc] init]);
    delegate_.reset([[NUTableDelegate alloc] initWithShell:shell]);
    [tableView_ setDelegate:delegate_];
    [self setBorderType:NSNoBorder];
    [self setHasVerticalScroller:YES];
    [self setHasHorizontalScroller:YES];
    [self setDocumentView:tableView_];
  }
  return self;
}

- (void)setColumnsVisible:(bool)visible {
  if (visible) {
    [tableView_ setHeaderView:headerView_];
    headerView_.reset();
  } else {
    headerView_.reset([tableView_ headerView]);
    [tableView_ setHeaderView:nil];
  }
}

- (void)setModel:(nu::TableModel*)model {
  if (model)
    dataSource_.reset([[NUTableDataSource alloc] initWithTableModel:model]);
  else
    dataSource_.reset();
  [tableView_ setDataSource:dataSource_];
  // Somehow the content may have some offset, scroll to top.
  [tableView_ scrollRowToVisible:0];
}

- (nu::NUViewPrivate*)nuPrivate {
  return &private_;
}

- (void)setNUFont:(nu::Font*)font {
  [tableView_ setFont:font->GetNative()];
}

- (void)setNUColor:(nu::Color)color {
}

- (void)setNUBackgroundColor:(nu::Color)color {
  [tableView_ setBackgroundColor:color.ToNSColor()];
}

- (void)setNUEnabled:(BOOL)enabled {
  [tableView_ setEnabled:enabled];
}

- (BOOL)isNUEnabled {
  return [tableView_ isEnabled];
}

@end

namespace nu {

NativeView Table::PlatformCreate() {
  return [[NUTable alloc] initWithShell:this];
}

void Table::PlatformDestroy() {
}

void Table::PlatformSetModel(TableModel* model) {
  auto* table = static_cast<NUTable*>(GetNative());
  [table setModel:model];
}

void Table::AddColumnWithOptions(const std::string& title,
                                 const ColumnOptions& options) {
  auto* tableView = static_cast<NSTableView*>(
      [static_cast<NUTable*>(GetNative()) documentView]);
  base::scoped_nsobject<NUTableColumn> tableColumn(
     [[NUTableColumn alloc] initWithTable:this
                                    title:title
                                  options:options]);
  [tableView addTableColumn:tableColumn];
}

int Table::GetColumnCount() const {
  auto* tableView = static_cast<NSTableView*>(
      [static_cast<NUTable*>(GetNative()) documentView]);
  return [tableView numberOfColumns];
}

void Table::SetColumnsVisible(bool visible) {
  if (visible == IsColumnsVisible())
    return;
  auto* table = static_cast<NUTable*>(GetNative());
  [table setColumnsVisible:visible];
}

bool Table::IsColumnsVisible() const {
  auto* tableView = static_cast<NSTableView*>(
      [static_cast<NUTable*>(GetNative()) documentView]);
  return [tableView headerView];
}

void Table::SetRowHeight(float height) {
  auto* tableView = static_cast<NSTableView*>(
      [static_cast<NUTable*>(GetNative()) documentView]);
  [tableView setRowHeight:height];
}

float Table::GetRowHeight() const {
  auto* tableView = static_cast<NSTableView*>(
      [static_cast<NUTable*>(GetNative()) documentView]);
  return [tableView rowHeight];
}

void Table::SelectRow(int row) {
  auto* tableView = static_cast<NSTableView*>(
      [static_cast<NUTable*>(GetNative()) documentView]);
  [tableView selectRowIndexes:[NSIndexSet indexSetWithIndex:row]
         byExtendingSelection:NO];
}

int Table::GetSelectedRow() const {
  auto* tableView = static_cast<NSTableView*>(
      [static_cast<NUTable*>(GetNative()) documentView]);
  return [tableView selectedRow];
}

void Table::NotifyRowInsertion(uint32_t row) {
  auto* tableView = static_cast<NSTableView*>(
      [static_cast<NUTable*>(GetNative()) documentView]);
  [tableView insertRowsAtIndexes:[NSIndexSet indexSetWithIndex:row]
                   withAnimation:NSTableViewAnimationEffectNone];
}

void Table::NotifyRowDeletion(uint32_t row) {
  auto* tableView = static_cast<NSTableView*>(
      [static_cast<NUTable*>(GetNative()) documentView]);
  [tableView removeRowsAtIndexes:[NSIndexSet indexSetWithIndex:row]
                   withAnimation:NSTableViewAnimationEffectNone];
}

void Table::NotifyValueChange(uint32_t column, uint32_t row) {
  auto* tableView = static_cast<NSTableView*>(
      [static_cast<NUTable*>(GetNative()) documentView]);
  [tableView reloadDataForRowIndexes:[NSIndexSet indexSetWithIndex:row]
                       columnIndexes:[NSIndexSet indexSetWithIndex:column]];
}

}  // namespace nu
