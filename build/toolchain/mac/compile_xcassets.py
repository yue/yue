# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import os
import sys


def Main():
  parser = argparse.ArgumentParser(
      description='compile assets catalog for a bundle')
  parser.add_argument(
      '--platform', '-p', required=True,
      choices=('macosx', 'iphoneos', 'iphonesimulator'),
      help='target platform for the compiled assets catalog')
  parser.add_argument(
      '--minimum-deployment-target', '-t', required=True,
      help='minimum deployment target for the compiled assets catalog')
  parser.add_argument(
      '--output', '-o', required=True,
      help='path to the compiled assets catalog')
  parser.add_argument(
      'inputs', nargs='+',
      help='path to input assets catalog sources')
  args = parser.parse_args()

  if os.path.basename(args.output) != 'Assets.car':
    sys.stderr.write(
        'output should be path to compiled asset catalog, not '
        'to the containing bundle: %s\n' % (args.output,))

  command = [
      'xcrun', 'actool', '--output-format', 'human-readable-text',
      '--compress-pngs', '--notices', '--warnings', '--errors',
      '--platform', args.platform, '--minimum-deployment-target',
      args.minimum_deployment_target,
  ]

  if args.platform == 'macosx':
    command.extend(['--target-device', 'mac'])
  else:
    command.extend(['--target-device', 'iphone', '--target-device', 'ipad'])

  # actool crashes if paths are relative, so use os.path.abspath to get absolute
  # path for input and outputs.
  command.extend(['--compile', os.path.abspath(os.path.dirname(args.output))])
  command.extend(map(os.path.abspath, args.inputs))

  os.execvp('xcrun', command)
  sys.exit(1)

if __name__ == '__main__':
  sys.exit(Main())
