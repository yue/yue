# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Functions for dealing with determining --tool-prefix."""

import distutils.spawn
import logging
import os

_STATUS_DETECTED = 1
_STATUS_VERIFIED = 2

SRC_ROOT = os.path.dirname(
    os.path.dirname(os.path.dirname(os.path.dirname(__file__))))


class LazyPaths(object):
  def __init__(self, tool_prefix=None, output_directory=None,
               any_path_within_output_directory=None):
    self._tool_prefix = tool_prefix
    self._output_directory = output_directory
    self._any_path_within_output_directory = any_path_within_output_directory
    self._output_directory_status = (
        _STATUS_DETECTED if output_directory is not None else 0)
    self._tool_prefix_status = (
        _STATUS_DETECTED if tool_prefix is not None else 0)

  @property
  def tool_prefix(self):
    if self._tool_prefix_status < _STATUS_DETECTED:
      self._tool_prefix_status = _STATUS_DETECTED
      self._tool_prefix = self._DetectToolPrefix() or ''
      logging.debug('Detected --tool-prefix=%s', self._tool_prefix)
    return self._tool_prefix

  @property
  def output_directory(self):
    if self._output_directory_status < _STATUS_DETECTED:
      self._output_directory_status = _STATUS_DETECTED
      self._output_directory = self._DetectOutputDirectory()
      logging.debug('Detected --output-directory=%s', self._output_directory)
    return self._output_directory

  def VerifyOutputDirectory(self):
    output_directory = self.output_directory
    if self._output_directory_status < _STATUS_VERIFIED:
      self._output_directory_status = _STATUS_VERIFIED
      if not output_directory or not os.path.isdir(output_directory):
        raise Exception('Bad --output-directory. Path not found: %s' %
                        output_directory)
      logging.info('Using --output-directory=%s', output_directory)
    return output_directory

  def VerifyToolPrefix(self):
    tool_prefix = self.tool_prefix
    if self._tool_prefix_status < _STATUS_VERIFIED:
      self._tool_prefix_status = _STATUS_VERIFIED
      if os.path.sep not in tool_prefix:
        full_path = distutils.spawn.find_executable(tool_prefix + 'c++filt')
      else:
        full_path = tool_prefix + 'c++filt'

      if not full_path or not os.path.isfile(full_path):
        raise Exception('Bad --tool-prefix. Path not found: %s' % full_path)
      logging.info('Using --tool-prefix=%s', self._tool_prefix)
    return tool_prefix

  def _DetectOutputDirectory(self):
    # Try and find build.ninja.
    abs_path = os.path.abspath(self._any_path_within_output_directory)
    while True:
      if os.path.exists(os.path.join(abs_path, 'build.ninja')):
        return os.path.relpath(abs_path)
      parent_dir = os.path.dirname(abs_path)
      if parent_dir == abs_path:
        break
      abs_path = abs_path = parent_dir

    # See if CWD=output directory.
    if os.path.exists('build.ninja'):
      return '.'
    return None

  def _DetectToolPrefix(self):
    output_directory = self.output_directory
    if output_directory:
      # Auto-detect from build_vars.txt
      build_vars_path = os.path.join(output_directory, 'build_vars.txt')
      if os.path.exists(build_vars_path):
        with open(build_vars_path) as f:
          build_vars = dict(l.rstrip().split('=', 1) for l in f if '=' in l)
        tool_prefix = build_vars['android_tool_prefix']
        ret = os.path.normpath(os.path.join(output_directory, tool_prefix))
        # Need to maintain a trailing /.
        if tool_prefix.endswith(os.path.sep):
          ret += os.path.sep
        # Check for output directories that have a stale build_vars.txt.
        if os.path.isfile(ret + 'c++filt'):
          return ret
    from_path = distutils.spawn.find_executable('c++filt')
    if from_path:
      return from_path[:-7]
    return None


def FromSrcRootRelative(path):
  ret = os.path.relpath(os.path.join(SRC_ROOT, path))
  # Need to maintain a trailing /.
  if path.endswith(os.path.sep):
    ret += os.path.sep
  return ret


def ToSrcRootRelative(path):
  ret = os.path.relpath(path, SRC_ROOT)
  # Need to maintain a trailing /.
  if path.endswith(os.path.sep):
    ret += os.path.sep
  return ret
