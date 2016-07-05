#!/usr/bin/env python
# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Runs the 'ar' command after removing its output file first.

This script is invoked like:
  python gcc_ar_wrapper.py --ar=$AR --output=$OUT $OP $INPUTS
to do the equivalent of:
  rm -f $OUT && $AR $OP $OUT $INPUTS
"""

import argparse
import os
import subprocess
import sys


# When running on a Windows host and using a toolchain whose tools are
# actually wrapper scripts (i.e. .bat files on Windows) rather than binary
# executables, the "command" to run has to be prefixed with this magic.
# The GN toolchain definitions take care of that for when GN/Ninja is
# running the tool directly.  When that command is passed in to this
# script, it appears as a unitary string but needs to be split up so that
# just 'cmd' is the actual command given to Python's subprocess module.
BAT_PREFIX = 'cmd /c call '

def CommandToRun(command):
  if command[0].startswith(BAT_PREFIX):
    command = command[0].split(None, 3) + command[1:]
  return command


def main():
  parser = argparse.ArgumentParser(description=__doc__)
  parser.add_argument('--ar',
                      required=True,
                      help='The ar binary to run',
                      metavar='PATH')
  parser.add_argument('--output',
                      required=True,
                      help='Output archive file',
                      metavar='ARCHIVE')
  parser.add_argument('--plugin',
                      help='Load plugin')
  parser.add_argument('operation',
                      help='Operation on the archive')
  parser.add_argument('inputs', nargs='+',
                      help='Input files')
  args = parser.parse_args()

  command = [args.ar, args.operation]
  if args.plugin is not None:
    command += ['--plugin', args.plugin]
  command.append(args.output)
  command += args.inputs

  # Remove the output file first.
  try:
    os.remove(args.output)
  except OSError as e:
    if e.errno != os.errno.ENOENT:
      raise

  # Now just run the ar command.
  return subprocess.call(CommandToRun(command))


if __name__ == "__main__":
  sys.exit(main())
