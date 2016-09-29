#!/usr/bin/env python

import fnmatch
import os
import subprocess
import sys

SOURCE_ROOT = os.path.abspath(os.path.dirname(os.path.dirname(__file__)))

IGNORE_FILES = [
  os.path.join(SOURCE_ROOT, 'nativeui', 'mac', 'nu_application_delegate.h'),
  os.path.join(SOURCE_ROOT, 'nativeui', 'win', 'util', 'msg_util.h'),
]


def main():
  os.chdir(SOURCE_ROOT)
  files = list_files(['yue', 'lua', 'nativeui'], ['*.cc', '*.h'])
  call_cpplint(list(set(files) - set(IGNORE_FILES)))


def list_files(directories, filters):
  matches = []
  for directory in directories:
    for root, _, filenames, in os.walk(os.path.join(SOURCE_ROOT, directory)):
      for f in filters:
        for filename in fnmatch.filter(filenames, f):
          matches.append(os.path.join(root, filename))
  return matches


def call_cpplint(files):
  cpplint = os.path.join(SOURCE_ROOT, 'tools', 'cpplint.py')
  try:
    output = subprocess.check_output([sys.executable, cpplint] + files,
                                     stderr=subprocess.STDOUT)
  except Exception as e:
    output = e.output
  for line in output.split('\n'):
    if line != '' and not line.startswith('Done processing'):
      print line

if __name__ == '__main__':
  sys.exit(main())
