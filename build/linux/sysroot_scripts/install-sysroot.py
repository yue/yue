#!/usr/bin/env python
# Copyright (c) 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Install Debian Wheezy sysroots for building chromium.
"""

# The sysroot is needed to ensure that binaries will run on Debian Wheezy,
# the oldest supported linux distribution.  This script can be run manually but
# is more often run as part of gclient hooks. When run from hooks this script
# in a no-op on non-linux platforms.

# The sysroot image could be constructed from scratch based on the current
# state or Debian Wheezy but for consistency we currently use a pre-built root
# image. The image will normally need to be rebuilt every time chrome's build
# dependencies are changed.

import hashlib
import platform
import optparse
import os
import re
import shutil
import subprocess
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))


URL_PREFIX = 'https://commondatastorage.googleapis.com'
URL_PATH = 'chrome-linux-sysroot/toolchain'
REVISION_AMD64 = 'c09e94d74f6efc174bd88195eee902b4350fc006'
REVISION_ARM = 'c09e94d74f6efc174bd88195eee902b4350fc006'
REVISION_ARM64 = 'bd10c315594d2a20e31a94a7a6c7adb9a0961c56'
REVISION_I386 = 'c09e94d74f6efc174bd88195eee902b4350fc006'
REVISION_MIPS = 'c09e94d74f6efc174bd88195eee902b4350fc006'
TARBALL_AMD64 = 'debian_wheezy_amd64_sysroot.tgz'
TARBALL_ARM = 'debian_wheezy_arm_sysroot.tgz'
TARBALL_ARM64 = 'debian_jessie_arm64_sysroot.tgz'
TARBALL_I386 = 'debian_wheezy_i386_sysroot.tgz'
TARBALL_MIPS = 'debian_wheezy_mips_sysroot.tgz'
TARBALL_AMD64_SHA1SUM = 'dceb51a6bd358f7ef64062e813f4698464e89554'
TARBALL_ARM_SHA1SUM = '9dfbf23878a75fc8d482f39b0569314bbb869d63'
TARBALL_ARM64_SHA1SUM = '0db3be51912e0be46bb1b906fc196c5c1dfc090f'
TARBALL_I386_SHA1SUM = '89da0ad90690c0bc71222caf0d55496f0ad8c201'
TARBALL_MIPS_SHA1SUM = '652631078c59f1c1f2bbb24637ef33374a2e8cdf'
SYSROOT_DIR_AMD64 = 'debian_wheezy_amd64-sysroot'
SYSROOT_DIR_ARM = 'debian_wheezy_arm-sysroot'
SYSROOT_DIR_ARM64 = 'debian_jessie_arm64-sysroot'
SYSROOT_DIR_I386 = 'debian_wheezy_i386-sysroot'
SYSROOT_DIR_MIPS = 'debian_wheezy_mips-sysroot'

valid_archs = ('arm', 'arm64', 'i386', 'amd64', 'mips')


class Error(Exception):
  pass


def GetSha1(filename):
  sha1 = hashlib.sha1()
  with open(filename, 'rb') as f:
    while True:
      # Read in 1mb chunks, so it doesn't all have to be loaded into memory.
      chunk = f.read(1024*1024)
      if not chunk:
        break
      sha1.update(chunk)
  return sha1.hexdigest()


def InstallDefaultSysroots():
  """Install the default set of sysroot images.

  This includes at least the sysroot for host architecture, and the 32-bit
  sysroot for building the v8 snapshot image.  It can also include the cross
  compile sysroot for ARM/MIPS if cross compiling environment can be detected.
  """
  InstallSysroot('amd64')
  InstallSysroot('i386')
  InstallSysroot('arm')
  InstallSysroot('arm64')


def main(args):
  parser = optparse.OptionParser('usage: %prog [OPTIONS]', description=__doc__)
  parser.add_option('--running-as-hook', action='store_true',
                    default=False, help='Used when running from gclient hooks.'
                                        ' Installs default sysroot images.')
  parser.add_option('--arch', type='choice', choices=valid_archs,
                    help='Sysroot architecture: %s' % ', '.join(valid_archs))
  options, _ = parser.parse_args(args)

  if options.arch:
    InstallSysroot(options.arch)
  else:
    InstallDefaultSysroots()

  return 0


def InstallSysroot(target_arch):
  # The sysroot directory should match the one specified in build/common.gypi.
  # TODO(thestig) Consider putting this else where to avoid having to recreate
  # it on every build.
  linux_dir = os.path.dirname(SCRIPT_DIR)
  debian_release = 'Wheezy'
  if target_arch == 'amd64':
    sysroot = os.path.join(linux_dir, SYSROOT_DIR_AMD64)
    tarball_filename = TARBALL_AMD64
    tarball_sha1sum = TARBALL_AMD64_SHA1SUM
    revision = REVISION_AMD64
  elif target_arch == 'arm':
    sysroot = os.path.join(linux_dir, SYSROOT_DIR_ARM)
    tarball_filename = TARBALL_ARM
    tarball_sha1sum = TARBALL_ARM_SHA1SUM
    revision = REVISION_ARM
  elif target_arch == 'arm64':
    debian_release = 'Jessie'
    sysroot = os.path.join(linux_dir, SYSROOT_DIR_ARM64)
    tarball_filename = TARBALL_ARM64
    tarball_sha1sum = TARBALL_ARM64_SHA1SUM
    revision = REVISION_ARM64
  elif target_arch == 'i386':
    sysroot = os.path.join(linux_dir, SYSROOT_DIR_I386)
    tarball_filename = TARBALL_I386
    tarball_sha1sum = TARBALL_I386_SHA1SUM
    revision = REVISION_I386
  elif target_arch == 'mips':
    sysroot = os.path.join(linux_dir, SYSROOT_DIR_MIPS)
    tarball_filename = TARBALL_MIPS
    tarball_sha1sum = TARBALL_MIPS_SHA1SUM
    revision = REVISION_MIPS
  else:
    raise Error('Unknown architecture: %s' % target_arch)

  url = '%s/%s/%s/%s' % (URL_PREFIX, URL_PATH, revision, tarball_filename)

  stamp = os.path.join(sysroot, '.stamp')
  if os.path.exists(stamp):
    with open(stamp) as s:
      if s.read() == url:
        print 'Debian Wheezy %s root image already up to date: %s' % \
            (target_arch, sysroot)
        return

  print 'Installing Debian Wheezy %s root image: %s' % (target_arch, sysroot)
  if os.path.isdir(sysroot):
    shutil.rmtree(sysroot)
  os.mkdir(sysroot)
  tarball = os.path.join(sysroot, tarball_filename)
  print 'Downloading %s' % url
  sys.stdout.flush()
  sys.stderr.flush()
  subprocess.check_call(
      ['curl', '--fail', '--retry', '3', '-L', url, '-o', tarball])
  sha1sum = GetSha1(tarball)
  if sha1sum != tarball_sha1sum:
    raise Error('Tarball sha1sum is wrong.'
                'Expected %s, actual: %s' % (tarball_sha1sum, sha1sum))
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
