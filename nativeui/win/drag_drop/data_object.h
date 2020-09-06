// Copyright 2019 Cheng Zhao. All rights reserved.
// Copyright 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#ifndef NATIVEUI_WIN_DRAG_DROP_DATA_OBJECT_H_
#define NATIVEUI_WIN_DRAG_DROP_DATA_OBJECT_H_

#include <objidl.h>
#include <shlobj.h>

#include <vector>

#include "nativeui/clipboard.h"

namespace nu {

// Our internal representation of stored data & type info.
struct StoredDataInfo {
  FORMATETC format_etc;
  STGMEDIUM* medium;
  bool owns_medium;

  StoredDataInfo(const FORMATETC& format_etc, STGMEDIUM* medium);
  StoredDataInfo(StoredDataInfo&&);
  ~StoredDataInfo();

  StoredDataInfo& operator=(StoredDataInfo&&);
};

// Provide data to drag and drop.
class DataObject : public IDataObject {
 public:
  explicit DataObject(std::vector<Clipboard::Data> objects);

  // IDataObject:
  HRESULT __stdcall GetData(FORMATETC* format_etc, STGMEDIUM* medium) override;
  HRESULT __stdcall GetDataHere(FORMATETC* format_etc,
                                STGMEDIUM* medium) override;
  HRESULT __stdcall QueryGetData(FORMATETC* format_etc) override;
  HRESULT __stdcall GetCanonicalFormatEtc(FORMATETC* format_etc,
                                          FORMATETC* result) override;
  HRESULT __stdcall SetData(FORMATETC* format_etc,
                            STGMEDIUM* medium,
                            BOOL should_release) override;
  HRESULT __stdcall EnumFormatEtc(DWORD direction,
                                  IEnumFORMATETC** enumerator) override;
  HRESULT __stdcall DAdvise(FORMATETC* format_etc,
                            DWORD advf,
                            IAdviseSink* sink,
                            DWORD* connection) override;
  HRESULT __stdcall DUnadvise(DWORD connection) override;
  HRESULT __stdcall EnumDAdvise(IEnumSTATDATA** enumerator) override;

  // IUnknown:
  HRESULT __stdcall QueryInterface(const IID& iid, void** object) override;
  ULONG __stdcall AddRef() override;
  ULONG __stdcall Release() override;

 private:
  friend class FormatEtcEnumerator;

  virtual ~DataObject();

  // Removes from contents_ the first data that matches |format|.
  void RemoveData(const FORMATETC& format);

  std::vector<StoredDataInfo> contents_;

  LONG ref_count_ = 0;
};

}  // namespace nu

#endif  // NATIVEUI_WIN_DRAG_DROP_DATA_OBJECT_H_
