#!/usr/bin/env node

// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {version, targetCpu, targetOs, execSync} = require('./common')

const fs = require('fs')
const JSZip = require('./libs/jszip')

// C++ static library.
const cppFiles = {
  linux: [
    'libyue.a',
  ],
  mac: [
    'libyue.a',
  ],
  win: [
    'libyue.lib',
  ],
}

// Lua loadable module.
const luaFiles = {
  linux: [
    'yue.so',
  ],
  mac: [
    'yue.so',
  ],
  win: [
  ],
}

// Independent executable.
const exeFiles = {
  linux: [
    'yue'
  ],
  mac: [
    'yue'
  ],
  win: [
    'yue.exe',
    'yue.lib',
    'yue.pdb',
  ],
}

generateZip('libyue', cppFiles)
generateZip('lua_yue_lua_5.3', luaFiles)
generateZip('yue', exeFiles)

function generateZip(name, list) {
  const zipname = `${name}_${version}_${targetOs}_${targetCpu}`
  let zip = new JSZip()
  for (let file of list[targetOs]) {
    if (targetOs == 'linux')
      strip(`out/Release/${file}`)
    zip.file(file, fs.readFileSync(`out/Release/${file}`))
  }
  zip.generateNodeStream({streamFiles:true})
     .pipe(fs.createWriteStream(`out/Release/${zipname}.zip`))
}

function strip(file) {
  if (file.endsWith('.a'))
    return
  const strip = targetCpu.startsWith('arm') ? 'arm-linux-gnueabihf-strip'
                                            : 'strip'
  execSync(`${strip} ${file}`)
}
