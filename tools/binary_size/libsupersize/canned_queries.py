# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Contains a set of Chrome-specific size queries."""

import logging
import models


class _Grouper(object):
  def __init__(self):
    self.groups = []

  def Add(self, name, group):
    logging.debug('Computed %s', name)
    sorted_group = group.Sorted()
    sorted_group.SetName(name)
    self.groups.append(sorted_group)
    return group.Inverted()

  def Finalize(self, remaining):
    self.groups.sort(key=lambda s:(s.name.startswith('Other'), -s.pss))
    if remaining:
      stars = remaining.Filter(lambda s: s.name.startswith('*'))
      if stars:
        remaining = stars.Inverted()
        stars = stars.Sorted()
        stars.SetName('** Merged Symbols')
        self.groups.append(stars)
      remaining.SetName('Other')
      self.groups.append(remaining)

    logging.debug('Finalized')
    return models.SymbolGroup(self.groups, is_sorted=True)


def _CategorizeByChromeComponent(symbols):
  g = _Grouper()

  # Put things that filter out a lot of symbols at the beginning where possible
  # to optimize speed.
  symbols = g.Add('WebRTC', symbols.WhereMatches(r'(?i)webrtc'))
  symbols = g.Add('Skia', symbols.Filter(lambda s: 'skia/' in s.source_path))
  symbols = g.Add('V8', symbols.Filter(
      lambda s: s.source_path.startswith('v8/')))

  # Next, put non-regex queries, since they're a bit faster.
  symbols = g.Add('ICU', symbols.Filter(lambda s: '/icu/' in s.source_path))
  symbols = g.Add('Prefetch', symbols.Filter(
      lambda s: 'resource_prefetch' in s.source_path))
  symbols = g.Add('Password Manager', symbols.Filter(
      lambda s: 'password_manager' in s.source_path))
  symbols = g.Add('Internals Pages', symbols.Filter(
      lambda s: '_internals' in s.source_path))
  symbols = g.Add('Autofill', symbols.WhereSourcePathMatches(r'(?i)autofill'))
  symbols = g.Add('WebGL', symbols.WhereMatches(r'(?i)webgl'))
  symbols = g.Add('WebBluetooth', symbols.WhereMatches(r'(?i)bluetooth'))
  symbols = g.Add('WebUSB', symbols.WhereMatches(r'(?i)webusb|(\b|_)usb(\b|_)'))
  symbols = g.Add('WebVR', symbols.WhereMatches(
      r'{{_gvr_}}|{{_cwebvr_}}|{{_vr_}}'))
  symbols = g.Add('FileSystem', symbols.WhereSourcePathMatches(
      r'content/.*/fileapi|WebKit/.*/filesystem'))
  symbols = g.Add('WebCrypto', symbols.WhereMatches(r'(?i)webcrypto'))
  symbols = g.Add('Printing', symbols.WhereMatches(r'printing'))
  symbols = g.Add('Cast', symbols.WhereSourcePathMatches(
      r'(?i)(\b|_)cast(\b|_)'))
  symbols = g.Add('Media Source', symbols.WhereMatches(
      r'(?i)mediasource|blink::.*TrackDefault|blink::.*SourceBuffer'))

  # XSLT must come before libxml.
  symbols = g.Add('XSLT', symbols.WhereMatches(r'(?i)xslt'))
  symbols = g.Add('libxml', symbols.Filter(
      lambda s: 'libxml' in s.source_path))

  # These have some overlap with above, so need to come afterwards.
  blink_syms = symbols.WhereSourcePathMatches(r'\b(blink|WebKit)\b')
  symbols = blink_syms.Inverted()
  blink_generated = blink_syms.WhereSourceIsGenerated()
  g.Add('Blink (generated)', blink_generated)
  g.Add('Blink (non-generated)', blink_generated.Inverted())

  symbols = g.Add('Codecs', symbols.WhereSourcePathMatches(
      r'^third_party/(libweb[mp]|libpng|libjpeg_turbo|opus|ffmpeg|libvpx)/'))
  symbols = g.Add('Other Third-Party', symbols.Filter(
      lambda s: 'third_party' in s.source_path))

  return g.Finalize(symbols)


def _CategorizeGenerated(symbols):
  g = _Grouper()

  # JNI is generated into .h files then #included, so the symbols don't actaully
  # appear as "SourceIsGenerated".
  # Note: String literals within symbols like "kBaseRegisteredMethods" are not
  #     being accounted for here because they end up within "** merge strings".
  #     This could be fixed by assigning them all to proper variables rather
  #     than having them be inline.
  symbols = g.Add('RegisterJNI', symbols.WhereFullNameMatches(
      r'Register.*JNIEnv\*\)|RegisteredMethods$'))

  symbols = symbols.WhereSourceIsGenerated()
  symbols = g.Add('Protocol Buffers', symbols.Filter(lambda s: (
      '/protobuf/' in s.object_path or
      s.object_path.endswith('.pbzero.o') or
      s.object_path.endswith('.pb.o'))))
  symbols = g.Add('Mojo', symbols.Filter(lambda s: (
      '.mojom' in s.source_path or  # Blink uses .mojom-blink.cc
      s.source_path.startswith('mojo/') or
      s.name.startswith('mojo::'))))
  symbols = g.Add('DevTools', symbols.WhereSourcePathMatches(
      r'\b(?:protocol|devtools)\b'))
  symbols = g.Add('Blink (bindings)', symbols.Filter(lambda s: (
      'blink/bindings' in s.source_path or
      ('WebKit/' in s.object_path and '/bindings/' in s.object_path))))
  symbols = g.Add('Blink (IDL)', symbols.Filter(lambda s: (
      'WebKit/Source/core' in s.object_path)))
  symbols = g.Add('Blink (Other)', symbols.Filter(lambda s: (
      'WebKit' in s.object_path or 'blink/' in s.object_path)))

  return g.Finalize(symbols)


class CannedQueries(object):
  """A set of pre-written queries."""

  def __init__(self, size_infos):
    self._size_infos = size_infos

  def _SymbolsArg(self, arg):
    arg = arg if arg is not None else self._size_infos[-1]
    if isinstance(arg, models.SizeInfo):
      arg = arg.symbols
    return arg

  def CategorizeGenerated(self, symbols=None):
    """Categorizes symbols that come from generated source files."""
    return _CategorizeGenerated(self._SymbolsArg(symbols))

  def CategorizeByChromeComponent(self, symbols=None):
    """Groups symbols by component using predefined queries."""
    return _CategorizeByChromeComponent(self._SymbolsArg(symbols))

  def TemplatesByName(self, symbols=None, depth=0):
    """Lists C++ templates grouped by name."""
    symbols = self._SymbolsArg(symbols)
    # Call Sorted() twice so that subgroups will be sorted.
    # TODO(agrieve): Might be nice to recursively GroupedByName() on these.
    return symbols.WhereIsTemplate().Sorted().GroupedByName(depth).Sorted()
