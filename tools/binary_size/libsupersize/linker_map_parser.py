# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import logging

import models

# About linker maps:
# * "Discarded input sections" include symbols merged with other symbols
#   (aliases), so the information there is not actually a list of unused things.
# * Linker maps include symbols that do not have names (with object path),
#   whereas "nm" skips over these (they don't account for much though).
# * The parse time for compressed linker maps is dominated by ungzipping.


class MapFileParser(object):
  """Parses a linker map file (tested only on files from gold linker)."""
  # Map file writer for gold linker:
  # https://github.com/gittup/binutils/blob/HEAD/gold/mapfile.cc

  def __init__(self):
    self._common_symbols = []
    self._symbols = []
    self._section_sizes = {}
    self._lines = None

  def Parse(self, lines):
    """Parses a linker map file.

    Args:
      lines: Iterable of lines.

    Returns:
      A tuple of (section_sizes, symbols).
    """
    self._lines = iter(lines)
    logging.debug('Scanning for Header')

    while True:
      line = self._SkipToLineWithPrefix('Common symbol', 'Memory map')
      if line.startswith('Common symbol'):
        self._common_symbols = self._ParseCommonSymbols()
        logging.debug('.bss common entries: %d', len(self._common_symbols))
        continue
      elif line.startswith('Memory map'):
        self._ParseSections()
      break
    return self._section_sizes, self._symbols

  def _SkipToLineWithPrefix(self, prefix, prefix2=None):
    for l in self._lines:
      if l.startswith(prefix) or (prefix2 and l.startswith(prefix2)):
        return l

  def _ParsePossiblyWrappedParts(self, line, count):
    parts = line.split(None, count - 1)
    if not parts:
      return None
    if len(parts) != count:
      line = next(self._lines)
      parts.extend(line.split(None, count - len(parts) - 1))
      assert len(parts) == count, 'parts: ' + ' '.join(parts)
    parts[-1] = parts[-1].rstrip()
    return parts

  def _ParseCommonSymbols(self):
# Common symbol       size              file
#
# ff_cos_131072       0x40000           obj/third_party/<snip>
# ff_cos_131072_fixed
#                     0x20000           obj/third_party/<snip>
    ret = []
    next(self._lines)  # Skip past blank line

    name, size_str, path = None, None, None
    for l in self._lines:
      parts = self._ParsePossiblyWrappedParts(l, 3)
      if not parts:
        break
      name, size_str, path = parts
      sym = models.Symbol('.bss',  int(size_str[2:], 16), full_name=name,
                          object_path=path)
      ret.append(sym)
    return ret

  def _ParseSections(self):
# .text           0x0028c600  0x22d3468
#  .text.startup._GLOBAL__sub_I_bbr_sender.cc
#                 0x0028c600       0x38 obj/net/net/bbr_sender.o
#  .text._reset   0x00339d00       0xf0 obj/third_party/icu/icuuc/ucnv.o
#  ** fill        0x0255fb00   0x02
#  .text._ZN4base8AutoLockD2Ev
#                 0x00290710        0xe obj/net/net/file_name.o
#                 0x00290711                base::AutoLock::~AutoLock()
#                 0x00290711                base::AutoLock::~AutoLock()
# .text._ZNK5blink15LayoutBlockFlow31mustSeparateMarginAfterForChildERK...
#                0xffffffffffffffff       0x46 obj/...
#                0x006808e1                blink::LayoutBlockFlow::...
# .bss
#  .bss._ZGVZN11GrProcessor11initClassIDI10LightingFPEEvvE8kClassID
#                0x02d4b294        0x4 obj/skia/skia/SkLightingShader.o
#                0x02d4b294   guard variable for void GrProcessor::initClassID
# .data           0x0028c600  0x22d3468
#  .data.rel.ro._ZTVN3gvr7android19ScopedJavaGlobalRefIP12_jfloatArrayEE
#                0x02d1e668       0x10 ../../third_party/.../libfoo.a(bar.o)
#                0x02d1e668   vtable for gvr::android::GlobalRef<_jfloatArray*>
#  ** merge strings
#                 0x0255fb00   0x1f2424
#  ** merge constants
#                 0x0255fb00   0x8
# ** common      0x02db5700   0x13ab48
    syms = self._symbols
    while True:
      line = self._SkipToLineWithPrefix('.')
      if not line:
        break
      section_name = None
      try:
        # Parse section name and size.
        parts = self._ParsePossiblyWrappedParts(line, 3)
        if not parts:
          break
        section_name, section_address_str, section_size_str = parts
        section_address = int(section_address_str[2:], 16)
        section_size = int(section_size_str[2:], 16)
        self._section_sizes[section_name] = section_size
        if (section_name in ('.bss', '.rodata', '.text') or
            section_name.startswith('.data')):
          logging.info('Parsing %s', section_name)
          if section_name == '.bss':
            # Common symbols have no address.
            syms.extend(self._common_symbols)
          prefix_len = len(section_name) + 1  # + 1 for the trailing .
          symbol_gap_count = 0
          merge_symbol_start_address = section_address
          sym_count_at_start = len(syms)
          line = next(self._lines)
          # Parse section symbols.
          while True:
            if not line or line.isspace():
              break
            if line.startswith(' **'):
              zero_index = line.find('0')
              if zero_index == -1:
                # Line wraps.
                name = line.strip()
                line = next(self._lines)
              else:
                # Line does not wrap.
                name = line[:zero_index].strip()
                line = line[zero_index:]
              address_str, size_str = self._ParsePossiblyWrappedParts(line, 2)
              line = next(self._lines)
              # These bytes are already accounted for.
              if name == '** common':
                continue
              address = int(address_str[2:], 16)
              size = int(size_str[2:], 16)
              path = None
              sym = models.Symbol(section_name, size, address=address,
                                  full_name=name, object_path=path)
              syms.append(sym)
              if merge_symbol_start_address > 0:
                merge_symbol_start_address += size
            else:
              # A normal symbol entry.
              subsection_name, address_str, size_str, path = (
                  self._ParsePossiblyWrappedParts(line, 4))
              size = int(size_str[2:], 16)
              assert subsection_name.startswith(section_name), (
                  'subsection name was: ' + subsection_name)
              mangled_name = subsection_name[prefix_len:]
              name = None
              address_str2 = None
              while True:
                line = next(self._lines).rstrip()
                if not line or line.startswith(' .'):
                  break
                # clang includes ** fill, but gcc does not.
                if line.startswith(' ** fill'):
                  # Alignment explicitly recorded in map file. Rather than
                  # record padding based on these entries, we calculate it
                  # using addresses. We do this because fill lines are not
                  # present when compiling with gcc (only for clang).
                  continue
                elif line.startswith(' **'):
                  break
                elif name is None:
                  address_str2, name = self._ParsePossiblyWrappedParts(line, 2)

              if address_str == '0xffffffffffffffff':
                # The section needs special handling (e.g., a merge section)
                # It also generally has a large offset after it, so don't
                # penalize the subsequent symbol for this gap (e.g. a 50kb gap).
                # There seems to be no corelation between where these gaps occur
                # and the symbols they come in-between.
                # TODO(agrieve): Learn more about why this happens.
                if address_str2:
                  address = int(address_str2[2:], 16) - 1
                elif syms and syms[-1].address > 0:
                  # Merge sym with no second line showing real address.
                  address = syms[-1].end_address
                else:
                  logging.warning('First symbol of section had address -1')
                  address = 0

                merge_symbol_start_address = address + size
              else:
                address = int(address_str[2:], 16)
                # Finish off active address gap / merge section.
                if merge_symbol_start_address:
                  merge_size = address - merge_symbol_start_address
                  merge_symbol_start_address = 0
                  if merge_size > 0:
                    # merge_size == 0 for the initial symbol generally.
                    logging.debug('Merge symbol of size %d found at:\n  %r',
                                  merge_size, syms[-1])
                    # Set size=0 so that it will show up as padding.
                    sym = models.Symbol(
                        section_name, 0,
                        address=address,
                        full_name='** symbol gap %d' % symbol_gap_count)
                    symbol_gap_count += 1
                    syms.append(sym)

              sym = models.Symbol(section_name, size, address=address,
                                  full_name=name or mangled_name,
                                  object_path=path)
              syms.append(sym)
          section_end_address = section_address + section_size
          if section_name != '.bss' and (
              syms[-1].end_address < section_end_address):
            # Set size=0 so that it will show up as padding.
            sym = models.Symbol(
                section_name, 0,
                address=section_end_address,
                full_name=(
                    '** symbol gap %d (end of section)' % symbol_gap_count))
            syms.append(sym)
          logging.debug('Symbol count for %s: %d', section_name,
                        len(syms) - sym_count_at_start)
      except:
        logging.error('Problem line: %r', line)
        logging.error('In section: %r', section_name)
        raise
