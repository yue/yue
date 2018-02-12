#!/usr/bin/env node

// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {argv, download, mkdir} = require('./common')

const fs      = require('fs')
const path    = require('path')
const extract = require('./libs/extract-zip')

if (argv.length != 2) {
  console.error('Usage: download_lua_libs version cpu')
  process.exit(1)
}

const version = argv[0]
const targetCpu = argv[1]

const arch = targetCpu == 'x64' ? 'Win64' : 'Win32'
const lua_dir = path.resolve('third_party', `lua_binaries_${version}_${arch}`)
if (fs.existsSync(lua_dir)) {
  process.exit(0)
}

const mirror = 'https://jaist.dl.sourceforge.net/project/luabinaries'
const folder = 'Windows%20Libraries/Static'
const zipname = `lua-${version}_${arch}_vc14_lib.zip`
const url = `${mirror}/${version}/${folder}/${zipname}`

download(url, (response) => {
  mkdir(lua_dir)
  const zippath = path.join(lua_dir, zipname)
  response.on('end', () => {
    extract(zippath, {dir: lua_dir}, (error) => {
      fs.unlinkSync(zippath)
    })
  })
  response.pipe(fs.createWriteStream(zippath))
})
