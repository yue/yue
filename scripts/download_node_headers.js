#!/usr/bin/env node

// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {argv, streamPromise} = require('./common')

const path = require('path')
const tar = require('tar')
const fs = require('fs-extra')
const fetch = require('node-fetch')
const useTmpDir = require('use-tmp-dir')

if (argv.length != 4) {
  console.error('Usage: download_node_headers runtime version os cpu')
  process.exit(1)
}

const runtime = argv[0]
const version = argv[1].startsWith('v') ? argv[1] : `v${argv[1]}`
const targetOs = argv[2]
const targetCpu = argv[3]

const prefix = {
  electron: 'https://electronjs.org/headers',
  node: 'https://nodejs.org/dist',
}

if (!(runtime in prefix)) {
  console.error(`Unkown runtime: ${runtime}`)
  process.exit(2)
}

const nodeDir = path.join('third_party', `${runtime}-${version}`)
const libDir = path.join(nodeDir, targetCpu)
if (fs.existsSync(targetOs == 'win' ? libDir : nodeDir)) {
  process.exit(0)
}
fs.emptyDirSync(nodeDir)

useTmpDir(async (cwd) => {
  const url = `${prefix[runtime]}/${version}/node-${version}-headers.tar.gz`
  const file = path.join(cwd, 'node_headers.tar.gz')
  const res = await fetch(url)
  await streamPromise(res.body.pipe(fs.createWriteStream(file)))
  await tar.x({file, cwd})

  const subdir = runtime == 'electron' ? 'node_headers' : `node-${version}`
  await fs.move(path.join(cwd, subdir, 'include'),
                path.join(nodeDir, 'include'))

  // Download node.lib on Windows.
  if (targetOs == 'win') {
    if (targetCpu == 'x64')
      await downloadNodeLib('x64')
    else if (targetCpu == 'x86')
      await downloadNodeLib('x86')
    else if (targetCpu == 'arm64')
      await downloadNodeLib('arm64')
    else
      throw new Error(`Unsupported targetCpu: ${targetCpu}`)
  }
})

async function downloadNodeLib(arch) {
  await fs.emptyDir(libDir)
  const url = `${prefix[runtime]}/${version}/win-${arch}/node.lib`
  const res = await fetch(url)
  res.body.pipe(fs.createWriteStream(path.join(libDir, 'node.lib')))
  return streamPromise(res.body)
}
