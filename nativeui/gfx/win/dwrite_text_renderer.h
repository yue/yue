// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_WIN_DWRITE_TEXT_RENDERER_H_
#define NATIVEUI_GFX_WIN_DWRITE_TEXT_RENDERER_H_

#include <d2d1.h>
#include <dwrite.h>
#include <wrl/client.h>

#include <map>

#include "base/macros.h"

namespace nu {

// Implementation of dwrite renderer.
class DWriteTextRenderer : public IDWriteTextRenderer {
 public:
  DWriteTextRenderer(ID2D1DCRenderTarget* target, float scale_factor);
  ~DWriteTextRenderer();

  // IDWriteTextRenderer:
  HRESULT __stdcall DrawGlyphRun(
      void* clientDrawingContext,
      FLOAT baselineOriginX,
      FLOAT baselineOriginY,
      DWRITE_MEASURING_MODE measuringMode,
      const DWRITE_GLYPH_RUN* glyphRun,
      const DWRITE_GLYPH_RUN_DESCRIPTION* glyphRunDescription,
      IUnknown* clientDrawingEffect) override;
  HRESULT __stdcall DrawUnderline(
      void* clientDrawingContext,
      FLOAT baselineOriginX,
      FLOAT baselineOriginY,
      const DWRITE_UNDERLINE* underline,
      IUnknown* clientDrawingEffect) override;
  HRESULT __stdcall DrawStrikethrough(
      void* clientDrawingContext,
      FLOAT baselineOriginX,
      FLOAT baselineOriginY,
      const DWRITE_STRIKETHROUGH* strikethrough,
      IUnknown* clientDrawingEffect) override;
  HRESULT __stdcall DrawInlineObject(
      void* clientDrawingContext,
      FLOAT originX,
      FLOAT originY,
      IDWriteInlineObject* inlineObject,
      BOOL isSideways,
      BOOL isRightToLeft,
      IUnknown* clientDrawingEffect) override;

  // IDWritePixelSnapping:
  HRESULT __stdcall IsPixelSnappingDisabled(
      void* clientDrawingContext,
      BOOL* isDisabled) override;
  HRESULT __stdcall GetCurrentTransform(
      void* clientDrawingContext,
      DWRITE_MATRIX* transform) override;
  HRESULT __stdcall GetPixelsPerDip(
      void* clientDrawingContext,
      FLOAT* pixelsPerDip) override;

  // IUnknown:
  HRESULT __stdcall QueryInterface(
      const IID& riid,
      void** ppvObject) override;
  ULONG __stdcall AddRef() override;
  ULONG __stdcall Release() override;

 private:
  ID2D1DCRenderTarget* target_;  // weak ref
  float scale_factor_;

  // Cached brushes for colors.
  std::map<uint32_t, Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>> brushes_;

  // Cached brush for default text.
  Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> default_brush_;

  LONG ref_ = 0;

  DISALLOW_COPY_AND_ASSIGN(DWriteTextRenderer);
};

}  // namespace nu

#endif  // NATIVEUI_GFX_WIN_DWRITE_TEXT_RENDERER_H_
