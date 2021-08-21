#!/usr/bin/env node

// Copyright 2021 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {argv, streamPromise} = require('./common')

const path = require('path')
const tar = require('tar')
const extract = require('extract-zip')
const fs = require('fs-extra')
const fetch = require('node-fetch')
const useTmpDir = require('use-tmp-dir')

if (argv.length != 2) {
  console.error('Usage: download_lua_sources runtime version')
  process.exit(1)
}

const runtime = argv[0]
let version = argv[1]
if (runtime == 'lua')
  version = version.startsWith('v') ? version : `v${version}`
let shortver = version
if (runtime == 'luajit')
  shortver = shortver.substr(0, 7)

const prefix = {
  lua: 'https://www.lua.org/ftp',
  luajit: 'https://github.com/LuaJIT/LuaJIT/archive/',
}

if (!(runtime in prefix)) {
  console.error(`Unkown runtime: ${runtime}`)
  process.exit(2)
}

const luaDir = path.resolve('third_party', `${runtime}-${shortver}`)
if (fs.existsSync(luaDir))
  process.exit(0)

useTmpDir(async (cwd) => {
  if (runtime == 'lua')
    await downloadLua(cwd)
  else
    await downloadLuaJIT(cwd)
})

async function downloadLua(cwd) {
  const filename = `lua-${version.substr(1)}`
  const file = path.join(cwd, filename + '.tar.gz')
  await downloadFile(`${prefix[runtime]}/${filename}.tar.gz`, file)
  await tar.x({file, cwd})
  await fs.move(path.join(cwd, filename), luaDir)
}

async function downloadLuaJIT(cwd) {
  const file = path.join(cwd, version + '.zip')
  await downloadFile(`${prefix[runtime]}/${version}.zip`, file)
  await extract(file, {dir: cwd})
  await fs.move(path.join(cwd, `LuaJIT-${version}`), luaDir)
}

async function downloadFile(url, target) {
  const res = await fetch(url)
  await streamPromise(res.body.pipe(fs.createWriteStream(target)))
}
