#!/usr/bin/env python
# Copyright (c) 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Install Debian sysroots for building chromium.
"""

# The sysroot is needed to ensure that binaries that get built will run on
# the oldest stable version of Debian that we currently support.
# This script can be run manually but is more often run as part of gclient
# hooks. When run from hooks this script is a no-op on non-linux platforms.

# The sysroot image could be constructed from scratch based on the current state
# of the Debian archive but for consistency we use a pre-built root image (we
# don't want upstream changes to Debian to effect the chromium build until we
# choose to pull them in). The images will normally need to be rebuilt every
# time chrome's build dependencies are changed but should also be updated
# periodically to include upstream security fixes from Debian.

from __future__ import print_function

import hashlib
import json
import platform
import optparse
import os
import re
import shutil
import subprocess
import sys
try:
    # For Python 3.0 and later
    from urllib.request import urlopen
except ImportError:
    # Fall back to Python 2's urllib2
    from urllib2 import urlopen

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))


URL_PREFIX = 'https://github.com'
URL_PATH = 'yue/debian-sysroot-image-creator/releases/download'

VALID_ARCHS = ('arm', 'arm64', 'i386', 'amd64', 'mips', 'mips64el')

ARCH_TRANSLATIONS = {
    'x64': 'amd64',
    'x86': 'i386',
    'mipsel': 'mips',
    'mips64': 'mips64el',
}

DEFAULT_TARGET_PLATFORM = 'sid'

class Error(Exception):
  pass


def main(args):
  parser = optparse.OptionParser('usage: %prog [OPTIONS]', description=__doc__)
  parser.add_option('--arch',
                    help='Sysroot architecture: %s' % ', '.join(VALID_ARCHS))
  parser.add_option('--all', action='store_true',
                    help='Install all sysroot images (useful when updating the'
                         ' images)')
  parser.add_option('--print-hash',
                    help='Print the hash of the sysroot for the given arch.')
  options, _ = parser.parse_args(args)
  if not sys.platform.startswith('linux'):
    return 0

  if options.arch:
    InstallSysroot(DEFAULT_TARGET_PLATFORM,
                   ARCH_TRANSLATIONS.get(options.arch, options.arch))
  elif options.all:
    for arch in VALID_ARCHS:
      InstallSysroot(DEFAULT_TARGET_PLATFORM, arch)
  else:
    print('You much specify one of the options.')
    return 1

  return 0


def GetSysrootDict(target_platform, target_arch):
  if target_arch not in VALID_ARCHS:
    raise Error('Unknown architecture: %s' % target_arch)

  sysroots_file = os.path.join(SCRIPT_DIR, 'sysroots.json')
  sysroots = json.load(open(sysroots_file))
  sysroot_key = '%s_%s' % (target_platform, target_arch)
  if sysroot_key not in sysroots:
    raise Error('No sysroot for: %s %s' % (target_platform, target_arch))
  return sysroots[sysroot_key]


def InstallSysroot(target_platform, target_arch):
  sysroot_dict = GetSysrootDict(target_platform, target_arch)
  tarball_filename = sysroot_dict['Tarball']
  tarball_version = sysroot_dict['Version']
  # TODO(thestig) Consider putting this elsewhere to avoid having to recreate
  # it on every build.
  linux_dir = os.path.dirname(os.path.dirname(SCRIPT_DIR))
  sysroot = os.path.join(linux_dir, 'third_party', sysroot_dict['SysrootDir'])

  url = '%s/%s/%s/%s' % (URL_PREFIX, URL_PATH, tarball_version,
                         tarball_filename)

  stamp = os.path.join(sysroot, '.stamp')
  if os.path.exists(stamp):
    with open(stamp) as s:
      if s.read() == url:
        return

  print('Installing Debian %s %s root image: %s' % \
      (target_platform, target_arch, sysroot))
  if os.path.isdir(sysroot):
    shutil.rmtree(sysroot)
  os.mkdir(sysroot)
  tarball = os.path.join(sysroot, tarball_filename)
  print('Downloading %s' % url)
  sys.stdout.flush()
  sys.stderr.flush()
  for _ in range(3):
    try:
      response = urlopen(url)
      with open(tarball, "wb") as f:
        f.write(response.read())
      break
    except Exception:  # Ignore exceptions.
      pass
  else:
    raise Error('Failed to download %s' % url)
  subprocess.check_call(['tar', 'xf', tarball, '-C', sysroot])
  os.remove(tarball)

  with open(stamp, 'w') as s:
    s.write(url)


if __name__ == '__main__':
  try:
    sys.exit(main(sys.argv[1:]))
  except Error as e:
    sys.stderr.write(str(e) + '\n')
    sys.exit(1)
