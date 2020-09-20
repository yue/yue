// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/table.h"

#include <utility>

#include "nativeui/table_model.h"

namespace nu {

// static
const char Table::kClassName[] = "Table";

Table::ColumnOptions::ColumnOptions() = default;

Table::ColumnOptions::ColumnOptions(const ColumnOptions& other) = default;

Table::ColumnOptions::~ColumnOptions() = default;

Table::Table() {
  TakeOverView(PlatformCreate());
}

Table::~Table() {
  PlatformDestroy();
  if (model_)
    model_->Unsubscribe(this);
}

void Table::SetModel(scoped_refptr<TableModel> model) {
  if (model_)
    model_->Unsubscribe(this);
  PlatformSetModel(model.get());
  model_ = std::move(model);
  if (model_)
    model_->Subscribe(this);
}

TableModel* Table::GetModel() {
  return model_.get();
}

void Table::AddColumn(const std::string& title) {
  AddColumnWithOptions(title, ColumnOptions());
}

const char* Table::GetClassName() const {
  return kClassName;
}

}  // namespace nu
