// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/image.h"

#include <shlwapi.h>
#include <wrl.h>

#include "base/win/scoped_hglobal.h"
#include "nativeui/gfx/win/gdiplus.h"

namespace nu {

Image::Image() : image_(new Gdiplus::Image(L"")) {}

Image::Image(const base::FilePath& path)
    : scale_factor_(GetScaleFactorFromFilePath(path)),
      image_(new Gdiplus::Image(path.value().c_str())) {}

Image::Image(const Buffer& buffer, float scale_factor)
    : scale_factor_(scale_factor) {
  HGLOBAL glob = ::GlobalAlloc(GPTR, buffer.size());
  {
    base::win::ScopedHGlobal<void*> global_lock(glob);
    memcpy(global_lock.get(), buffer.content(), buffer.size());
  }
  Microsoft::WRL::ComPtr<IStream> stream;
  ::CreateStreamOnHGlobal(glob, TRUE, &stream);
  image_ = new Gdiplus::Image(stream.Get());
}

Image::~Image() {
  delete image_;
}

bool Image::IsEmpty() const {
  Gdiplus::Image* image = const_cast<Gdiplus::Image*>(image_);
  return image->GetWidth() == 0 || image->GetHeight() == 0;
}

SizeF Image::GetSize() const {
  Gdiplus::Image* image = const_cast<Gdiplus::Image*>(image_);
  return ScaleSize(SizeF(image->GetWidth(), image->GetHeight()),
                   1.f / scale_factor_);
}

NativeImage Image::GetNative() const {
  return image_;
}

}  // namespace nu
