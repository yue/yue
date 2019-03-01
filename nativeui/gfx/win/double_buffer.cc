// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/win/double_buffer.h"

namespace nu {

namespace {

HBITMAP CreateBitmap(HDC dc, const Size& size) {
  BITMAPINFOHEADER bih = { 0 };
  bih.biBitCount = 32;
  bih.biSize = sizeof(BITMAPINFOHEADER);
  bih.biWidth = size.width();
  bih.biHeight = size.height();
  bih.biPlanes = 1;
  bih.biSizeImage = size.width() * size.height() * 4;
  bih.biCompression = BI_RGB;
  RGBQUAD* bits = nullptr;
  return ::CreateDIBSection(dc, reinterpret_cast<BITMAPINFO*>(&bih), 0,
                            reinterpret_cast<void**>(&bits), NULL, 0);
}

}  // namespace

DoubleBuffer::DoubleBuffer(HWND hwnd, const Size& size)
    : DoubleBuffer(::GetDC(hwnd), size, Rect(), Point()) {
  copy_on_destruction_ = false;
  ::ReleaseDC(hwnd, dc_);
}

DoubleBuffer::DoubleBuffer(HDC dc, const Size& size, const Rect& src,
                           const Point& dest)
    : dc_(dc), src_(src), dest_(dest),
      mem_dc_(::CreateCompatibleDC(dc)),
      mem_bitmap_(CreateBitmap(dc, size)),
      select_bitmap_(mem_dc_.Get(), mem_bitmap_.get()) {}

DoubleBuffer::~DoubleBuffer() {
  if (copy_on_destruction_) {
    // Transfer the off-screen DC to the screen.
    BitBlt(dc_, src_.x(), src_.y(), src_.width(), src_.height(),
           dc(), dest_.x(), dest_.y(), SRCCOPY);
  }
}

std::unique_ptr<Gdiplus::Bitmap> DoubleBuffer::GetGdiplusBitmap() const {
  // Code from Microsoft/VSSDK-Extensibility-Samples:
  // ArchivedSamples/High-DPI_Images_Icons/Cpp/VsUIGdiplusImage.cpp
  //
  // Copyright (c) Microsoft. All rights reserved.
  // Licensed under the MIT license.
  DIBSECTION dib = {0};
  if (::GetObject(mem_bitmap_.get(), sizeof(dib), &dib) != sizeof(DIBSECTION) ||
      dib.dsBm.bmBitsPixel != 32) {
    // Fall back to Gdiplus conversion.
    return std::unique_ptr<Gdiplus::Bitmap>(
        Gdiplus::Bitmap::FromHBITMAP(mem_bitmap_.get(), NULL));
  }

  // If we have a 32bpp DIB created by calling CreateDIBSection, assume that
  // it's in ARGB format.
  // This is the preferred format for full per-pixel alpha support.
  int width = dib.dsBmih.biWidth;
  int height = dib.dsBmih.biHeight;
  int pitch = dib.dsBm.bmWidthBytes;
  BYTE* bits = static_cast<BYTE*>(dib.dsBm.bmBits);

  if (height < 0) {  // top-down
    height = -height;
  } else {  // bottom-up
    // Adjust the Scan0 to the start of the last row.
    bits += (height - 1) * pitch;
    // and set the pitch to a -ve value.
    pitch = -pitch;
  }

  return std::make_unique<Gdiplus::Bitmap>(
      width, height, pitch, PixelFormat32bppARGB, bits);
}

}  // namespace nu
