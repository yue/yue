// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/table.h"

#include "base/apple/scoped_nsobject.h"
#include "nativeui/gfx/font.h"
#include "nativeui/mac/nu_private.h"
#include "nativeui/mac/nu_view.h"
#include "nativeui/mac/table/nu_table_cell.h"
#include "nativeui/mac/table/nu_table_column.h"
#include "nativeui/mac/table/nu_table_data_source.h"

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
  auto* tableCell = static_cast<NUTableCell*>(
      [tableView makeViewWithIdentifier:[tableColumn identifier] owner:self]);
  if (!tableCell) {
    tableCell = [[[NUTableCell alloc]
        initWithShell:shell_
        columnOptions:[tableColumn options]] autorelease];
    [tableCell setIdentifier:[tableColumn identifier]];
  }
  [tableCell setTableModel:shell_->GetModel()
                    column:[tableColumn columnInModel]
                       row:row];
  return tableCell;
}

- (void)tableViewSelectionDidChange:(NSNotification*)notification {
  shell_->on_selection_change.Emit(shell_);
}

- (void)doubleAction:(id)sender {
  NSTableView* table = static_cast<NSTableView*>(sender);
  if ([table clickedRow] != -1)
    shell_->on_row_activate.Emit(shell_, [table clickedRow]);
}

@end

@interface NUTable : NSScrollView<NUViewMethods> {
 @private
  base::apple::scoped_nsobject<NSTableView> tableView_;
  base::apple::scoped_nsobject<NUTableDelegate> delegate_;
  base::apple::scoped_nsobject<NUTableDataSource> dataSource_;
  base::apple::scoped_nsobject<NSTableHeaderView> headerView_;
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
    [tableView_ setTarget:delegate_];
    [tableView_ setDoubleAction:@selector(doubleAction:)];
    [self setBorderType:NSNoBorder];
    [self setHasVerticalScroller:YES];
    [self setHasHorizontalScroller:YES];
    [self setDocumentView:tableView_];
    // Table with header is incorrectly scrolled.
    // https://bugs.eclipse.org/bugs/show_bug.cgi?id=575259
    [self setFrame:NSMakeRect(0, 0, 100, 100)];
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
  base::apple::scoped_nsobject<NUTableColumn> tableColumn(
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

void Table::SetHasBorder(bool yes) {
  auto* table = static_cast<NUTable*>(GetNative());
  return [table setBorderType:NSBezelBorder];
}

bool Table::HasBorder() const {
  auto* table = static_cast<NUTable*>(GetNative());
  return [table borderType] != NSNoBorder;
}

void Table::EnableMultipleSelection(bool enable) {
  auto* tableView = static_cast<NSTableView*>(
      [static_cast<NUTable*>(GetNative()) documentView]);
  [tableView setAllowsMultipleSelection:enable];
}

bool Table::IsMultipleSelectionEnabled() const {
  auto* tableView = static_cast<NSTableView*>(
      [static_cast<NUTable*>(GetNative()) documentView]);
  return [tableView allowsMultipleSelection];
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

void Table::SelectRows(std::set<int> rows) {
  auto* tableView = static_cast<NSTableView*>(
      [static_cast<NUTable*>(GetNative()) documentView]);
  NSMutableIndexSet* index = [NSMutableIndexSet indexSet];
  for (int row : rows)
    [index addIndex:row];
  [tableView selectRowIndexes:index byExtendingSelection:NO];
}

std::set<int> Table::GetSelectedRows() const {
  auto* tableView = static_cast<NSTableView*>(
      [static_cast<NUTable*>(GetNative()) documentView]);
  NSIndexSet* index = [tableView selectedRowIndexes];
  std::set<int> selection;
  for (NSUInteger i = [index firstIndex];
       i != NSNotFound;
       i = [index indexGreaterThanIndex:i]) {
    selection.insert(i);
  }
  return selection;
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
