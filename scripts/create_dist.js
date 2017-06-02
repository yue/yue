#!/usr/bin/env node

// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {version, targetCpu, targetOs, execSync} = require('./common')

const fs = require('fs')
const JSZip = require('./libs/jszip')

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

const luaFiles = {
  linux: [
    'yue',
    'yue.so',
  ],
  mac: [
    'yue',
    'yue.so',
  ],
  win: [
    'yue.exe',
    'yue.lib',
    'yue.pdb',
  ],
}

generateZip('libyue', cppFiles)
generateZip('lua_yue', luaFiles)

function generateZip(name, list) {
  const zipname = `${name}_${version}_${targetOs}_${targetCpu}`
  let zip = new JSZip()
  for (let file of list[targetOs])
    zip.file(file, fs.readFileSync(`out/Release/${file}`))
  zip.generateNodeStream({streamFiles:true})
     .pipe(fs.createWriteStream(`out/Release/${zipname}.zip`))
}
