# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This script computs the number of concurrent links we want to run in the build
# as a function of machine spec. It's based on GetDefaultConcurrentLinks in GYP.

import optparse
import os
import re
import subprocess
import sys

def _GetDefaultConcurrentLinks(is_lto):
  # Inherit the legacy environment variable for people that have set it in GYP.
  pool_size = int(os.getenv('GYP_LINK_CONCURRENCY', 0))
  if pool_size:
    return pool_size

  if sys.platform in ('win32', 'cygwin'):
    import ctypes

    class MEMORYSTATUSEX(ctypes.Structure):
      _fields_ = [
        ("dwLength", ctypes.c_ulong),
        ("dwMemoryLoad", ctypes.c_ulong),
        ("ullTotalPhys", ctypes.c_ulonglong),
        ("ullAvailPhys", ctypes.c_ulonglong),
        ("ullTotalPageFile", ctypes.c_ulonglong),
        ("ullAvailPageFile", ctypes.c_ulonglong),
        ("ullTotalVirtual", ctypes.c_ulonglong),
        ("ullAvailVirtual", ctypes.c_ulonglong),
        ("sullAvailExtendedVirtual", ctypes.c_ulonglong),
      ]

    stat = MEMORYSTATUSEX(dwLength=ctypes.sizeof(MEMORYSTATUSEX))
    ctypes.windll.kernel32.GlobalMemoryStatusEx(ctypes.byref(stat))

    # VS 2015 uses 20% more working set than VS 2013 and can consume all RAM
    # on a 64 GB machine.
    mem_limit = max(1, stat.ullTotalPhys / (5 * (2 ** 30)))  # total / 5GB
    hard_cap = max(1, int(os.getenv('GYP_LINK_CONCURRENCY_MAX', 2**32)))
    return min(mem_limit, hard_cap)
  elif sys.platform.startswith('linux'):
    if os.path.exists("/proc/meminfo"):
      with open("/proc/meminfo") as meminfo:
        memtotal_re = re.compile(r'^MemTotal:\s*(\d*)\s*kB')
        for line in meminfo:
          match = memtotal_re.match(line)
          if not match:
            continue
          mem_total_gb = float(match.group(1)) / (2 ** 20)
          # Allow 8Gb per link on Linux because Gold is quite memory hungry
          mem_per_link_gb = 8
          if is_lto:
            mem_total_gb -= 10 # Reserve
            # For LTO builds the RAM requirements are even higher
            mem_per_link_gb = 24
          return int(max(1, mem_total_gb / mem_per_link_gb))
    return 1
  elif sys.platform == 'darwin':
    try:
      avail_bytes = int(subprocess.check_output(['sysctl', '-n', 'hw.memsize']))
      # A static library debug build of Chromium's unit_tests takes ~2.7GB, so
      # 4GB per ld process allows for some more bloat.
      return max(1, avail_bytes / (4 * (2 ** 30)))  # total / 4GB
    except Exception:
      return 1
  else:
    # TODO(scottmg): Implement this for other platforms.
    return 1

def main():
  parser = optparse.OptionParser()
  parser.add_option('--lto', action="store_true", default=False,
                    help='This is an LTO build with higher memory requirements')
  parser.disable_interspersed_args()
  options, args = parser.parse_args()

  print _GetDefaultConcurrentLinks(is_lto=options.lto)
  return 0

if __name__ == '__main__':
  sys.exit(main())
