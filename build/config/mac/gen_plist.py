# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import plistlib
import os
import re
import subprocess
import sys
import tempfile
import shlex


# Xcode substitutes variables like ${PRODUCT_NAME} when compiling Info.plist.
# It also supports supports modifiers like :identifier or :rfc1034identifier.
# SUBST_RE matches a variable substitution pattern with an optional modifier,
# while IDENT_RE matches all characters that are not valid in an "identifier"
# value (used when applying the modifier).
SUBST_RE = re.compile(r'\$\{(?P<id>[^}]*?)(?P<modifier>:[^}]*)?\}')
IDENT_RE = re.compile(r'[/\s]')


class ArgumentParser(argparse.ArgumentParser):
  """Subclass of argparse.ArgumentParser to work with GN response files.

  GN response file writes all the arguments on a single line and assumes
  that the python script uses shlext.split() to extract them. Since the
  default ArgumentParser expects a single argument per line, we need to
  provide a subclass to have the correct support for @{{response_file_name}}.
  """

  def convert_arg_line_to_args(self, arg_line):
    return shlex.split(arg_line)


def InterpolateList(values, substitutions):
  """Interpolates variable references into |value| using |substitutions|.

  Inputs:
    values: a list of values
    substitutions: a mapping of variable names to values

  Returns:
    A new list of values with all variables references ${VARIABLE} replaced
    by their value in |substitutions| or None if any of the variable has no
    subsitution.
  """
  result = []
  for value in values:
    interpolated = InterpolateValue(value, substitutions)
    if interpolated is None:
      return None
    result.append(interpolated)
  return result


def InterpolateString(value, substitutions):
  """Interpolates variable references into |value| using |substitutions|.

  Inputs:
    value: a string
    substitutions: a mapping of variable names to values

  Returns:
    A new string with all variables references ${VARIABLES} replaced by their
    value in |substitutions| or None if any of the variable has no substitution.
  """
  result = value
  for match in reversed(list(SUBST_RE.finditer(value))):
    variable = match.group('id')
    if variable not in substitutions:
      return None
    # Some values need to be identifier and thus the variables references may
    # contains :modifier attributes to indicate how they should be converted
    # to identifiers ("identifier" replaces all invalid characters by '-' and
    # "rfc1034identifier" replaces them by "_" to make valid URI too).
    modifier = match.group('modifier')
    if modifier == 'identifier':
      interpolated = IDENT_RE.sub('-', substitutions[variable])
    elif modifier == 'rfc1034identifier':
      interpolated = IDENT_RE.sub('_', substitutions[variable])
    else:
      interpolated = substitutions[variable]
    result = result[:match.start()] + interpolated + result[match.end():]
  return result


def InterpolateValue(value, substitutions):
  """Interpolates variable references into |value| using |substitutions|.

  Inputs:
    value: a value, can be a dictionary, list, string or other
    substitutions: a mapping of variable names to values

  Returns:
    A new value with all variables references ${VARIABLES} replaced by their
    value in |substitutions| or None if any of the variable has no substitution.
  """
  if isinstance(value, dict):
    return Interpolate(value, substitutions)
  if isinstance(value, list):
    return InterpolateList(value, substitutions)
  if isinstance(value, str):
    return InterpolateString(value, substitutions)
  return value


def Interpolate(plist, substitutions):
  """Interpolates variable references into |value| using |substitutions|.

  Inputs:
    plist: a dictionary representing a Property List (.plist) file
    substitutions: a mapping of variable names to values

  Returns:
    A new plist with all variables references ${VARIABLES} replaced by their
    value in |substitutions|. All values that contains references with no
    substitutions will be removed and the corresponding key will be cleared
    from the plist (not recursively).
  """
  result = {}
  for key in plist:
    value = InterpolateValue(plist[key], substitutions)
    if value is not None:
      result[key] = value
  return result


def LoadPList(path):
  """Loads Plist at |path| and returns it as a dictionary."""
  fd, name = tempfile.mkstemp()
  try:
    subprocess.check_call(['plutil', '-convert', 'xml1', '-o', name, path])
    with os.fdopen(fd, 'r') as f:
      return plistlib.readPlist(f)
  finally:
    os.unlink(name)


def SavePList(path, format, data):
  """Saves |data| as a Plist to |path| in the specified |format|."""
  fd, name = tempfile.mkstemp()
  try:
    with os.fdopen(fd, 'w') as f:
      plistlib.writePlist(data, f)
    subprocess.check_call(['plutil', '-convert', format, '-o', path, name])
  finally:
    os.unlink(name)


def MergePList(plist1, plist2):
  """Merges |plist1| with |plist2| recursively.

  Creates a new dictionary representing a Property List (.plist) files by
  merging the two dictionary |plist1| and |plist2| recursively (only for
  dictionary values).

  Args:
    plist1: a dictionary representing a Property List (.plist) file
    plist2: a dictionary representing a Property List (.plist) file

  Returns:
    A new dictionary representing a Property List (.plist) file by merging
    |plist1| with |plist2|. If any value is a dictionary, they are merged
    recursively, otherwise |plist2| value is used.
  """
  if not isinstance(plist1, dict) or not isinstance(plist2, dict):
    if plist2 is not None:
      return plist2
    else:
      return plist1
  result = {}
  for key in set(plist1) | set(plist2):
    if key in plist2:
      value = plist2[key]
    else:
      value = plist1[key]
    if isinstance(value, dict):
      value = MergePList(plist1.get(key, None), plist2.get(key, None))
    result[key] = value
  return result


def main():
  parser = ArgumentParser(
      description='A script to generate iOS application Info.plist.',
      fromfile_prefix_chars='@')
  parser.add_argument('-o', '--output', required=True,
                      help='Path to output plist file.')
  parser.add_argument('-s', '--subst', action='append', default=[],
                      help='Substitution rule in the format "key=value".')
  parser.add_argument('-f', '--format', required=True,
                      help='Plist format (e.g. binary1, xml1) to output.')
  parser.add_argument('path', nargs="+", help='Path to input plist files.')
  args = parser.parse_args()
  substitutions = {}
  for subst in args.subst:
    key, value = subst.split('=', 1)
    substitutions[key] = value
  data = {}
  for filename in args.path:
    data = MergePList(data, LoadPList(filename))
  data = Interpolate(data, substitutions)
  SavePList(args.output, args.format, data)
  return 0

if __name__ == '__main__':
  sys.exit(main())
