#!/usr/bin/env node

// Copyright 2021 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {version, clang, argv, targetCpu, targetOs, hostCpu, strip, execSync, spawnSync} = require('./common')
const {gnConfig, gnSysrootConfig} = require('./config')
const {createZip} = require('./zip_utils')

const path = require('path')
const fs = require('fs-extra')

if (argv.length != 2) {
  console.error('Usage: create_lua_extension runtime luaver')
  process.exit(1)
}

const runtime = argv[0]
let luaver = argv[1]
if (runtime == 'lua')
  luaver = luaver.startsWith('v') ? luaver : `v${luaver}`
let shortver = luaver
if (runtime == 'luajit')
  shortver = shortver.substr(0, 7)

const args = gnConfig.concat([
  'use_jumbo_build=true',
  `lua_version="${shortver}"`,
  `lua_runtime="${runtime}"`,
  // Same with Release build.
  'is_component_build=false',
  'is_debug=false',
  'is_official_build=true',
])
if (targetOs == 'linux')
  args.push(...gnSysrootConfig)

console.log(`Creating native extension for ${runtime} ${shortver} ${targetCpu}...`)

// Build the module.
execSync(`node ./scripts/download_lua_sources.js ${runtime} ${luaver}`)
spawnSync('gn', ['gen', 'out/Lua', `--args=${args.join(' ')}`])
execSync('ninja -C out/Lua lua_yue')

// Strip the binaries on Linux.
if (targetOs == 'linux')
  strip('out/Lua/yue.so')

// Run smoke tests.
if (hostCpu == targetCpu) {
  execSync('ninja -C out/Lua lua lua_unittests_module')
  const lua = path.join('out', 'Lua', 'lua')
  execSync(`${lua} lua/test/smoke.lua out/Lua`)
  execSync(`${lua} lua_yue/test/smoke.lua out/Lua`)
}

// Generate a name conforming lua's convention.
let luaName = 'luajit'
if (runtime == 'lua')
  luaName = 'lua_' + luaver.substring(1, luaver.lastIndexOf('.'))

// Create zip archive of the module.
fs.ensureDirSync('out/Dist')
const zip = createZip({withLicense: true})
if (targetOs == 'win')
  zip.addFile('out/Lua/WebView2Loader.dll', 'out/Lua')
zip.addFile(`out/Lua/${targetOs == 'win' ? 'yue.dll' : 'yue.so'}`, 'out/Lua')
   .writeToFile(`lua_yue_${luaName}_${version}_${targetOs}_${targetCpu}`)
