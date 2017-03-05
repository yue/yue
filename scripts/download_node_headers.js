#!/usr/bin/env node

// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const cp    = require('child_process')
const fs    = require('fs')
const https = require('https')
const zlib  = require('zlib')

const {argv} = require('./common')

if (argv.length != 2) {
  console.error('Usage: download_node_headers runtime version')
  process.exit(1)
}

const runtime = argv[0]
const version = argv[1]

const prefix = {
  electron: 'https://atom.io/download/electron',
  node: 'https://nodejs.org/dist',
}

if (!(runtime in prefix)) {
  console.error(`Unkown runtime: ${runtime}`)
  process.exit(2)
}

if (fs.existsSync(`third_party/node-${version}`)) {
  process.exit(0)
}

const url = `${prefix[runtime]}/${version}/node-${version}-headers.tar.gz`
https.get(url, (response) => {
  response.pipe(zlib.createGunzip())
          .pipe(cp.exec('tar x', {cwd: 'third_party'}).stdin)
})
