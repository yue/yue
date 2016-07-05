#!/usr/bin/env python
# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Runs a linking command and optionally a strip command.

This script exists to avoid using complex shell commands in
gcc_toolchain.gni's tool("link"), in case the host running the compiler
does not have a POSIX-like shell (e.g. Windows).
"""

import argparse
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
  parser.add_argument('--strip',
                      help='The strip binary to run',
                      metavar='PATH')
  parser.add_argument('--unstripped-file',
                      required=True,
                      help='Executable file produced by linking command',
                      metavar='FILE')
  parser.add_argument('--output',
                      required=True,
                      help='Final output executable file',
                      metavar='FILE')
  parser.add_argument('command', nargs='+',
                      help='Linking command')
  args = parser.parse_args()

  # First, run the actual link.
  result = subprocess.call(CommandToRun(args.command))
  if result != 0:
    return result

  # Finally, strip the linked executable (if desired).
  if args.strip:
    result = subprocess.call(CommandToRun([
        args.strip, '--strip-unneeded', '-o', args.output, args.unstripped_file
        ]))

  return result


if __name__ == "__main__":
  sys.exit(main())
