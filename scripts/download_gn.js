#!/usr/bin/env node

// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const fs = require('fs')
const path = require('path')
const extract = require('./libs/extract-zip')

const {targetOs, download} = require('./common')

const version = 'v0.2.7'
const url = `https://github.com/yue/build-gn/releases/download/${version}/gn_${version}_${targetOs}_x64.zip`

const gnDir = path.resolve('building', 'tools', 'gn')
const verFile = path.join(gnDir, '.version')
if (fs.existsSync(verFile) && fs.readFileSync(verFile) == version)
  return

download(url, (response) => {
  response.on('end', () => {
    extract('gn.zip', {dir: gnDir}, () => {
      fs.writeFileSync(verFile, version)
      fs.unlinkSync('gn.zip')
    })
  })
  response.pipe(fs.createWriteStream('gn.zip'))
})
