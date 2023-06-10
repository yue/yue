// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/image.h"

#include <shlwapi.h>
#include <wrl.h>

#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "base/win/scoped_hglobal.h"
#include "nativeui/gfx/canvas.h"
#include "nativeui/gfx/painter.h"
#include "nativeui/gfx/win/double_buffer.h"
#include "nativeui/gfx/win/gdiplus.h"

namespace nu {

namespace {

bool GetEncoderClsid(base::WStringPiece format, CLSID* clsid) {
  UINT num = 0, size = 0;
  if (Gdiplus::GetImageEncodersSize(&num, &size) != Gdiplus::Ok)
    return false;
  std::vector<Gdiplus::ImageCodecInfo> codecs(size);
  if (Gdiplus::GetImageEncoders(num, size, codecs.data()) != Gdiplus::Ok)
    return false;
  for (const auto& codec : codecs) {
    if (format == codec.MimeType) {
      *clsid = codec.Clsid;
      return true;
    }
  }
  return false;
}

Buffer EncodeImage(const Gdiplus::Image* image,
                   base::WStringPiece mime_type,
                   Gdiplus::EncoderParameters* params = nullptr) {
  // Create an IStream object in memory.
  Microsoft::WRL::ComPtr<IStream> stream;
  ::CreateStreamOnHGlobal(nullptr, TRUE, &stream);
  // Save the image to the stream as mime_type.
  CLSID encoder;
  if (!GetEncoderClsid(mime_type, &encoder))
    return Buffer();
  const_cast<Gdiplus::Image*>(image)->Save(stream.Get(), &encoder, params);
  // Read the buffer from stream.
  HGLOBAL hdata = nullptr;
  if (FAILED(::GetHGlobalFromStream(stream.Get(), &hdata)))
    return Buffer();
  base::win::ScopedHGlobal<void*> locked_data(hdata);
  size_t size = locked_data.Size();
  void* memory = new char[size];
  memcpy(memory, locked_data.release(), size);
  return Buffer::TakeOver(memory, size,
                          [](void* data) { delete static_cast<char*>(data); });
}

}  // namespace

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

Image* Image::Tint(Color color) const {
  Gdiplus::Image* image = const_cast<Gdiplus::Image*>(image_);
  // Create a bitmap to draw on.
  float width = image->GetWidth();
  float height = image->GetHeight();
  std::unique_ptr<Gdiplus::Bitmap> bitmap(
      new Gdiplus::Bitmap(width, height, PixelFormat32bppARGB));
  Gdiplus::Graphics graphics(bitmap.get());
  // Set color matrix for tint.
  Gdiplus::ColorMatrix matrix = {
    color.r() / 255., 0, 0, 0, 0,
    0, color.g() / 255., 0, 0, 0,
    0, 0, color.b() / 255., 0, 0,
    0, 0, 0, color.a() / 255., 0,
    // FIXME(zcbenz): Other platforms use BlenMode::SourceAtop to apply the
    // tint color, which is not possible with GDI+. Using 0.5 is a very poor
    // way to achive the mixed colors.
    0.5, 0.5, 0.5, 0, 1,
  };
  Gdiplus::ImageAttributes attributes;
  attributes.SetColorMatrix(&matrix);
  // Draw tinted image.
  graphics.DrawImage(image_,
                     Gdiplus::RectF(0, 0, width, height),
                     Gdiplus::RectF(0, 0, width, height),
                     Gdiplus::UnitPixel,
                     &attributes);
  // Create new image.
  return new Image(bitmap.release(), scale_factor_);
}

Image* Image::Resize(SizeF new_size, float scale_factor) const {
  // Create a bitmap to draw on.
  SizeF scaled_size = ScaleSize(new_size, scale_factor);
  std::unique_ptr<Gdiplus::Bitmap> bitmap(
      new Gdiplus::Bitmap(scaled_size.width(), scaled_size.height(),
                          PixelFormat32bppARGB));
  // Draw on it.
  Gdiplus::Graphics graphics(bitmap.get());
  graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
  graphics.DrawImage(image_, ToGdi(RectF(scaled_size)));
  // Create new image.
  return new Image(bitmap.release(), scale_factor);
}

Buffer Image::ToPNG() const {
  return EncodeImage(image_, L"image/png");
}

Buffer Image::ToJPEG(int quality) const {
  Gdiplus::EncoderParameters params;
  params.Count = 1;
  params.Parameter[0].Guid = Gdiplus::EncoderQuality;
  params.Parameter[0].Type = Gdiplus::EncoderParameterValueTypeLong;
  params.Parameter[0].NumberOfValues = 1;
  params.Parameter[0].Value = &quality;
  return EncodeImage(image_, L"image/jpeg", &params);
}

bool Image::WriteToFile(const std::string& format,
                        const base::FilePath& target) {
  CLSID encoder;
  if (!GetEncoderClsid(base::UTF8ToWide("image/" + format), &encoder)) {
    LOG(ERROR) << "Unable to find encoder for " << format;
    return false;
  }
  Gdiplus::Image* image = const_cast<Gdiplus::Image*>(image_);
  return image->Save(target.value().c_str(), &encoder, nullptr) == Gdiplus::Ok;
}

base::win::ScopedHICON Image::GetHICON(const SizeF& size) const {
  scoped_refptr<Canvas> canvas = new Canvas(size);
  canvas->GetPainter()->DrawImage(this, RectF(size));
  base::win::ScopedHICON result;
  if (canvas->GetBitmap()->GetGdiplusBitmap()->GetHICON(
          base::win::ScopedHICON::Receiver(result).get()) != Gdiplus::Ok)
    LOG(WARNING) << "Error converting image to HICON";
  return result;
}

}  // namespace nu
