#!/usr/bin/env node

// Copyright 2021 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {argv, execSync, spawnSync, streamPromise} = require('./common')

if (argv.length < 2) {
  console.error('Usage: test_node_extension runtime version [outDir]')
  process.exit(1)
}

const fs = require('fs')
const path = require('path')
const extract = require('extract-zip')
const fetch = require('node-fetch')
const tar = require('tar')
const useTmpDir = require('use-tmp-dir')

const runtime = argv[0]
const version = argv[1].startsWith('v') ? argv[1] : `v${argv[1]}`
const outDir = argv[2] ? argv[2] : 'out/Component'
const platform = argv[3] ? argv[3] : process.platform

useTmpDir(async (tmpDir) => {
  if (runtime == 'electron')
    await testElectron(tmpDir)
  else if (runtime == 'node')
    await testNode(tmpDir)
  else
    throw new Error(`Unrecognized runtime ${runtime}`)
}).catch((error) => {
  // On Windows it may fail to remove dir due to Electron's process not exiting.
  if (error.code == 'ENOTEMPTY')
    console.error(error)
  else
    throw error
})

function testElectron(cwd) {
  execSync(`npm install electron@${version}`, {cwd})
  const electron = require(path.join(cwd, 'node_modules', 'electron'))
  process.exit(spawnSync(electron, ['napi_yue/test', outDir]).status)
}

async function testNode(cwd) {
  const ext = platform == 'win32' ? 'zip' : 'tar.gz'
  const filename = `node-${version}-${platform == 'win32' ? 'win' : platform}-${process.arch}`
  const res = await fetch(`https://nodejs.org/dist/${version}/${filename}.${ext}`)
  const filepath = path.join(cwd, `${filename}.${ext}`)
  await streamPromise(res.body.pipe(fs.createWriteStream(filepath)))
  let node
  if (platform == 'win32') {
    await extract(filepath, {dir: cwd})
    node = path.join(cwd, filename, 'node.exe')
  } else {
    await tar.x({file: filepath, cwd})
    node = path.join(cwd, filename, 'bin', 'node')
  }
  process.exit(spawnSync(node, ['--expose-gc', 'napi_yue/test', outDir, '--run-gc-tests']).status)
}
