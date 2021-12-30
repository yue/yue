// Copyright 2019 Cheng Zhao. All rights reserved.
// Copyright 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#include "nativeui/win/drag_drop/data_object.h"

#include <algorithm>
#include <iterator>
#include <string>
#include <utility>

#include "base/notreached.h"
#include "base/strings/utf_string_conversions.h"
#include "base/win/scoped_hglobal.h"
#include "nativeui/win/drag_drop/clipboard_util.h"

namespace nu {

namespace {

STGMEDIUM* GetStorageForBytes(const void* data, size_t bytes) {
  HANDLE handle = GlobalAlloc(GPTR, static_cast<int>(bytes));
  if (handle) {
    base::win::ScopedHGlobal<uint8_t*> scoped(handle);
    memcpy(scoped.get(), data, bytes);
  }

  STGMEDIUM* storage = new STGMEDIUM;
  storage->hGlobal = handle;
  storage->tymed = TYMED_HGLOBAL;
  storage->pUnkForRelease = NULL;
  return storage;
}

template<typename T>
STGMEDIUM* GetStorageForString(const std::basic_string<T>& data) {
  return GetStorageForBytes(
      data.c_str(),
      (data.size() + 1) * sizeof(typename std::basic_string<T>::value_type));
}

STGMEDIUM* GetStorageForImage(Image* image) {
  STGMEDIUM* storage = new STGMEDIUM;
  storage->hBitmap = GetBitmapFromImage(image);
  storage->tymed = TYMED_GDI;
  storage->pUnkForRelease = NULL;
  return storage;
}

// Safely makes a copy of all of the relevant bits of a FORMATETC object.
// TODO(zcbenz): We should not modify |source|.
void CloneFormatEtc(FORMATETC* source, FORMATETC* clone) {
  *clone = *source;
  if (source->ptd) {
    source->ptd =
        static_cast<DVTARGETDEVICE*>(CoTaskMemAlloc(sizeof(DVTARGETDEVICE)));
    *(clone->ptd) = *(source->ptd);
  }
}

// The following function, DuplicateMedium, is derived from WCDataObject.cpp
// in the WebKit source code. This is the license information for the file:
/*
 * Copyright (C) 2007 Apple Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
void DuplicateMedium(CLIPFORMAT source_clipformat,
                     STGMEDIUM* source,
                     STGMEDIUM* destination) {
  switch (source->tymed) {
    case TYMED_HGLOBAL:
      destination->hGlobal =
          static_cast<HGLOBAL>(OleDuplicateData(
              source->hGlobal, source_clipformat, 0));
      break;
    case TYMED_MFPICT:
      destination->hMetaFilePict =
          static_cast<HMETAFILEPICT>(OleDuplicateData(
              source->hMetaFilePict, source_clipformat, 0));
      break;
    case TYMED_GDI:
      destination->hBitmap =
          static_cast<HBITMAP>(OleDuplicateData(
              source->hBitmap, source_clipformat, 0));
      break;
    case TYMED_ENHMF:
      destination->hEnhMetaFile =
          static_cast<HENHMETAFILE>(OleDuplicateData(
              source->hEnhMetaFile, source_clipformat, 0));
      break;
    case TYMED_FILE:
      destination->lpszFileName =
          static_cast<LPOLESTR>(OleDuplicateData(
              source->lpszFileName, source_clipformat, 0));
      break;
    case TYMED_ISTREAM:
      destination->pstm = source->pstm;
      destination->pstm->AddRef();
      break;
    case TYMED_ISTORAGE:
      destination->pstg = source->pstg;
      destination->pstg->AddRef();
      break;
  }

  destination->tymed = source->tymed;
  destination->pUnkForRelease = source->pUnkForRelease;
  if (destination->pUnkForRelease)
    destination->pUnkForRelease->AddRef();
}

}  // namespace

///////////////////////////////////////////////////////////////////////////////
// FormatEtcEnumerator

// This object implements an enumeration interface. The existence of an
// implementation of this interface is exposed to clients through
// OSExchangeData's EnumFormatEtc method. Our implementation is nobody's
// business but our own, so it lives in this file.
//
// This Windows API is truly a gem. It wants to be an enumerator but assumes
// some sort of sequential data (why not just use an array?). See comments
// throughout.
class FormatEtcEnumerator final : public IEnumFORMATETC {
 public:
  explicit FormatEtcEnumerator(std::vector<StoredDataInfo>* contents);
  ~FormatEtcEnumerator();

  FormatEtcEnumerator& operator=(const FormatEtcEnumerator&) = delete;
  FormatEtcEnumerator(const FormatEtcEnumerator&) = delete;

  // IEnumFORMATETC implementation:
  HRESULT __stdcall Next(ULONG count,
                         FORMATETC* elements_array,
                         ULONG* elements_fetched) override;
  HRESULT __stdcall Skip(ULONG skip_count) override;
  HRESULT __stdcall Reset() override;
  HRESULT __stdcall Clone(IEnumFORMATETC** clone) override;

  // IUnknown implementation:
  HRESULT __stdcall QueryInterface(const IID& iid, void** object) override;
  ULONG __stdcall AddRef() override;
  ULONG __stdcall Release() override;

 private:
  // This can only be called from |CloneFromOther|, since it initializes the
  // contents_ from the other enumerator's contents.
  FormatEtcEnumerator() : cursor_(0), ref_count_(0) {
  }

  // Clone a new FormatEtc from another instance of this enumeration.
  static FormatEtcEnumerator* CloneFromOther(FormatEtcEnumerator* other);

 private:
  // We are _forced_ to use a vector as our internal data model as Windows'
  // IEnumFORMATETC API assumes a deterministic ordering of elements through
  // methods like Next and Skip. This exposes the underlying data structure to
  // the user. Bah.
  std::vector<FORMATETC> contents_;

  // The cursor of the active enumeration - an index into |contents_|.
  size_t cursor_;

  LONG ref_count_;
};

FormatEtcEnumerator::FormatEtcEnumerator(std::vector<StoredDataInfo>* contents)
    : cursor_(0), ref_count_(0) {
  // Copy FORMATETC data from our source into ourselves.
  for (StoredDataInfo& info : *contents) {
    FORMATETC format_etc = {0};
    CloneFormatEtc(&info.format_etc, &format_etc);
    contents_.emplace_back(std::move(format_etc));
  }
}

FormatEtcEnumerator::~FormatEtcEnumerator() {
}

STDMETHODIMP FormatEtcEnumerator::Next(
    ULONG count, FORMATETC* elements_array, ULONG* elements_fetched) {
  // MSDN says |elements_fetched| is allowed to be NULL if count is 1.
  if (!elements_fetched)
    DCHECK_EQ(count, 1ul);

  // This method copies count elements into |elements_array|.
  ULONG index = 0;
  while (cursor_ < contents_.size() && index < count) {
    CloneFormatEtc(&contents_[cursor_], &elements_array[index]);
    ++cursor_;
    ++index;
  }
  // The out param is for how many we actually copied.
  if (elements_fetched)
    *elements_fetched = index;

  // If the two don't agree, then we fail.
  return index == count ? S_OK : S_FALSE;
}

STDMETHODIMP FormatEtcEnumerator::Skip(ULONG skip_count) {
  cursor_ += skip_count;
  // MSDN implies it's OK to leave the enumerator trashed.
  // "Whatever you say, boss"
  return cursor_ <= contents_.size() ? S_OK : S_FALSE;
}

STDMETHODIMP FormatEtcEnumerator::Reset() {
  cursor_ = 0;
  return S_OK;
}

STDMETHODIMP FormatEtcEnumerator::Clone(IEnumFORMATETC** clone) {
  // Clone the current enumerator in its exact state, including cursor.
  FormatEtcEnumerator* e = CloneFromOther(this);
  e->AddRef();
  *clone = e;
  return S_OK;
}

STDMETHODIMP FormatEtcEnumerator::QueryInterface(const IID& iid,
                                                 void** object) {
  *object = NULL;
  if (IsEqualIID(iid, IID_IUnknown) || IsEqualIID(iid, IID_IEnumFORMATETC)) {
    *object = this;
  } else {
    return E_NOINTERFACE;
  }
  AddRef();
  return S_OK;
}

ULONG FormatEtcEnumerator::AddRef() {
  return InterlockedIncrement(&ref_count_);
}

ULONG FormatEtcEnumerator::Release() {
  if (InterlockedDecrement(&ref_count_) == 0) {
    ULONG copied_refcnt = ref_count_;
    delete this;
    return copied_refcnt;
  }
  return ref_count_;
}

// static
FormatEtcEnumerator* FormatEtcEnumerator::CloneFromOther(
    FormatEtcEnumerator* other) {
  FormatEtcEnumerator* e = new FormatEtcEnumerator;
  // Copy FORMATETC data from our source into ourselves.
  std::transform(other->contents_.begin(), other->contents_.end(),
                 std::back_inserter(e->contents_),
                 [](FORMATETC& format_etc) {
                   FORMATETC clone = {0};
                   CloneFormatEtc(&format_etc, &clone);
                   return clone;
                 });
  // Carry over
  e->cursor_ = other->cursor_;
  return e;
}

///////////////////////////////////////////////////////////////////////////////
// StoredDataInfo implementation:

StoredDataInfo::StoredDataInfo(const FORMATETC& format_etc,
                               STGMEDIUM* medium)
    : format_etc(format_etc), medium(medium), owns_medium(true) {
}

StoredDataInfo::StoredDataInfo(StoredDataInfo&& other) : owns_medium(false) {
  (*this) = std::move(other);
}

StoredDataInfo::~StoredDataInfo() {
  if (owns_medium) {
    ReleaseStgMedium(medium);
    delete medium;
  }
}

StoredDataInfo& StoredDataInfo::operator=(StoredDataInfo&& other) {
  this->~StoredDataInfo();
  format_etc = std::move(other.format_etc);
  medium = other.medium;
  owns_medium = other.owns_medium;
  other.medium = nullptr;
  other.owns_medium = false;
  return *this;
}

///////////////////////////////////////////////////////////////////////////////
// DataObject implementation:

DataObject::DataObject(std::vector<Clipboard::Data> objects) {
  using Data = Clipboard::Data;

  for (auto& data : objects) {
    FORMATETC format = {0};
    if (!GetFormatEtc(data.type(), &format))
      continue;

    switch (data.type()) {
      case Data::Type::Text:
        contents_.emplace_back(
            format, GetStorageForString(base::UTF8ToWide(data.str())));
        break;
      case Data::Type::HTML: {
        std::string cf_html = HtmlToCFHtml(data.str(), "about:blank");
        contents_.emplace_back(
            format, GetStorageForBytes(cf_html.c_str(), cf_html.size()));
      }
      case Data::Type::Image: {
        format.tymed = TYMED_GDI;
        contents_.emplace_back(format, GetStorageForImage(data.image()));
        break;
      }
      case Data::Type::FilePaths:
        contents_.emplace_back(
            format, GetStorageForFileNames(data.file_paths()));
        break;
      case Data::Type::None:
        break;
      default:
        NOTREACHED() << "Invalid type: " << static_cast<int>(data.type());
    }
  }
}

DataObject::~DataObject() {}

void DataObject::RemoveData(const FORMATETC& format) {
  if (format.ptd)
    return;  // Don't attempt to compare target devices.

  for (auto i = contents_.begin(); i != contents_.end(); ++i) {
    if (!i->format_etc.ptd &&
        format.cfFormat == i->format_etc.cfFormat &&
        format.dwAspect == i->format_etc.dwAspect &&
        format.lindex == i->format_etc.lindex &&
        format.tymed == i->format_etc.tymed) {
      contents_.erase(i);
      return;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// DataObject, IDataObject implementation:

HRESULT DataObject::GetData(FORMATETC* format_etc, STGMEDIUM* medium) {
  for (const StoredDataInfo& content : contents_) {
    if (content.format_etc.cfFormat == format_etc->cfFormat &&
        content.format_etc.lindex == format_etc->lindex &&
        (content.format_etc.tymed & format_etc->tymed)) {
      if (content.medium)
        DuplicateMedium(content.format_etc.cfFormat, content.medium, medium);
      return S_OK;
    }
  }
  return DV_E_FORMATETC;
}

HRESULT DataObject::GetDataHere(FORMATETC* format_etc,
                                STGMEDIUM* medium) {
  return DATA_E_FORMATETC;
}

HRESULT DataObject::QueryGetData(FORMATETC* format_etc) {
  for (const StoredDataInfo& content : contents_) {
    if (content.format_etc.cfFormat == format_etc->cfFormat)
      return S_OK;
  }
  return DV_E_FORMATETC;
}

HRESULT DataObject::GetCanonicalFormatEtc(
    FORMATETC* format_etc, FORMATETC* result) {
  format_etc->ptd = NULL;
  return E_NOTIMPL;
}

HRESULT DataObject::SetData(
    FORMATETC* format_etc, STGMEDIUM* medium, BOOL should_release) {
  RemoveData(*format_etc);

  STGMEDIUM* local_medium = new STGMEDIUM;
  if (should_release) {
    *local_medium = *medium;
  } else {
    DuplicateMedium(format_etc->cfFormat, medium, local_medium);
  }

  StoredDataInfo info(*format_etc, local_medium);
  info.medium->tymed = format_etc->tymed;
  info.owns_medium = !!should_release;
  // Make newly added data appear first.
  contents_.insert(contents_.begin(), std::move(info));

  return S_OK;
}

HRESULT DataObject::EnumFormatEtc(
    DWORD direction, IEnumFORMATETC** enumerator) {
  if (direction == DATADIR_GET) {
    FormatEtcEnumerator* e = new FormatEtcEnumerator(&contents_);
    e->AddRef();
    *enumerator = e;
    return S_OK;
  }
  return E_NOTIMPL;
}

HRESULT DataObject::DAdvise(
    FORMATETC* format_etc, DWORD advf, IAdviseSink* sink, DWORD* connection) {
  return OLE_E_ADVISENOTSUPPORTED;
}

HRESULT DataObject::DUnadvise(DWORD connection) {
  return OLE_E_ADVISENOTSUPPORTED;
}

HRESULT DataObject::EnumDAdvise(IEnumSTATDATA** enumerator) {
  return OLE_E_ADVISENOTSUPPORTED;
}

///////////////////////////////////////////////////////////////////////////////
// DataObject, IUnknown implementation:

HRESULT DataObject::QueryInterface(const IID& iid, void** object) {
  if (!object)
    return E_POINTER;
  if (IsEqualIID(iid, IID_IDataObject) || IsEqualIID(iid, IID_IUnknown)) {
    *object = static_cast<IDataObject*>(this);
  } else {
    *object = NULL;
    return E_NOINTERFACE;
  }
  AddRef();
  return S_OK;
}

ULONG DataObject::AddRef() {
  return ++ref_count_;
}

ULONG DataObject::Release() {
  if (--ref_count_ == 0) {
    delete this;
    return 0U;
  }
  return ref_count_;
}

}  // namespace nu
