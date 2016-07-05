# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import fnmatch
import glob
import os
import plistlib
import shutil
import subprocess
import sys
import tempfile


class InstallationError(Exception):
  """Signals a local installation error that prevents code signing."""

  def __init__(self, fmt, *args):
    super(Exception, self).__init__(fmt % args)


def GetProvisioningProfilesDir():
  """Returns the location of the installed mobile provisioning profiles.

  Returns:
    The path to the directory containing the installed mobile provisioning
    profiles as a string.
  """
  return os.path.join(
      os.environ['HOME'], 'Library', 'MobileDevice', 'Provisioning Profiles')


def LoadPlistFile(plist_path):
  """Loads property list file at |plist_path|.

  Args:
    plist_path: path to the property list file to load.

  Returns:
    The content of the property list file as a python object.
  """
  return plistlib.readPlistFromString(subprocess.check_output([
      'xcrun', 'plutil', '-convert', 'xml1', '-o', '-', plist_path]))


class Bundle(object):
  """Wraps a bundle."""

  def __init__(self, bundle_path):
    """Initializes the Bundle object with data from bundle Info.plist file."""
    self._path = bundle_path
    self._data = LoadPlistFile(os.path.join(self._path, 'Info.plist'))

  @property
  def path(self):
    return self._path

  @property
  def identifier(self):
    return self._data['CFBundleIdentifier']

  @property
  def binary_path(self):
    return os.path.join(self._path, self._data['CFBundleExecutable'])


class ProvisioningProfile(object):
  """Wraps a mobile provisioning profile file."""

  def __init__(self, provisioning_profile_path):
    """Initializes the ProvisioningProfile with data from profile file."""
    self._path = provisioning_profile_path
    self._data = plistlib.readPlistFromString(subprocess.check_output([
        'xcrun', 'security', 'cms', '-D', '-i', provisioning_profile_path]))

  @property
  def path(self):
    return self._path

  @property
  def application_identifier_pattern(self):
    return self._data.get('Entitlements', {}).get('application-identifier', '')

  @property
  def team_identifier(self):
    return self._data.get('TeamIdentifier', [''])[0]

  @property
  def entitlements(self):
    return self._data.get('Entitlements', {})

  def ValidToSignBundle(self, bundle):
    """Checks whether the provisioning profile can sign bundle_identifier.

    Args:
      bundle: the Bundle object that needs to be signed.

    Returns:
      True if the mobile provisioning profile can be used to sign a bundle
      with the corresponding bundle_identifier, False otherwise.
    """
    return fnmatch.fnmatch(
        '%s.%s' % (self.team_identifier, bundle.identifier),
        self.application_identifier_pattern)

  def Install(self, bundle):
    """Copies mobile provisioning profile info the bundle."""
    installation_path = os.path.join(bundle.path, 'embedded.mobileprovision')
    shutil.copy2(self.path, installation_path)


class Entitlements(object):
  """Wraps an Entitlement plist file."""

  def __init__(self, entitlements_path):
    """Initializes Entitlements object from entitlement file."""
    self._path = entitlements_path
    self._data = LoadPlistFile(self._path)

  @property
  def path(self):
    return self._path

  def ExpandVariables(self, substitutions):
    self._data = self._ExpandVariables(self._data, substitutions)

  def _ExpandVariables(self, data, substitutions):
    if isinstance(data, str):
      for key, substitution in substitutions.iteritems():
        data = data.replace('$(%s)' % (key,), substitution)
      return data

    if isinstance(data, dict):
      for key, value in data.iteritems():
        data[key] = self._ExpandVariables(value, substitutions)
      return data

    if isinstance(data, list):
      for i, value in enumerate(data):
        data[i] = self._ExpandVariables(value, substitutions)

    return data

  def LoadDefaults(self, defaults):
    for key, value in defaults.iteritems():
      if key not in self._data:
        self._data[key] = value

  def WriteTo(self, target_path):
    plistlib.writePlist(self._data, target_path)


def FindProvisioningProfile(bundle, provisioning_profile_short_name):
  """Finds mobile provisioning profile to use to sign bundle.

  Args:
    bundle: the Bundle object to sign.
    provisioning_profile_short_path: optional short name of the mobile
        provisioning profile file to use to sign (will still be checked
        to see if it can sign bundle).

  Returns:
    The ProvisioningProfile object that can be used to sign the Bundle
    object.

  Raises:
    InstallationError if no mobile provisioning profile can be used to
    sign the Bundle object.
  """
  provisioning_profiles_dir = GetProvisioningProfilesDir()

  # First check if there is a mobile provisioning profile installed with
  # the requested short name. If this is the case, restrict the search to
  # that mobile provisioning profile, otherwise consider all the installed
  # mobile provisioning profiles.
  provisioning_profile_paths = []
  if provisioning_profile_short_name:
    provisioning_profile_path = os.path.join(
        provisioning_profiles_dir,
        provisioning_profile_short_name + '.mobileprovision')
    if os.path.isfile(provisioning_profile_path):
      provisioning_profile_paths.append(provisioning_profile_path)

  if not provisioning_profile_paths:
    provisioning_profile_paths = glob.glob(
        os.path.join(provisioning_profiles_dir, '*.mobileprovision'))

  # Iterate over all installed mobile provisioning profiles and filter those
  # that can be used to sign the bundle.
  valid_provisioning_profiles = []
  for provisioning_profile_path in provisioning_profile_paths:
    provisioning_profile = ProvisioningProfile(provisioning_profile_path)
    if provisioning_profile.ValidToSignBundle(bundle):
      valid_provisioning_profiles.append(provisioning_profile)

  if not valid_provisioning_profiles:
    raise InstallationError(
        'no mobile provisioning profile for "%s"',
        bundle.identifier)

  # Select the most specific mobile provisioning profile, i.e. the one with
  # the longest application identifier pattern.
  return max(
      valid_provisioning_profiles,
      key=lambda p: len(p.application_identifier_pattern))


def CodeSignBundle(binary, bundle, args):
  """Cryptographically signs bundle.

  Args:
    bundle: the Bundle object to sign.
    args: a dictionary with configuration settings for the code signature,
        need to define 'entitlements_path', 'provisioning_profile_short_name',
        'deep_signature' and 'identify' keys.
  """
  provisioning_profile = FindProvisioningProfile(
      bundle, args.provisioning_profile_short_name)
  provisioning_profile.Install(bundle)

  signature_file = os.path.join(bundle.path, '_CodeSignature', 'CodeResources')
  if os.path.isfile(signature_file):
    os.unlink(signature_file)

  shutil.copy(binary, bundle.binary_path)

  if args.preserve:
    subprocess.check_call([
        'xcrun', 'codesign', '--force', '--sign', args.identity,
        '--deep', '--preserve-metadata=identifier,entitlements',
        '--timestamp=none', bundle.path])
  else:
    entitlements = Entitlements(args.entitlements_path)
    entitlements.LoadDefaults(provisioning_profile.entitlements)
    entitlements.ExpandVariables({
      'CFBundleIdentifier': bundle.identifier,
      'AppIdentifierPrefix': '%s.' % (provisioning_profile.team_identifier,)
    })

    with tempfile.NamedTemporaryFile(suffix='.xcent') as temporary_file_path:
      entitlements.WriteTo(temporary_file_path.name)
      subprocess.check_call([
          'xcrun', 'codesign', '--force', '--sign', args.identity,
          '--entitlements', temporary_file_path.name, '--timestamp=none',
          bundle.path])


def Main():
  parser = argparse.ArgumentParser('codesign iOS bundles')
  parser.add_argument(
      'path', help='path to the iOS bundle to codesign')
  parser.add_argument(
      '--binary', '-b', required=True,
      help='path to the iOS bundle binary')
  parser.add_argument(
      '--provisioning-profile', '-p', dest='provisioning_profile_short_name',
      help='short name of the mobile provisioning profile to use ('
           'if undefined, will autodetect the mobile provisioning '
           'to use)')
  parser.add_argument(
      '--identity', '-i', required=True,
      help='identity to use to codesign')
  group = parser.add_mutually_exclusive_group(required=True)
  group.add_argument(
      '--entitlements', '-e', dest='entitlements_path',
      help='path to the entitlements file to use')
  group.add_argument(
      '--deep', '-d', action='store_true', default=False, dest='preserve',
      help='deep signature (default: %(default)s)')
  args = parser.parse_args()

  CodeSignBundle(args.binary, Bundle(args.path), args)


if __name__ == '__main__':
  sys.exit(Main())
