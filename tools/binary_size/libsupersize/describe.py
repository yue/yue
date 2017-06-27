# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Methods for converting model objects to human-readable formats."""

import datetime
import itertools
import time

import models


def _PrettySize(size):
  # Arbitrarily chosen cut-off.
  if abs(size) < 2000:
    return '%d bytes' % size
  # Always show 3 digits.
  size /= 1024.0
  if abs(size) < 10:
    return '%.2fkb' % size
  elif abs(size) < 100:
    return '%.1fkb' % size
  elif abs(size) < 1024:
    return '%dkb' % size
  size /= 1024.0
  if abs(size) < 10:
    return '%.2fmb' % size
  # We shouldn't be seeing sizes > 100mb.
  return '%.1fmb' % size


def _FormatPss(pss):
  # Shows a decimal for small numbers to make it clear that a shared symbol has
  # a non-zero pss.
  if abs(pss) > 10:
    return str(int(pss))
  ret = str(round(pss, 1))
  if ret.endswith('.0'):
    ret = ret[:-2]
    if ret == '0' and pss:
      ret = '~0'
  return ret


def _Divide(a, b):
  return float(a) / b if b else 0


class Describer(object):
  def __init__(self, verbose=False, recursive=False):
    self.verbose = verbose
    self.recursive = recursive

  def _DescribeSectionSizes(self, section_sizes):
    def include_in_totals(name):
      return name != '.bss' and '(' not in name

    total_bytes = sum(v for k, v in section_sizes.iteritems()
                      if include_in_totals(k))
    max_bytes = max(abs(v) for k, v in section_sizes.iteritems()
                    if include_in_totals(k))

    def is_relevant_section(name, size):
      # Show all sections containing symbols, plus relocations.
      # As a catch-all, also include any section that comprises > 4% of the
      # largest section. Use largest section rather than total so that it still
      # works out when showing a diff containing +100, -100 (total=0).
      return (name in models.SECTION_TO_SECTION_NAME.values() or
              name in ('.rela.dyn', '.rel.dyn') or
              include_in_totals(name) and abs(_Divide(size, max_bytes)) > .04)

    section_names = sorted(k for k, v  in section_sizes.iteritems()
                           if is_relevant_section(k, v))
    yield ''
    yield 'Section Sizes (Total={} ({} bytes)):'.format(
        _PrettySize(total_bytes), total_bytes)
    for name in section_names:
      size = section_sizes[name]
      if not include_in_totals(name):
        yield '    {}: {} ({} bytes) (not included in totals)'.format(
            name, _PrettySize(size), size)
      else:
        percent = _Divide(size, total_bytes)
        yield '    {}: {} ({} bytes) ({:.1%})'.format(
            name, _PrettySize(size), size, percent)

    if self.verbose:
      yield ''
      yield 'Other section sizes:'
      section_names = sorted(k for k in section_sizes.iterkeys()
                             if k not in section_names)
      for name in section_names:
        not_included_part = ''
        if not include_in_totals(name):
          not_included_part = ' (not included in totals)'
        yield '    {}: {} ({} bytes){}'.format(
            name, _PrettySize(section_sizes[name]), section_sizes[name],
            not_included_part)

  def _DescribeSymbol(self, sym, single_line=False):
    if sym.IsGroup():
      address = 'Group'
    else:
      address = hex(sym.address)
    last_field = ''
    if sym.IsGroup():
      last_field = 'count=%d' % len(sym)
    elif sym.IsDelta():
      if sym.before_symbol is None:
        num_aliases = sym.after_symbol.num_aliases
      elif sym.after_symbol is None:
        num_aliases = sym.before_symbol.num_aliases
      elif sym.before_symbol.num_aliases == sym.after_symbol.num_aliases:
        num_aliases = sym.before_symbol.num_aliases
      else:
        last_field = 'num_aliases=%d->%d' % (
            sym.before_symbol.num_aliases, sym.after_symbol.num_aliases)
      if not last_field and (num_aliases > 1 or self.verbose):
        last_field = 'num_aliases=%d' % num_aliases
    elif sym.num_aliases > 1 or self.verbose:
      last_field = 'num_aliases=%d' % sym.num_aliases

    if sym.IsDelta():
      if sym.IsGroup():
        b = sum(s.before_symbol.pss_without_padding if s.before_symbol else 0
                for s in sym.IterLeafSymbols())
        a = sum(s.after_symbol.pss_without_padding if s.after_symbol else 0
                for s in sym.IterLeafSymbols())
      else:
        b = sym.before_symbol.pss_without_padding if sym.before_symbol else 0
        a = sym.after_symbol.pss_without_padding if sym.after_symbol else 0
      pss_with_sign = _FormatPss(sym.pss)
      if pss_with_sign[0] not in '~-':
        pss_with_sign = '+' + pss_with_sign
      pss_field = '{} ({}->{})'.format(
          pss_with_sign, _FormatPss(b), _FormatPss(a))
    elif sym.num_aliases > 1:
      pss_field = '{} (size={})'.format(_FormatPss(sym.pss), sym.size)
    else:
      pss_field = '{}'.format(_FormatPss(sym.pss))

    if self.verbose:
      if last_field:
        last_field = '  ' + last_field
      if sym.IsDelta():
        yield '{}@{:<9s}  {}{}'.format(
            sym.section, address, pss_field, last_field)
      else:
        l = '{}@{:<9s}  pss={}  padding={}{}'.format(
            sym.section, address, pss_field, sym.padding, last_field)
        yield l
      yield '    source_path={} \tobject_path={}'.format(
          sym.source_path, sym.object_path)
      if sym.name:
        yield '    flags={}  name={}'.format(sym.FlagsString(), sym.name)
        if sym.full_name is not sym.name:
          yield '         full_name={}'.format(sym.full_name)
      elif sym.full_name:
        yield '    flags={}  full_name={}'.format(
            sym.FlagsString(), sym.full_name)
    else:
      if last_field:
        last_field = ' ({})'.format(last_field)
      if sym.IsDelta():
        pss_field = '{:<18}'.format(pss_field)
      else:
        pss_field = '{:<14}'.format(pss_field)
      if single_line:
        yield '{}@{:<9s}  {}  {}{}'.format(
            sym.section, address, pss_field, sym.name, last_field)
      else:
        yield '{}@{:<9s}  {} {}'.format(
            sym.section, address, pss_field,
            sym.source_path or sym.object_path or '{no path}')
        if sym.name:
          yield '    {}{}'.format(sym.name, last_field)

  def _DescribeSymbolGroupChildren(self, group, indent=0):
    running_total = 0
    running_percent = 0
    is_delta = group.IsDelta()
    all_groups = all(s.IsGroup() for s in group)

    indent_prefix = '> ' * indent
    diff_prefix = ''
    total = group.pss
    for index, s in enumerate(group):
      if group.IsBss() or not s.IsBss():
        running_total += s.pss
        running_percent = _Divide(running_total, total)
      for l in self._DescribeSymbol(s, single_line=all_groups):
        if l[:4].isspace():
          indent_size = 8 + len(indent_prefix) + len(diff_prefix)
          yield '{} {}'.format(' ' * indent_size, l)
        else:
          if is_delta:
            diff_prefix = models.DIFF_PREFIX_BY_STATUS[s.diff_status]
          yield '{}{}{:<4} {:>8} {:7} {}'.format(
              indent_prefix, diff_prefix, str(index) + ')',
              _FormatPss(running_total), '({:.1%})'.format(running_percent), l)

      if self.recursive and s.IsGroup():
        for l in self._DescribeSymbolGroupChildren(s, indent=indent + 1):
          yield l

  def _DescribeSymbolGroup(self, group):
    total_size = group.pss
    code_size = 0
    ro_size = 0
    data_size = 0
    bss_size = 0
    unique_paths = set()
    for s in group.IterLeafSymbols():
      if s.section == 't':
        code_size += s.pss
      elif s.section == 'r':
        ro_size += s.pss
      elif s.section == 'd':
        data_size += s.pss
      elif s.section == 'b':
        bss_size += s.pss
      # Ignore paths like foo/{shared}/2
      if '{' not in s.object_path:
        unique_paths.add(s.object_path)

    if group.IsDelta():
      unique_part = 'aliases not grouped for diffs'
    else:
      unique_part = '{:,} unique'.format(group.CountUniqueSymbols())

    if self.verbose:
      titles = 'Index | Running Total | Section@Address | ...'
    elif group.IsDelta():
      titles = (u'Index | Running Total | Section@Address | \u0394 PSS '
                u'(\u0394 size_without_padding) | Path').encode('utf-8')
    else:
      titles = ('Index | Running Total | Section@Address | PSS | Path')

    header_desc = [
        'Showing {:,} symbols ({}) with total pss: {} bytes'.format(
            len(group), unique_part, int(total_size)),
        '.text={:<10} .rodata={:<10} .data*={:<10} .bss={:<10} total={}'.format(
            _PrettySize(int(code_size)), _PrettySize(int(ro_size)),
            _PrettySize(int(data_size)), _PrettySize(int(bss_size)),
            _PrettySize(int(total_size))),
        'Number of unique paths: {}'.format(len(unique_paths)),
        '',
        titles,
        '-' * 60
    ]
    # Apply this filter after calcualating stats since an alias being removed
    # causes some symbols to be UNCHANGED, yet have pss != 0.
    if group.IsDelta() and not self.verbose:
      group = group.WhereDiffStatusIs(models.DIFF_STATUS_UNCHANGED).Inverted()
    children_desc = self._DescribeSymbolGroupChildren(group)
    return itertools.chain(header_desc, children_desc)

  def _DescribeDiffObjectPaths(self, delta_group):
    paths_by_status = [set(), set(), set(), set()]
    for s in delta_group.IterLeafSymbols():
      path = s.source_path or s.object_path
      # Ignore paths like foo/{shared}/2
      if '{' not in path:
        paths_by_status[s.diff_status].add(path)
    # Initial paths sets are those where *any* symbol is
    # unchanged/changed/added/removed.
    unchanged, changed, added, removed = paths_by_status
    # Consider a path with both adds & removes as "changed".
    changed.update(added.intersection(removed))
    # Consider a path added / removed only when all symbols are new/removed.
    added.difference_update(unchanged)
    added.difference_update(changed)
    added.difference_update(removed)
    removed.difference_update(unchanged)
    removed.difference_update(changed)
    removed.difference_update(added)
    yield '{} paths added, {} removed, {} changed'.format(
        len(added), len(removed), len(changed))

    if self.verbose and len(added):
      yield 'Added files:'
      for p in sorted(added):
        yield '  ' + p
    if self.verbose and len(removed):
      yield 'Removed files:'
      for p in sorted(removed):
        yield '  ' + p
    if self.verbose and len(changed):
      yield 'Changed files:'
      for p in sorted(changed):
        yield '  ' + p

  def _DescribeDeltaSymbolGroup(self, delta_group):
    header_template = ('{} symbols added (+), {} changed (~), {} removed (-), '
                       '{} unchanged ({})')
    unchanged_msg = '=' if self.verbose else 'not shown'
    counts = delta_group.CountsByDiffStatus()
    num_unique_before_symbols, num_unique_after_symbols = (
        delta_group.CountUniqueSymbols())
    diff_summary_desc = [
        header_template.format(
            counts[models.DIFF_STATUS_ADDED],
            counts[models.DIFF_STATUS_CHANGED],
            counts[models.DIFF_STATUS_REMOVED],
            counts[models.DIFF_STATUS_UNCHANGED],
            unchanged_msg),
        'Number of unique symbols {} -> {} ({:+})'.format(
            num_unique_before_symbols, num_unique_after_symbols,
            num_unique_after_symbols - num_unique_before_symbols),
        ]
    path_delta_desc = self._DescribeDiffObjectPaths(delta_group)

    group_desc = self._DescribeSymbolGroup(delta_group)
    return itertools.chain(diff_summary_desc, path_delta_desc, ('',),
                           group_desc)

  def _DescribeDeltaSizeInfo(self, diff):
    common_metadata = {k: v for k, v in diff.before_metadata.iteritems()
                       if diff.after_metadata[k] == v}
    before_metadata = {k: v for k, v in diff.before_metadata.iteritems()
                       if k not in common_metadata}
    after_metadata = {k: v for k, v in diff.after_metadata.iteritems()
                      if k not in common_metadata}
    metadata_desc = itertools.chain(
        ('Common Metadata:',),
        ('    %s' % line for line in DescribeMetadata(common_metadata)),
        ('Old Metadata:',),
        ('    %s' % line for line in DescribeMetadata(before_metadata)),
        ('New Metadata:',),
        ('    %s' % line for line in DescribeMetadata(after_metadata)))
    section_desc = self._DescribeSectionSizes(diff.section_sizes)
    group_desc = self.GenerateLines(diff.symbols)
    return itertools.chain(metadata_desc, section_desc, ('',), group_desc)

  def _DescribeSizeInfo(self, size_info):
    metadata_desc = itertools.chain(
        ('Metadata:',),
        ('    %s' % line for line in DescribeMetadata(size_info.metadata)))
    section_desc = self._DescribeSectionSizes(size_info.section_sizes)
    coverage_desc = ()
    if self.verbose:
      coverage_desc = itertools.chain(
          ('',), DescribeSizeInfoCoverage(size_info))
    group_desc = self.GenerateLines(size_info.symbols)
    return itertools.chain(metadata_desc, section_desc, coverage_desc, ('',),
                           group_desc)

  def GenerateLines(self, obj):
    if isinstance(obj, models.DeltaSizeInfo):
      return self._DescribeDeltaSizeInfo(obj)
    if isinstance(obj, models.SizeInfo):
      return self._DescribeSizeInfo(obj)
    if isinstance(obj, models.DeltaSymbolGroup):
      return self._DescribeDeltaSymbolGroup(obj)
    if isinstance(obj, models.SymbolGroup):
      return self._DescribeSymbolGroup(obj)
    if isinstance(obj, models.Symbol):
      return self._DescribeSymbol(obj)
    return (repr(obj),)


def DescribeSizeInfoCoverage(size_info):
  """Yields lines describing how accurate |size_info| is."""
  for section in models.SECTION_TO_SECTION_NAME:
    if section == 'd':
      expected_size = sum(v for k, v in size_info.section_sizes.iteritems()
                          if k.startswith('.data'))
    else:
      expected_size = size_info.section_sizes[
          models.SECTION_TO_SECTION_NAME[section]]

    in_section = size_info.raw_symbols.WhereInSection(section)
    actual_size = in_section.size
    size_percent = _Divide(actual_size, expected_size)
    yield ('Section {}: has {:.1%} of {} bytes accounted for from '
           '{} symbols. {} bytes are unaccounted for.').format(
               section, size_percent, actual_size, len(in_section),
               expected_size - actual_size)
    star_syms = in_section.WhereNameMatches(r'^\*')
    padding = in_section.padding - star_syms.padding
    anonymous_syms = star_syms.Inverted().WhereHasAnyAttribution().Inverted()
    yield '* Padding accounts for {} bytes ({:.1%})'.format(
        padding, _Divide(padding, in_section.size))
    if len(star_syms):
      yield ('* {} placeholders (symbols that start with **) account for '
             '{} bytes ({:.1%})').format(
                 len(star_syms), star_syms.size,
                 _Divide(star_syms.size,  in_section.size))
    if anonymous_syms:
      yield '* {} anonymous symbols account for {} bytes ({:.1%})'.format(
          len(anonymous_syms), int(anonymous_syms.pss),
          _Divide(star_syms.size, in_section.size))

    aliased_symbols = in_section.Filter(lambda s: s.aliases)
    if section == 't':
      if len(aliased_symbols):
        uniques = sum(1 for s in aliased_symbols.IterUniqueSymbols())
        yield ('* Contains {} aliases, mapped to {} unique addresses '
               '({} bytes)').format(
                   len(aliased_symbols), uniques, aliased_symbols.size)
      else:
        yield '* Contains 0 aliases'

    inlined_symbols = in_section.WhereObjectPathMatches('{shared}')
    if len(inlined_symbols):
      yield '* {} symbols have shared ownership ({} bytes)'.format(
          len(inlined_symbols), inlined_symbols.size)
    else:
      yield '* 0 symbols have shared ownership'



def _UtcToLocal(utc):
  epoch = time.mktime(utc.timetuple())
  offset = (datetime.datetime.fromtimestamp(epoch) -
            datetime.datetime.utcfromtimestamp(epoch))
  return utc + offset


def DescribeMetadata(metadata):
  display_dict = metadata.copy()
  timestamp = display_dict.get(models.METADATA_ELF_MTIME)
  if timestamp:
    timestamp_obj = datetime.datetime.utcfromtimestamp(timestamp)
    display_dict[models.METADATA_ELF_MTIME] = (
        _UtcToLocal(timestamp_obj).strftime('%Y-%m-%d %H:%M:%S'))
  gn_args = display_dict.get(models.METADATA_GN_ARGS)
  if gn_args:
    display_dict[models.METADATA_GN_ARGS] = ' '.join(gn_args)
  return sorted('%s=%s' % t for t in display_dict.iteritems())


def GenerateLines(obj, verbose=False, recursive=False):
  """Returns an iterable of lines (without \n) that describes |obj|."""
  return Describer(verbose=verbose, recursive=recursive).GenerateLines(obj)


def WriteLines(lines, func):
  for l in lines:
    func(l)
    func('\n')
