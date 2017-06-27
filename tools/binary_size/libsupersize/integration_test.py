#!/usr/bin/env python
# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import contextlib
import copy
import difflib
import glob
import itertools
import logging
import os
import unittest
import re
import subprocess
import sys
import tempfile

import archive
import describe
import diff
import file_format
import models


_SCRIPT_DIR = os.path.dirname(__file__)
_TEST_DATA_DIR = os.path.join(_SCRIPT_DIR, 'testdata')
_TEST_OUTPUT_DIR = os.path.join(_TEST_DATA_DIR, 'mock_output_directory')
_TEST_TOOL_PREFIX = os.path.join(
    os.path.abspath(_TEST_DATA_DIR), 'mock_toolchain', '')
_TEST_MAP_PATH = os.path.join(_TEST_DATA_DIR, 'test.map')
_TEST_ELF_PATH = os.path.join(_TEST_OUTPUT_DIR, 'elf')

update_goldens = False


def _AssertGolden(expected_lines, actual_lines):
  expected = list(expected_lines)
  actual = list(l + '\n' for l in actual_lines)
  assert actual == expected, ('Did not match .golden.\n' +
      ''.join(difflib.unified_diff(expected, actual, 'expected', 'actual')))


def _CompareWithGolden(name=None):
  def real_decorator(func):
    basename = name
    if not basename:
      basename = func.__name__.replace('test_', '')
    golden_path = os.path.join(_TEST_DATA_DIR, basename + '.golden')

    def inner(self):
      actual_lines = func(self)
      actual_lines = (re.sub(r'(elf_mtime=).*', r'\1{redacted}', l)
                      for l in actual_lines)
      actual_lines = (re.sub(r'(Loaded from ).*', r'\1{redacted}', l)
                      for l in actual_lines)

      if update_goldens:
        with open(golden_path, 'w') as file_obj:
          describe.WriteLines(actual_lines, file_obj.write)
        logging.info('Wrote %s', golden_path)
      else:
        with open(golden_path) as file_obj:
          _AssertGolden(file_obj, actual_lines)
    return inner
  return real_decorator


@contextlib.contextmanager
def _AddMocksToPath():
  prev_path = os.environ['PATH']
  os.environ['PATH'] = _TEST_TOOL_PREFIX[:-1] + os.path.pathsep + prev_path
  yield
  os.environ['PATH'] = prev_path


def _RunApp(name, args, debug_measures=False):
  argv = [os.path.join(_SCRIPT_DIR, 'main.py'), name, '--no-pypy']
  argv.extend(args)
  with _AddMocksToPath():
    env = None
    if debug_measures:
      env = os.environ.copy()
      env['SUPERSIZE_DISABLE_ASYNC'] = '1'
      env['SUPERSIZE_MEASURE_GZIP'] = '1'

    return subprocess.check_output(argv, env=env).splitlines()


def _DiffCounts(sym):
  counts = sym.CountsByDiffStatus()
  return (counts[models.DIFF_STATUS_CHANGED],
          counts[models.DIFF_STATUS_ADDED],
          counts[models.DIFF_STATUS_REMOVED])


class IntegrationTest(unittest.TestCase):
  maxDiff = None  # Don't trucate diffs in errors.
  cached_size_info = [None, None, None]

  def _CloneSizeInfo(self, use_output_directory=True, use_elf=True):
    assert not use_elf or use_output_directory
    i = int(use_output_directory) + int(use_elf)
    if not IntegrationTest.cached_size_info[i]:
      elf_path = _TEST_ELF_PATH if use_elf else None
      output_directory = _TEST_OUTPUT_DIR if use_output_directory else None
      IntegrationTest.cached_size_info[i] = archive.CreateSizeInfo(
          _TEST_MAP_PATH, elf_path, _TEST_TOOL_PREFIX, output_directory)
      if use_elf:
        with _AddMocksToPath():
          IntegrationTest.cached_size_info[i].metadata = archive.CreateMetadata(
              _TEST_MAP_PATH, elf_path, None, _TEST_TOOL_PREFIX,
              output_directory)
    return copy.deepcopy(IntegrationTest.cached_size_info[i])

  def _DoArchiveTest(self, use_output_directory=True, use_elf=True,
                     debug_measures=False):
    with tempfile.NamedTemporaryFile(suffix='.size') as temp_file:
      args = [temp_file.name, '--map-file', _TEST_MAP_PATH]
      if use_output_directory:
        # Let autodetection find output_directory when --elf-file is used.
        if not use_elf:
          args += ['--output-directory', _TEST_OUTPUT_DIR]
      else:
        args += ['--no-source-paths']
      if use_elf:
        args += ['--elf-file', _TEST_ELF_PATH]
      _RunApp('archive', args, debug_measures=debug_measures)
      size_info = archive.LoadAndPostProcessSizeInfo(temp_file.name)
    # Check that saving & loading is the same as directly parsing the .map.
    expected_size_info = self._CloneSizeInfo(
        use_output_directory=use_output_directory, use_elf=use_elf)
    self.assertEquals(expected_size_info.metadata, size_info.metadata)
    # Don't cluster.
    expected_size_info.symbols = expected_size_info.raw_symbols
    size_info.symbols = size_info.raw_symbols
    expected = list(describe.GenerateLines(expected_size_info))
    actual = list(describe.GenerateLines(size_info))
    self.assertEquals(expected, actual)

    sym_strs = (repr(sym) for sym in size_info.symbols)
    stats = describe.DescribeSizeInfoCoverage(size_info)
    if size_info.metadata:
      metadata = describe.DescribeMetadata(size_info.metadata)
    else:
      metadata = []
    return itertools.chain(metadata, stats, sym_strs)

  @_CompareWithGolden()
  def test_Archive(self):
    return self._DoArchiveTest(use_output_directory=False, use_elf=False)

  @_CompareWithGolden()
  def test_Archive_OutputDirectory(self):
    return self._DoArchiveTest(use_elf=False)

  @_CompareWithGolden()
  def test_Archive_Elf(self):
    return self._DoArchiveTest()

  @_CompareWithGolden(name='Archive_Elf')
  def test_Archive_Elf_DebugMeasures(self):
    return self._DoArchiveTest(debug_measures=True)

  @_CompareWithGolden()
  def test_Console(self):
    with tempfile.NamedTemporaryFile(suffix='.size') as size_file, \
         tempfile.NamedTemporaryFile(suffix='.txt') as output_file:
      file_format.SaveSizeInfo(self._CloneSizeInfo(), size_file.name)
      query = [
          'ShowExamples()',
          'ExpandRegex("_foo_")',
          'canned_queries.CategorizeGenerated()',
          'canned_queries.CategorizeByChromeComponent()',
          'canned_queries.TemplatesByName()',
          'Print(size_info, to_file=%r)' % output_file.name,
      ]
      ret = _RunApp('console', [size_file.name, '--query', '; '.join(query)])
      with open(output_file.name) as f:
        ret.extend(l.rstrip() for l in f)
      return ret

  @_CompareWithGolden()
  def test_Diff_NullDiff(self):
    with tempfile.NamedTemporaryFile(suffix='.size') as temp_file:
      file_format.SaveSizeInfo(self._CloneSizeInfo(), temp_file.name)
      return _RunApp('diff', [temp_file.name, temp_file.name])

  @_CompareWithGolden()
  def test_Diff_Basic(self):
    size_info1 = self._CloneSizeInfo(use_elf=False)
    size_info2 = self._CloneSizeInfo(use_elf=False)
    size_info1.metadata = {"foo": 1, "bar": [1,2,3], "baz": "yes"}
    size_info2.metadata = {"foo": 1, "bar": [1,3], "baz": "yes"}
    size_info1.symbols -= size_info1.symbols[:2]
    size_info2.symbols -= size_info2.symbols[-3:]
    size_info1.symbols[1].size -= 10
    d = diff.Diff(size_info1, size_info2)
    d.symbols = d.symbols.Sorted()
    return describe.GenerateLines(d, verbose=True)

  def test_Diff_Aliases1(self):
    size_info1 = self._CloneSizeInfo()
    size_info2 = self._CloneSizeInfo()

    # Removing 1 alias should not change the size.
    a1, _, _ = (
        size_info2.raw_symbols.Filter(lambda s: s.num_aliases == 3)[0].aliases)
    size_info2.raw_symbols -= [a1]
    a1.aliases.remove(a1)
    d = diff.Diff(size_info1, size_info2)
    self.assertEquals(d.raw_symbols.pss, 0)
    self.assertEquals((0, 0, 1), _DiffCounts(d.raw_symbols))
    # shrinkToFit is in a cluster, so removed turns to a changed when clustered.
    self.assertEquals((1, 0, 0), _DiffCounts(d.symbols.GroupedByFullName()))

    # Adding one alias should not change size.
    d = diff.Diff(size_info2, size_info1)
    self.assertEquals(d.raw_symbols.pss, 0)
    self.assertEquals((0, 1, 0), _DiffCounts(d.raw_symbols))
    self.assertEquals((1, 0, 0), _DiffCounts(d.symbols.GroupedByFullName()))

  def test_Diff_Aliases2(self):
    size_info1 = self._CloneSizeInfo()
    size_info2 = self._CloneSizeInfo()

    # Removing 2 aliases should not change the size.
    a1, a2, _ = (
        size_info2.raw_symbols.Filter(lambda s: s.num_aliases == 3)[0].aliases)
    size_info2.raw_symbols -= [a1, a2]
    a1.aliases.remove(a1)
    a1.aliases.remove(a2)
    d = diff.Diff(size_info1, size_info2)
    self.assertEquals(d.raw_symbols.pss, 0)
    self.assertEquals((0, 0, 2), _DiffCounts(d.raw_symbols))
    self.assertEquals((1, 0, 1), _DiffCounts(d.symbols.GroupedByFullName()))

    # Adding 2 aliases should not change size.
    d = diff.Diff(size_info2, size_info1)
    self.assertEquals(d.raw_symbols.pss, 0)
    self.assertEquals((0, 2, 0), _DiffCounts(d.raw_symbols))
    self.assertEquals((1, 1, 0), _DiffCounts(d.symbols.GroupedByFullName()))

  def test_Diff_Aliases3(self):
    size_info1 = self._CloneSizeInfo()
    size_info2 = self._CloneSizeInfo()

    # Removing all 3 aliases should change the size.
    a1, a2, a3 = (
        size_info2.raw_symbols.Filter(lambda s: s.num_aliases == 3)[0].aliases)
    size_info2.raw_symbols -= [a1, a2, a3]
    d = diff.Diff(size_info1, size_info2)
    self.assertEquals((0, 0, 3), _DiffCounts(d.raw_symbols))
    self.assertEquals((1, 0, 2), _DiffCounts(d.symbols.GroupedByFullName()))

    # Adding all 3 aliases should change size.
    d = diff.Diff(size_info2, size_info1)
    self.assertEquals(d.raw_symbols.pss, a1.size)
    self.assertEquals((0, 3, 0), _DiffCounts(d.raw_symbols))
    self.assertEquals((1, 2, 0), _DiffCounts(d.symbols.GroupedByFullName()))

  def test_Diff_Clustering(self):
    size_info1 = self._CloneSizeInfo()
    size_info2 = self._CloneSizeInfo()
    S = '.text'
    size_info1.symbols += [
        models.Symbol(S, 11, name='.L__unnamed_1193', object_path='a'), # 1
        models.Symbol(S, 22, name='.L__unnamed_1194', object_path='a'), # 2
        models.Symbol(S, 33, name='.L__unnamed_1195', object_path='b'), # 3
        models.Symbol(S, 44, name='.L__bar_195', object_path='b'), # 4
        models.Symbol(S, 55, name='.L__bar_1195', object_path='b'), # 5
    ]
    size_info2.symbols += [
        models.Symbol(S, 33, name='.L__unnamed_2195', object_path='b'), # 3
        models.Symbol(S, 11, name='.L__unnamed_2194', object_path='a'), # 1
        models.Symbol(S, 22, name='.L__unnamed_2193', object_path='a'), # 2
        models.Symbol(S, 44, name='.L__bar_2195', object_path='b'), # 4
        models.Symbol(S, 55, name='.L__bar_295', object_path='b'), # 5
    ]
    d = diff.Diff(size_info1, size_info2)
    d.symbols = d.symbols.Sorted()
    self.assertEquals(d.symbols.CountsByDiffStatus()[models.DIFF_STATUS_ADDED],
                      0)
    self.assertEquals(d.symbols.size, 0)

  @_CompareWithGolden()
  def test_FullDescription(self):
    size_info = self._CloneSizeInfo()
    # Show both clustered and non-clustered so that they can be compared.
    size_info.symbols = size_info.raw_symbols
    return itertools.chain(
        describe.GenerateLines(size_info, verbose=True),
        describe.GenerateLines(size_info.symbols._Clustered(), recursive=True,
                               verbose=True),
    )

  @_CompareWithGolden()
  def test_SymbolGroupMethods(self):
    all_syms = self._CloneSizeInfo().symbols
    global_syms = all_syms.WhereNameMatches('GLOBAL')
    # Tests Filter(), Inverted(), and __sub__().
    non_global_syms = global_syms.Inverted()
    self.assertEqual(non_global_syms, (all_syms - global_syms))
    # Tests Sorted() and __add__().
    self.assertEqual(all_syms.Sorted(),
                     (global_syms + non_global_syms).Sorted())
    # Tests GroupedByName() and __len__().
    return itertools.chain(
        ['GroupedByName()'],
        describe.GenerateLines(all_syms.GroupedByName()),
        ['GroupedByName(depth=1)'],
        describe.GenerateLines(all_syms.GroupedByName(depth=1)),
        ['GroupedByName(depth=-1)'],
        describe.GenerateLines(all_syms.GroupedByName(depth=-1)),
        ['GroupedByName(depth=1, min_count=2)'],
        describe.GenerateLines(all_syms.GroupedByName(depth=1, min_count=2)),
    )


def main():
  argv = sys.argv
  if len(argv) > 1 and argv[1] == '--update':
    argv.pop(0)
    global update_goldens
    update_goldens = True
    for f in glob.glob(os.path.join(_TEST_DATA_DIR, '*.golden')):
      os.unlink(f)

  unittest.main(argv=argv, verbosity=2)


if __name__ == '__main__':
  main()
