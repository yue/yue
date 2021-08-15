#!/usr/bin/env node

// Copyright 2021 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {argv, targetOs, verbose} = require('./common')

const path = require('path')
const fs = require('fs-extra')

const targets = [
  'third_party/abseil-cpp',
  'third_party/apple_apsl',
  'third_party/modp_b64',
  'third_party/yoga',
  'base/third_party/double_conversion/double-conversion',
  'base/third_party/libevent',
  'base/third_party/symbolize',
  'base/third_party/xdg_mime',
  'base/third_party/xdg_user_dirs',
]
const buildHeaders = [
  'build/build_config.h',
]

const dryRun = argv.includes('--dry-run')

process.chdir('out/Dist/source')
for (const target of targets) {
  for (const rootDir of [`src/${targetOs}`, 'include']) {
    const sourceDir = `${rootDir}/${target}`
    if (fs.existsSync(sourceDir))
      modifyIncludesForDir(sourceDir, rootDir, target)
  }
}

function modifyIncludesForDir(sourceDir, rootDir, newBaseDir) {
  const files = fs.readdirSync(sourceDir)
  for (const file of files) {
    const p = `${sourceDir}/${file}`
    const stats = fs.statSync(p)
    if (stats.isDirectory())
      modifyIncludesForDir(p, rootDir, newBaseDir)
    else
      modifyIncludesForFile(p, rootDir, newBaseDir)
  }
}

function modifyIncludesForFile(sourceFile, rootDir, newBaseDir) {
  let hasModification = false
  const lines = fs.readFileSync(sourceFile).toString().split('\n')
  for (const i in lines) {
    const result = parseHeader(lines[i])
    if (!result)
      continue
    const [header, prefix, suffix] = result
    if (header.startsWith(newBaseDir) || buildHeaders.includes(header))
      continue
    const newHeader = rebaseHeader(header, sourceFile, rootDir, newBaseDir)
    if (verbose)
      console.log('Replacing', header, 'with', newHeader)
    hasModification = true
    lines[i] = prefix + newHeader + suffix
  }
  if (hasModification) {
    if (verbose)
      console.log('Writing:', sourceFile)
    if (!dryRun)
      fs.writeFileSync(sourceFile, lines.join('\n'))
  }
}

function parseHeader(line) {
  let match = line.match(/^#include "(.+)"/)
  if (match)
    return [match[1], '#include "', '"']
  // A special case for abseil.
  match = line.match(/^ +"(.+.inc)"/)
  if (match)
    return [match[1], '    "', '"']
  return null
}

function rebaseHeader(header, sourceFile, rootDir, newBaseDir) {
  const newHeader = `${newBaseDir}/${header}`
  if (fs.existsSync(`include/${newHeader}`))
    return newHeader
  const relativeSourceFile =
      path.relative(path.join(rootDir, newBaseDir), sourceFile)
  const relativeHeader =
      path.join(newBaseDir, path.dirname(relativeSourceFile), header)
  if (fs.existsSync(`include/${relativeHeader}`))
    return relativeHeader.replace(/\\/g, '/')
  if (verbose)
    console.log('Unexisting header:', header, 'from', sourceFile)
  return newHeader
}
