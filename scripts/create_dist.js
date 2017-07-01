#!/usr/bin/env node

// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {version, targetCpu, targetOs, mkdir, execSync} = require('./common')

const fs = require('fs')
const path = require('path')
const JSZip = require('./libs/jszip')

// C++ static library.
const staticLibs = {
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
    'yue',
  ],
  mac: [
    'yue',
  ],
  win: [
    'yue.exe',
  ],
}

// Clear previous distributions.
mkdir('out/Dist')
fs.readdirSync('out/Dist').forEach((f) => {
  if (f.endsWith('.zip'))
    fs.unlinkSync(`out/Dist/${f}`)
})

// Strip binaries for Linux.
if (targetOs == 'linux') {
  const list = staticLibs.linux.concat(luaFiles.linux).concat(exeFiles.linux)
  for (const file of list)
    strip(`out/Release/${file}`)
}

// Zip the static library and headers.
const yuezip = new JSZip()
const headers = searchFiles('base', '.h').concat(
                searchFiles('nativeui', '.h')).concat(
                searchFiles('testing', '.h')).concat(
                ['build/build_config.h', 'build/buildflag.h'])
for (const h of headers)
  addFileToZip(yuezip, h, '', 'include')
for (const file of staticLibs[targetOs]) {
  addFileToZip(yuezip, `out/Release/${file}`, 'out/Release', 'lib')
  addFileToZip(yuezip, `out/Debug/${file}`, 'out/Debug', 'lib', 'd')
}
generateZip('libyue', [], yuezip)

// Zip other binaries.
generateZip('yue', exeFiles[targetOs])
if (targetOs != 'win')
  generateZip('lua_yue_lua_5.3', luaFiles[targetOs])

// Zip docs, but only do it for linux/x64 when running on CI, to avoid uploading
// docs for multiple times.
if (process.env.CI != 'true' || (targetOs == 'linux' && targetCpu == 'x64')) {
  execSync('node ./scripts/create_docs.js')
  addFileToZip(new JSZip(), 'out/Dist/docs', 'out/Dist/docs')
    .generateNodeStream({streamFiles:true})
    .pipe(fs.createWriteStream(`out/Dist/yue_docs_${version}.zip`))
}

function generateZip(name, list, zip = new JSZip()) {
  const zipname = `${name}_${version}_${targetOs}_${targetCpu}`
  for (let file of list)
    addFileToZip(zip, `out/Release/${file}`, 'out/Release')
  zip.generateNodeStream({streamFiles:true})
     .pipe(fs.createWriteStream(`out/Dist/${zipname}.zip`))
}

function addFileToZip(zip, file, base, prefix = '', suffix = '') {
  const stat = fs.statSync(file)
  if (stat.isDirectory()) {
    const subfiles = fs.readdirSync(file)
    for (let sub of subfiles)
      addFileToZip(zip, `${file}/${sub}`, base)
  } else if (stat.isFile()) {
    const extname = path.extname(file)
    const filename = path.basename(file, extname) + suffix + extname
    let p = path.relative(base, file)
    p = path.join(prefix, path.dirname(p), filename)
    zip.file(p, fs.readFileSync(file))
  }
  return zip
}

function searchFiles(dir, suffix, list = []) {
  return fs.readdirSync(dir).reduce((list, filename) => {
    const p = path.join(dir, filename)
    const stat = fs.statSync(p)
    if (stat.isFile() && filename.endsWith(suffix))
      list.push(p)
    else if (stat.isDirectory())
      searchFiles(p, suffix, list)
    return list
  }, list)
}

function strip(file) {
  // TODO(zcbenz): Use |file| command to determine type.
  if (!file.endsWith('.so') && path.basename(file) != 'yue')
    return
  // TODO(zcbenz): Copy the debug symbols out before striping.
  const strip = targetCpu.startsWith('arm') ? 'arm-linux-gnueabihf-strip'
                                            : 'strip'
  execSync(`${strip} ${file}`)
}
