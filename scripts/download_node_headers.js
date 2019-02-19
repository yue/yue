#!/usr/bin/env node

// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const cp    = require('child_process')
const fs    = require('fs')
const path  = require('path')
const zlib  = require('zlib')

const {argv, download} = require('./common')

if (argv.length != 3) {
  console.error('Usage: download_node_headers runtime version cpu')
  process.exit(1)
}

const runtime = argv[0]
const version = argv[1].startsWith('v') ? argv[1] : `v${argv[1]}`
const targetCpu = argv[2]

const prefix = {
  electron: 'https://gh-contractor-zcbenz.s3.amazonaws.com/atom-shell/dist',
  node: 'https://nodejs.org/dist',
}

if (!(runtime in prefix)) {
  console.error(`Unkown runtime: ${runtime}`)
  process.exit(2)
}

const node_dir = path.join('third_party', `node-${version}`)
const lib_dir = path.join(node_dir, targetCpu)
if (fs.existsSync(process.platform === 'win32' ? lib_dir : node_dir)) {
  process.exit(0)
}

// Expose tar to PATH.
if (process.platform == 'win32') {
  const bindir = path.resolve('building', 'tools', 'win')
  process.env.PATH = `${bindir}${path.delimiter}${process.env.PATH}`
}

const suffix = runtime == 'electron' ? '' : '-headers'
const url = `${prefix[runtime]}/${version}/node-${version}${suffix}.tar.gz`
download(url, (response) => {
  response.pipe(zlib.createGunzip())
          .pipe(cp.exec(`tar xf - node-${version}`, {cwd: 'third_party'}).stdin)

  // Download node.lib on Windows.
  if (process.platform == 'win32') {
    response.on('end', () => {
      if (targetCpu == 'x64')
        downloadNodeLib('x64')
      else if (targetCpu == 'x86')
        downloadNodeLib('x86')
      else
        throw new Error(`Unsupported targetCpu: ${targetCpu}`)
    })
  }
})

function downloadNodeLib(arch) {
  const name = runtime == 'electron' ? 'iojs' : 'node'
  const lib = `${prefix[runtime]}/${version}/win-${arch}/${name}.lib`
  download(lib, (response) => {
    fs.mkdirSync(lib_dir)
    response.pipe(fs.createWriteStream(path.join(lib_dir, 'node.lib')))
  })
}
