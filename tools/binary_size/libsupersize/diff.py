# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Logic for diffing two SizeInfo objects."""

import collections
import re

import models


def _SymbolKey(symbol):
  """Returns a tuple that can be used to see if two Symbol are the same.

  Keys are not guaranteed to be unique within a SymbolGroup. When multiple
  symbols have the same key, they will be matched up in order of appearance.
  We do this because the numbering of these generated symbols is not stable.

  Examples of symbols with shared keys:
    "** merge strings"
    "** symbol gap 3", "** symbol gap 5"
    "foo() [clone ##]"
    "CSWTCH.61", "CSWTCH.62"
    "._468", "._467"
    ".L__unnamed_1193", ".L__unnamed_712"
  """
  name = symbol.full_name
  clone_idx = name.find(' [clone ')
  if clone_idx != -1:
    name = name[:clone_idx]
  if name.startswith('*'):
    # "symbol gap 3 (bar)" -> "symbol gaps"
    name = re.sub(r'\s+\d+( \(.*\))?$', 's', name)

  # Use section rather than section_name since clang & gcc use
  # .data.rel.ro vs .data.rel.ro.local.
  if '.' not in name:
    return (symbol.section, name)

  # Compiler or Linker generated symbol.
  name = re.sub(r'[.0-9]', '', name)  # Strip out all numbers and dots.
  return (symbol.section, name, symbol.object_path)


def _DiffSymbolGroups(before, after):
  before_symbols_by_key = collections.defaultdict(list)
  for s in before:
    before_symbols_by_key[_SymbolKey(s)].append(s)

  delta_symbols = []
  # For changed symbols, padding is zeroed out. In order to not lose the
  # information entirely, store it in aggregate.
  padding_by_section_name = collections.defaultdict(int)

  # Create a DeltaSymbol for each after symbol.
  for after_sym in after:
    matching_syms = before_symbols_by_key.get(_SymbolKey(after_sym))
    before_sym = None
    if matching_syms:
      before_sym = matching_syms.pop(0)
      # Padding tracked in aggregate, except for padding-only symbols.
      if before_sym.size_without_padding:
        padding_by_section_name[before_sym.section_name] += (
            after_sym.padding_pss - before_sym.padding_pss)
    delta_symbols.append(models.DeltaSymbol(before_sym, after_sym))

  # Create a DeltaSymbol for each unmatched before symbol.
  for remaining_syms in before_symbols_by_key.itervalues():
    for before_sym in remaining_syms:
      delta_symbols.append(models.DeltaSymbol(before_sym, None))

  # Create a DeltaSymbol to represent the zero'd out padding of matched symbols.
  for section_name, padding in padding_by_section_name.iteritems():
    if padding != 0:
      after_sym = models.Symbol(section_name, padding,
                                name="** aggregate padding of diff'ed symbols")
      after_sym.padding = padding
      delta_symbols.append(models.DeltaSymbol(None, after_sym))

  return models.DeltaSymbolGroup(delta_symbols)


def Diff(before, after):
  """Diffs two SizeInfo objects. Returns a DeltaSizeInfo."""
  assert isinstance(before, models.SizeInfo)
  assert isinstance(after, models.SizeInfo)
  section_sizes = {k: after.section_sizes.get(k, 0) - v
                   for k, v in before.section_sizes.iteritems()}
  for k, v in after.section_sizes.iteritems():
    if k not in section_sizes:
      section_sizes[k] = v

  symbol_diff = _DiffSymbolGroups(before.raw_symbols, after.raw_symbols)
  return models.DeltaSizeInfo(section_sizes, symbol_diff, before.metadata,
                              after.metadata)
