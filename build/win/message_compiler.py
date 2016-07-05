# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Runs the Microsoft Message Compiler (mc.exe). This Python adapter is for the
# GN build, which can only run Python and not native binaries.
#
# Usage: message_compiler.py <environment_file> [<args to mc.exe>*]

import subprocess
import sys

# Read the environment block from the file. This is stored in the format used
# by CreateProcess. Drop last 2 NULs, one for list terminator, one for trailing
# vs. separator.
env_pairs = open(sys.argv[1]).read()[:-2].split('\0')
env_dict = dict([item.split('=', 1) for item in env_pairs])

# mc writes to stderr, so this explicitly redirects to stdout and eats it.
try:
  # This needs shell=True to search the path in env_dict for the mc executable.
  subprocess.check_output(["mc.exe"] + sys.argv[2:],
                          env=env_dict,
                          stderr=subprocess.STDOUT,
                          shell=True)
except subprocess.CalledProcessError as e:
  print e.output
  sys.exit(e.returncode)
