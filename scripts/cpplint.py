#!/usr/bin/env python

import fnmatch
import os
import subprocess
import sys

IGNORE_FILES = [
]

SOURCE_ROOT = os.path.abspath(os.path.dirname(os.path.dirname(__file__)))


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
  subprocess.check_call([sys.executable, cpplint] + files)


if __name__ == '__main__':
  sys.exit(main())
