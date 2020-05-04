#!/usr/bin/env node

// Copyright 2020 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {targetCpu, targetOs, searchFiles, execSync, spawnSync} = require('./common')

const path = require('path')
const fs = require('./libs/fs-extra')

// Do a jumbo build to prepare for searching files.
const args = [
  'is_component_build=false',
  'is_debug=false',
  `target_cpu="${targetCpu}"`,
  'use_jumbo_build=true',
]
fs.removeSync('out/Source')
process.env.JUMBO_INCLUDE_FILE_CONTENTS = 'true'
spawnSync('gn', ['gen', 'out/Source', `--args=${args.join(' ')}`])
spawnSync('ninja', ['-C', 'out/Source', 'nativeui'])

// Search files.
const sources = new Set()
const headers = new Set()
DescribeAll('nativeui', sources, headers)

// Copy source files out.
const targetDir = 'out/Dist/source'
fs.ensureDirSync(targetDir)
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
  EXTRA_HEADERS['out/Source/gen'] = [ 'base/trace_event/etw_manifest' ]
  // This file is needed by Windows, but marked as posix.
  CopySource('base/posix/eintr_wrapper.h', path.join(targetDir, 'include'))
}
for (const parent in EXTRA_HEADERS) {
  for (const dir of EXTRA_HEADERS[parent]) {
    const headers = searchFiles(path.join(parent, dir), '.h')
    for (const h of headers)
      CopySource(h, path.join(targetDir, 'include'), parent)
  }
}

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
      sources.add(s.substr(2))
  }
  if (desc.public) {
    for (const p of desc.public) {
      if (p.endsWith('.h'))
        sources.add(p.substr(2))
    }
  }
  if (desc.deps) {
    for (const d of desc.deps)
      deps.add(d)
  }
  if (desc.type === 'action') {
    for (const o of desc.outputs) {
      if (o.endsWith('.h'))
        sources.add(o.substr(2))
    }
  }
  return sources
}

function MergeSources(sources, headers, from) {
  let isThirdParty = false
  let isHeaderOnly = true
  for (const f of from) {
    if (IsSourceFile(f)) {
      isHeaderOnly = false
      if (path.normalize(f).split(path.sep).includes('third_party'))
        isThirdParty = true
    }
  }
  for (const f of from) {
    if (isThirdParty) {
      // When the target is from third party, we have to put sources and headers
      // together since it may search for headers from the source dir.
      if (!isHeaderOnly)
        sources.add(f)
    } else {
      // Otherwise we can safely separate headers from sources.
      if (IsSourceFile(f))
        sources.add(f)
      else
        headers.add(f)
    }
  }
}

function CopySource(file, targetDir, baseDir = '') {
  let targetName = file.substr(baseDir.length)
  if (file.startsWith('out'))
    targetName = path.join.apply(path, path.normalize(file).split(path.sep).slice(3))
  if (targetName.endsWith('.cpp'))
    targetName = targetName.substr(0, targetName.length - 3) + 'cc'
  fs.copySync(file, path.join(targetDir, targetName),
              {errorOnExist: true})
}

function IsSourceFile(f) {
  return ['.cc', '.c', '.cpp', '.mm', '.S'].includes(path.extname(f))
}
