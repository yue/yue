#!/usr/bin/env node

// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const cp    = require('child_process')
const https = require('https')
const zlib  = require('zlib')

require('./common')

if (process.argv.length != 4) {
  console.error('Usage: download-node-headers node|electron VERSION')
  process.exit(1)
}

const runtime = process.argv[2]
const version = process.argv[3]

const prefix = {
  electron: 'https://atom.io/download/electron',
  node: 'https://nodejs.org/dist',
}

if (!(runtime in prefix)) {
  console.error(`Unkown runtime: ${runtime}`)
  process.exit(2)
}

const url = `${prefix[runtime]}/${version}/node-${version}-headers.tar.gz`
https.get(url, (response) => {
  response.pipe(zlib.createGunzip())
          .pipe(cp.exec('tar x', {cwd: 'third_party'}).stdin)
})
