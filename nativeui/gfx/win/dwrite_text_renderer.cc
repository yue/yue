// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/win/dwrite_text_renderer.h"

#include <shlwapi.h>

#include "base/logging.h"
#include "nativeui/gfx/win/drawing_effect.h"
#include "nativeui/system.h"

namespace nu {

DWriteTextRenderer::DWriteTextRenderer(ID2D1DCRenderTarget* target,
                                       float scale_factor)
    : target_(target), scale_factor_(scale_factor) {}

DWriteTextRenderer::~DWriteTextRenderer() {}

HRESULT __stdcall DWriteTextRenderer::DrawGlyphRun(
    void* clientDrawingContext,
    FLOAT baselineOriginX,
    FLOAT baselineOriginY,
    DWRITE_MEASURING_MODE measuringMode,
    const DWRITE_GLYPH_RUN* glyphRun,
    const DWRITE_GLYPH_RUN_DESCRIPTION* glyphRunDescription,
    IUnknown* clientDrawingEffect) {
  ID2D1SolidColorBrush* brush = nullptr;

  // Read the passed color.
  if (clientDrawingEffect) {
    auto* drawing_effect = static_cast<DrawingEffect*>(clientDrawingEffect);
    // Create brush on request.
    auto it = brushes_.find(drawing_effect->fg_color.value());
    if (it == brushes_.end()) {
      Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> brush;
      target_->CreateSolidColorBrush(
          drawing_effect->fg_color.ToD2D1Color(), brush.GetAddressOf());
      it = brushes_.emplace(drawing_effect->fg_color.value(), brush).first;
    }
    brush = it->second.Get();
  }

  // Use system text color as default brush.
  if (!brush) {
    if (!default_brush_)
      target_->CreateSolidColorBrush(
          System::GetColor(System::Color::Text).ToD2D1Color(),
          default_brush_.GetAddressOf());
    brush = default_brush_.Get();
  }

  target_->DrawGlyphRun({baselineOriginX, baselineOriginY},
                        glyphRun, brush, measuringMode);
  return S_OK;
}

HRESULT __stdcall DWriteTextRenderer::DrawUnderline(
    void* clientDrawingContext,
    FLOAT baselineOriginX,
    FLOAT baselineOriginY,
    const DWRITE_UNDERLINE* underline,
    IUnknown* clientDrawingEffect) {
  return E_NOTIMPL;
}

HRESULT __stdcall DWriteTextRenderer::DrawStrikethrough(
    void* clientDrawingContext,
    FLOAT baselineOriginX,
    FLOAT baselineOriginY,
    const DWRITE_STRIKETHROUGH* strikethrough,
    IUnknown* clientDrawingEffect) {
  return E_NOTIMPL;
}

HRESULT __stdcall DWriteTextRenderer::DrawInlineObject(
    void* clientDrawingContext,
    FLOAT originX,
    FLOAT originY,
    IDWriteInlineObject* inlineObject,
    BOOL isSideways,
    BOOL isRightToLeft,
    IUnknown* clientDrawingEffect) {
  if (inlineObject == NULL)
    return E_POINTER;
  return inlineObject->Draw(clientDrawingContext, this,
                            originX, originY,
                            isSideways, isRightToLeft,
                            clientDrawingEffect);
}

HRESULT __stdcall DWriteTextRenderer::IsPixelSnappingDisabled(
    void* clientDrawingContext,
    BOOL* isDisabled) {
  *isDisabled = FALSE;
  return S_OK;
}

HRESULT __stdcall DWriteTextRenderer::GetCurrentTransform(
    void* clientDrawingContext,
    DWRITE_MATRIX* transform) {
  target_->GetTransform(reinterpret_cast<D2D1_MATRIX_3X2_F*>(transform));
  return S_OK;
}

HRESULT __stdcall DWriteTextRenderer::GetPixelsPerDip(
    void* clientDrawingContext,
    FLOAT* pixelsPerDip) {
  *pixelsPerDip = scale_factor_;
  return S_OK;
}

HRESULT __stdcall DWriteTextRenderer::QueryInterface(
    const IID& riid,
    void** ppvObject) {
  const QITAB QITable[] = {
    QITABENT(DWriteTextRenderer, IDWriteTextRenderer),
    QITABENT(DWriteTextRenderer, IDWritePixelSnapping),
    {0},
  };
  return QISearch(this, QITable, riid, ppvObject);
}

ULONG __stdcall DWriteTextRenderer::AddRef() {
  return InterlockedIncrement(&ref_);
}

ULONG __stdcall DWriteTextRenderer::Release() {
  auto cref = InterlockedDecrement(&ref_);
  if (cref == 0)
    delete this;
  return cref;
}

}  // namespace nu
