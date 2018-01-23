#!/usr/bin/env node

// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const cp    = require('child_process')
const fs    = require('fs')
const path  = require('path')
const https = require('https')
const zlib  = require('zlib')

const {argv} = require('./common')

if (argv.length != 3) {
  console.error('Usage: download_node_headers runtime version cpu')
  process.exit(1)
}

const runtime = argv[0]
const version = argv[1]
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
if (fs.existsSync(node_dir)) {
  process.exit(0)
}

const suffix = runtime == 'electron' ? '' : '-headers'
const url = `${prefix[runtime]}/${version}/node-${version}${suffix}.tar.gz`
download(url, (response) => {
  response.pipe(zlib.createGunzip())
          .pipe(cp.exec('tar xf -', {cwd: 'third_party'}).stdin)

  // Download node.lib on Windows.
  if (process.platform == 'win32') {
    response.on('end', () => {
      if (targetCpu == 'x64')
        downloadNodeLib('x64', () => {})
      else if (targetCpu == 'x86')
        downloadNodeLib('x86', () => {})
      else
        throw new Error(`Unsupported targetCpu: ${targetCpu}`)
    })
  }
})

function download(url, callback) {
  https.get(url, (response) => {
    process.stdout.write(`Downloading ${url} `)
    let length = 0
    response.on('end', () => {
      if (length > 0)
        process.stdout.write('.')
      console.log(' Done')
    })
    .on('data', (chunk) => {
      length += chunk.length
      while (length >= 1024 * 1024) {
        process.stdout.write('.')
        length %= 1024 * 1024
      }
    })
    callback(response)
  })
}

function downloadNodeLib(arch, callback) {
  const name = runtime == 'electron' ? 'iojs' : 'node'
  const lib = `${prefix[runtime]}/${version}/win-${arch}/${name}.lib`
  download(lib, (response) => {
    const lib_dir = path.join(node_dir, arch)
    fs.mkdirSync(lib_dir)
    response.on('end', callback)
            .pipe(fs.createWriteStream(path.join(lib_dir, 'node.lib')))
  })
}
