#!/usr/bin/env node

// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {version, targetCpu, targetOs, strip, execSync} = require('./common')
const {createZip} = require('./zip_utils')

const path = require('path')
const fs = require('fs-extra')

// Clear previous distributions.
fs.removeSync('out/Dist')
fs.ensureDirSync('out/Dist')

// Zip lua modules.
const luaFiles = {
  linux: 'yue.so',
  mac: 'yue.so',
  win: 'yue.dll',
}
if (targetOs == 'linux')
  strip(`out/Release/${luaFiles[targetOs]}`)
createZip({withLicense: true})
  .addFile(`out/Release/${luaFiles[targetOs]}`, 'out/Release')
  .writeToFile(`lua_yue_lua_5.3_${version}_${targetOs}_${targetCpu}`)

// Zip sources, but do it for x64 on CI.
if (process.env.CI != 'true' || targetCpu == 'x64')
  execSync('node ./scripts/create_source_dist.js')

// Zip docs, but only do it for linux/x64 when running on CI, to avoid uploading
// docs for multiple times.
if (process.env.CI != 'true' || (targetOs == 'linux' && targetCpu == 'x64')) {
  execSync('node ./scripts/create_docs.js')
  createZip()
    .addFile('out/Dist/docs', 'out/Dist/docs')
    .writeToFile(`yue_docs_${version}`)
  execSync('node ./scripts/create_typescript_declarations.js')
  createZip()
    .addFile('out/Dist/index.d.ts', 'out/Dist')
    .writeToFile(`node_yue_types_${version}`)
}
