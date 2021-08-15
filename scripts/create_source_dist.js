#!/usr/bin/env node

// Copyright 2020 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {version, targetOs, searchFiles, execSync, spawnSync} = require('./common')
const {gnConfig} = require('./config')
const {createZip} = require('./zip_utils')

const path = require('path')
const fs = require('fs-extra')

// Do a jumbo build to prepare for searching files.
const args = gnConfig.concat([
  'use_jumbo_build=true',
  'is_component_build=false',
  'is_debug=false',
])
fs.emptyDirSync('out/Source')
process.env.JUMBO_INCLUDE_FILE_CONTENTS = 'true'
spawnSync('gn', ['gen', 'out/Source', `--args=${args.join(' ')}`])
spawnSync('ninja', ['-C', 'out/Source', 'nativeui'])

// Search files.
const sources = new Set()
const headers = new Set()
DescribeAll('nativeui', sources, headers)

// Copy source files out.
const targetDir = 'out/Dist/source'
fs.emptyDirSync(targetDir)
for (const file of sources)
  CopySource(file, path.join(targetDir, 'src', targetOs))
for (const file of headers)
  CopySource(file, path.join(targetDir, 'include'))

// Copy some extra files.
const EXTRA_HEADERS = {
  'building/tools/gn': [
    'build',
    'testing/gtest',
    'third_party/googletest/src/googletest/include'
  ],
}
if (targetOs === 'mac') {
  EXTRA_HEADERS[''] = [ 'third_party/apple_apsl' ]
} else if (targetOs === 'win') {
  CopySource('//base/win/windows_defines.inc', path.join(targetDir, 'include'))
  CopySource('//base/win/windows_undefines.inc', path.join(targetDir, 'include'))
  // This file is needed by Windows, but marked as posix.
  CopySource('//base/posix/eintr_wrapper.h', path.join(targetDir, 'include'))
}
for (const parent in EXTRA_HEADERS) {
  for (const dir of EXTRA_HEADERS[parent]) {
    const headers = searchFiles(path.join(parent, dir), '.h')
    for (const h of headers)
      CopySource('//' + h, path.join(targetDir, 'include'), parent)
  }
}

execSync('node scripts/modify_base_includes.js')

createZip({withLicense: true})
  .addFile('out/Dist/source', 'out/Dist/source')
  .addFile('sample_app/CMakeLists.txt', 'sample_app')
  .addFile('sample_app/main.cc')
  .addFile('sample_app/exe.manifest')
  .writeToFile(`libyue_${version}_${targetOs}`)

function DescribeAll(target, sources, headers) {
  const deps = new Set()
  const described = new Set()
  deps.add(target)
  while (deps.size > 0) {
    const target = Array.from(deps)[0]
    deps.delete(target)
    if (described.has(target))
      continue
    else
      described.add(target)

    const subsources = new Set()
    Descibe(target, subsources, deps)
    MergeSources(sources, headers, subsources)
  }
}

function Descibe(target, sources, deps) {
  const ret = execSync(`gn desc --format=json out/Source ${target}`, {stdio: 'pipe'})
  const desc = Object.values(JSON.parse(String(ret)))[0]
  if (desc.sources) {
    for (const s of desc.sources)
      sources.add(s)
  }
  if (desc.public) {
    for (const p of desc.public) {
      if (IsHeaderFile(p))
        sources.add(p)
    }
  }
  if (desc.deps) {
    for (const d of desc.deps)
      deps.add(d)
  }
  if (desc.type === 'action') {
    for (const o of desc.outputs) {
      if (IsHeaderFile(o))
        sources.add(o)
    }
  }
  return sources
}

function MergeSources(sources, headers, from) {
  for (const f of from) {
    if (IsSourceFile(f))
      sources.add(f)
    else
      headers.add(f)
  }
}

function CopySource(file, targetDir, baseDir = '') {
  let originalName = file
  let targetName
  file = file.replace(/\\/g, '/')
  if (file.startsWith('//')) {
    // When file starts with '//', it is relative path.
    targetName = file = file.substr(2)
  } else {
    // The absolute path on Windows may looks like '/c:/cygiwn/home'.
    if (process.platform == 'win32' && /^\/\w:/.test(file))
      file = file.substr(1)
    // the paths must be relative.
    targetName = file = path.relative(process.cwd(), file)
  }
  targetName = targetName.substr(baseDir.length)
  // Move generated headers out of dirs.
  if (file.startsWith('building/tools/gn/'))
    targetName = path.join.apply(path, path.normalize(file).split(path.sep).slice(3))
  else if (file.startsWith('out/Source/gen/building/tools/gn/'))
    targetName = path.join.apply(path, path.normalize(file).split(path.sep).slice(6))
  else if (file.startsWith('out/Source/gen/'))
    targetName = path.join.apply(path, path.normalize(file).split(path.sep).slice(3))
  // Rename cpp files to cc.
  if (targetName.endsWith('.cpp'))
    targetName = targetName.substr(0, targetName.length - 3) + 'cc'
  try {
    fs.copySync(file, path.join(targetDir, targetName),
                {errorOnExist: true})
  } catch (e) {
    throw new Error(`Unable to copy file ${originalName}: ${e.message}`)
  }
}

function IsHeaderFile(f) {
  return ['.h', '.inc'].includes(path.extname(f))
}

function IsSourceFile(f) {
  return ['.cc', '.c', '.cpp', '.mm', '.S'].includes(path.extname(f))
}
