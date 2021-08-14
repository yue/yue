#!/usr/bin/env node

// Copyright 2021 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {argv, targetOs, verbose} = require('./common')

const path = require('path')
const fs = require('fs-extra')

const targets = [
  'third_party/modp_b64',
  'third_party/apple_apsl',
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
    const line = lines[i]
    if (!line.startsWith('#include "'))
      continue
    const header = line.match(/#include "(.+)"/)[1]
    if (!header)
      throw new Error(`Unable to parse: ${line}`)
    if (header.startsWith(newBaseDir) || buildHeaders.includes(header))
      continue
    const newHeader = rebaseHeader(header, sourceFile, rootDir, newBaseDir)
    if (!fs.existsSync(`include/${newHeader}`)) {
      if (verbose)
        console.log('Unexisting file:', header)
    }
    if (verbose)
      console.log('Replacing', header, 'with', newHeader)
    hasModification = true
    lines[i] = `#include "${newHeader}"`
  }
  if (hasModification) {
    if (verbose)
      console.log('Writing:', sourceFile)
    if (!dryRun)
      fs.writeFileSync(sourceFile, lines.join('\n'))
  }
}

function rebaseHeader(header, sourceFile, rootDir, newBaseDir) {
  const fullPath = path.resolve(rootDir, path.dirname(sourceFile), header)
  return `${newBaseDir}/${header}`
}
