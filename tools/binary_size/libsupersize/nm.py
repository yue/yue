# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Functions that rely on parsing output of "nm" tool."""

import atexit
import collections
import errno
import logging
import os
import subprocess
import sys

import concurrent

_active_subprocesses = None


def _IsRelevantNmName(name):
  # Skip lines like:
  # 00000000 t $t
  # 00000000 r $d
  # 0000041b r .L.str
  # 0000041b r .L.str.38
  # 00000344 N
  return name and not name.startswith('.L.str') and not (
      len(name) == 2 and name.startswith('$'))


def _IsRelevantObjectFileName(name):
  # Prevent marking compiler-generated symbols as candidates for shared paths.
  # E.g., multiple files might have "CSWTCH.12", but they are different symbols.
  #
  # Find these via:
  #   size_info.symbols.GroupedByFullName(min_count=-2).Filter(
  #       lambda s: s.WhereObjectPathMatches('{')).SortedByCount()
  # and then search for {shared}.
  # List of names this applies to:
  #   startup
  #   __tcf_0  <-- Generated for global destructors.
  #   ._79
  #   .Lswitch.table, .Lswitch.table.12
  #   CSWTCH.12
  #   lock.12
  #   table.12
  #   __compound_literal.12
  #   .L.ref.tmp.1
  #   .L.str, .L.str.3
  if name in ('__tcf_0', 'startup'):
    return False
  if name.startswith('._') and name[2:].isdigit():
    return False
  if name.startswith('.L') and name.find('.', 2) != -1:
    return False

  dot_idx = name.find('.')
  if dot_idx == -1:
    return True
  name = name[:dot_idx]

  return name not in (
      'CSWTCH', 'lock', '__compound_literal', '__func__', 'table')


def CollectAliasesByAddress(elf_path, tool_prefix):
  """Runs nm on |elf_path| and returns a dict of address->[names]"""
  names_by_address = collections.defaultdict(list)

  # About 60mb of output, but piping takes ~30s, and loading it into RAM
  # directly takes 3s.
  args = [tool_prefix + 'nm', '--no-sort', '--defined-only', '--demangle',
          elf_path]
  output = subprocess.check_output(args)
  for line in output.splitlines():
    space_idx = line.find(' ')
    address_str = line[:space_idx]
    section = line[space_idx + 1]
    name = line[space_idx + 3:]

    # To verify that rodata does not have aliases:
    #   nm --no-sort --defined-only libchrome.so > nm.out
    #   grep -v '\$' nm.out | grep ' r ' | sort | cut -d' ' -f1 > addrs
    #   wc -l < addrs; uniq < addrs | wc -l
    if section not in 'tT' or not _IsRelevantNmName(name):
      continue

    address = int(address_str, 16)
    if not address:
      continue
    # Constructors often show up twice.
    name_list = names_by_address[address]
    if name not in name_list:
      name_list.append(name)

  # Since this is run in a separate process, minimize data passing by returning
  # only aliased symbols.
  names_by_address = {k: v for k, v in names_by_address.iteritems()
                      if len(v) > 1}

  return names_by_address


def _CollectAliasesByAddressAsyncHelper(elf_path, tool_prefix):
  result = CollectAliasesByAddress(elf_path, tool_prefix)
  return concurrent.EncodeDictOfLists(result, key_transform=str)


def CollectAliasesByAddressAsync(elf_path, tool_prefix):
  """Calls CollectAliasesByAddress in a helper process. Returns a Result."""
  def decode(encoded):
    return concurrent.DecodeDictOfLists(
        encoded[0], encoded[1], key_transform=int)
  return concurrent.ForkAndCall(
      _CollectAliasesByAddressAsyncHelper, (elf_path, tool_prefix),
      decode_func=decode)


def _ParseOneObjectFileOutput(lines):
  ret = []
  for line in lines:
    if not line:
      break
    space_idx = line.find(' ')  # Skip over address.
    name = line[space_idx + 3:]
    if _IsRelevantNmName(name) and _IsRelevantObjectFileName(name):
      ret.append(name)
  return ret


def _BatchCollectNames(target, tool_prefix, output_directory):
  is_archive = isinstance(target, basestring)
  # Ensure tool_prefix is absolute so that CWD does not affect it
  if os.path.sep in tool_prefix:
    # Use abspath() on the dirname to avoid it stripping a trailing /.
    dirname = os.path.dirname(tool_prefix)
    tool_prefix = os.path.abspath(dirname) + tool_prefix[len(dirname):]

  args = [tool_prefix + 'nm', '--no-sort', '--defined-only', '--demangle']
  if is_archive:
    args.append(target)
  else:
    args.extend(target)
  output = subprocess.check_output(args, cwd=output_directory)
  lines = output.splitlines()
  if not lines:
    return '', ''
  is_multi_file = not lines[0]
  lines = iter(lines)
  if is_multi_file:
    next(lines)
    path = next(lines)[:-1]  # Path ends with a colon.
  else:
    assert not is_archive
    path = target[0]

  ret = {}
  while True:
    if is_archive:
      # E.g. foo/bar.a(baz.o)
      path = '%s(%s)' % (target, path)
    # The multiprocess API uses pickle, which is ridiculously slow. More than 2x
    # faster to use join & split.
    ret[path] = _ParseOneObjectFileOutput(lines)
    path = next(lines, ':')[:-1]
    if not path:
      return concurrent.EncodeDictOfLists(ret)


class _BulkObjectFileAnalyzerWorker(object):
  """Runs nm on all given paths and returns a dict of name->[paths]"""

  def __init__(self, tool_prefix, output_directory):
    self._tool_prefix = tool_prefix
    self._output_directory = output_directory
    self._batches = []
    self._result = None

  def AnalyzePaths(self, paths):
    def iter_job_params():
      object_paths = []
      for path in paths:
        if path.endswith('.a'):
          yield path, self._tool_prefix, self._output_directory
        else:
          object_paths.append(path)

      BATCH_SIZE = 50  # Chosen arbitrarily.
      for i in xrange(0, len(object_paths), BATCH_SIZE):
        batch = object_paths[i:i + BATCH_SIZE]
        yield batch, self._tool_prefix, self._output_directory

    paths_by_name = collections.defaultdict(list)
    params = list(iter_job_params())
    for encoded_ret in concurrent.BulkForkAndCall(_BatchCollectNames, params):
      names_by_path = concurrent.DecodeDictOfLists(*encoded_ret)
      for path, names in names_by_path.iteritems():
        for name in names:
          paths_by_name[name].append(path)
    self._batches.append(paths_by_name)

  def Close(self):
    assert self._result is None
    assert self._batches
    paths_by_name = self._batches[0]
    for batch in self._batches[1:]:
      for name, path_list in batch.iteritems():
        paths_by_name.setdefault(name, []).extend(path_list)

    # It would speed up mashalling of the values by removing all entries
    # that have only 1 path. However, these entries are needed to give
    # path information to symbol aliases.
    self._result = paths_by_name

  def Get(self):
    assert self._result is not None
    return self._result


def _TerminateSubprocesses():
  for proc in _active_subprocesses:
    proc.kill()


class _BulkObjectFileAnalyzerMaster(object):
  """Runs BulkObjectFileAnalyzer in a subprocess."""

  def __init__(self, tool_prefix, output_directory):
    self._process = None
    self._tool_prefix = tool_prefix
    self._output_directory = output_directory

  def _Spawn(self):
    global _active_subprocesses
    log_level = str(logging.getLogger().getEffectiveLevel())
    args = [sys.executable, __file__, log_level, self._tool_prefix,
            self._output_directory]
    self._process = subprocess.Popen(
        args, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    if _active_subprocesses is None:
      _active_subprocesses = []
      atexit.register(_TerminateSubprocesses)
    _active_subprocesses.append(self._process)

  def AnalyzePaths(self, paths):
    if self._process is None:
      self._Spawn()

    logging.debug('Sending batch of %d paths to subprocess', len(paths))
    payload = '\x01'.join(paths)
    self._process.stdin.write('{:08x}'.format(len(payload)))
    self._process.stdin.write(payload)

  def Close(self):
    assert not self._process.stdin.closed
    self._process.stdin.close()
    _active_subprocesses.remove(self._process)

  def Get(self):
    assert self._process.stdin.closed
    logging.debug('Decoding nm results from forked process')

    encoded_keys_len = int(self._process.stdout.read(8), 16)
    encoded_keys = self._process.stdout.read(encoded_keys_len)
    encoded_values = self._process.stdout.read()
    return concurrent.DecodeDictOfLists(encoded_keys, encoded_values)


BulkObjectFileAnalyzer = _BulkObjectFileAnalyzerMaster
if concurrent.DISABLE_ASYNC:
  BulkObjectFileAnalyzer = _BulkObjectFileAnalyzerWorker


def _SubMain(log_level, tool_prefix, output_directory):
  logging.basicConfig(
      level=int(log_level),
      format='nm: %(levelname).1s %(relativeCreated)6d %(message)s')
  bulk_analyzer = _BulkObjectFileAnalyzerWorker(tool_prefix, output_directory)
  while True:
    payload_len = int(sys.stdin.read(8) or '0', 16)
    if not payload_len:
      logging.debug('nm bulk subprocess received eof.')
      break
    paths = sys.stdin.read(payload_len).split('\x01')
    bulk_analyzer.AnalyzePaths(paths)

  bulk_analyzer.Close()
  paths_by_name = bulk_analyzer.Get()
  encoded_keys, encoded_values = concurrent.EncodeDictOfLists(paths_by_name)
  try:
    sys.stdout.write('%08x' % len(encoded_keys))
    sys.stdout.write(encoded_keys)
    sys.stdout.write(encoded_values)
  except IOError, e:
    # Parent process exited.
    if e.errno == errno.EPIPE:
      sys.exit(1)

  logging.debug('nm bulk subprocess finished.')


if __name__ == '__main__':
  _SubMain(*sys.argv[1:])
