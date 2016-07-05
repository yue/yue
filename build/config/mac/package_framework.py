# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import sys

# Packages a framework bundle by setting up symlinks for the "Current" version.
# Usage: python /path/to/Foo.framework current_version

def Main(args):
  if len(args) != 3:
    print >> sys.stderr, "Usage: %s /path/to/Something.framework A", (args[0],)
    return 1

  (framework, version) = args[1:]

  # Find the name of the binary based on the part before the ".framework".
  binary = os.path.splitext(os.path.basename(framework))[0]

  CURRENT = 'Current'
  RESOURCES = 'Resources'
  VERSIONS = 'Versions'

  if not os.path.exists(os.path.join(framework, VERSIONS, version, binary)):
    # Binary-less frameworks don't seem to contain symlinks (see e.g.
    # chromium's out/Debug/org.chromium.Chromium.manifest/ bundle).
    return 0

  # Move into the framework directory to set the symlinks correctly.
  os.chdir(framework)

  # Set up the Current version.
  _Relink(version, os.path.join(VERSIONS, CURRENT))

  # Set up the root symlinks.
  _Relink(os.path.join(VERSIONS, CURRENT, binary), binary)
  _Relink(os.path.join(VERSIONS, CURRENT, RESOURCES), RESOURCES)

  # The following directories are optional but should also be symlinked
  # in the root.
  EXTRA_DIRS = [
      'Helpers',
      'Internet Plug-Ins',
      'Libraries',
      'XPCServices',
  ]
  for extra_dir in EXTRA_DIRS:
    extra_dir_target = os.path.join(VERSIONS, version, extra_dir)
    if os.path.exists(extra_dir_target):
      _Relink(extra_dir_target, extra_dir)

  return 0


def _Relink(dest, link):
  """Creates a symlink to |dest| named |link|. If |link| already exists,
  it is overwritten."""
  if os.path.lexists(link):
    os.remove(link)
  os.symlink(dest, link)


if __name__ == '__main__':
  sys.exit(Main(sys.argv))
