#!/usr/bin/env node

// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {version, targetCpu, targetOs, searchFiles, execSync} = require('./common')

const path = require('path')
const fs = require('./libs/fs-extra')
const JSZip = require('./libs/jszip')

// Lua loadable module.
const luaFiles = {
  linux: [
    'yue.so',
  ],
  mac: [
    'yue.so',
  ],
  win: [
    'yue.dll',
  ],
}

// Clear previous distributions.
fs.removeSync('out/Dist')
fs.ensureDirSync('out/Dist')

// Strip binaries for Linux.
if (targetOs == 'linux') {
  for (const file of luaFiles[targetOs])
    strip(`out/Release/${file}`)
}

// Zip other binaries.
generateZip('lua_yue_lua_5.3', luaFiles[targetOs])

// Zip sources.
if (targetCpu == 'x64') {
  execSync('node ./scripts/create_source_dist.js')
  const zip = addFileToZip(new JSZip(), 'out/Dist/source', 'out/Dist/source')
  addFileToZip(zip, 'sample_app/CMakeLists.txt', 'sample_app')
  addFileToZip(zip, 'sample_app/main.cc', '')
  addLicenseToZip(zip)
  writeZipToFile(zip, `libyue_${version}_${targetOs}`)
}

// Zip docs, but only do it for linux/x64 when running on CI, to avoid uploading
// docs for multiple times.
if (process.env.CI != 'true' || (targetOs == 'linux' && targetCpu == 'x64')) {
  execSync('node ./scripts/create_docs.js')
  writeZipToFile(addFileToZip(new JSZip(), 'out/Dist/docs', 'out/Dist/docs'),
                 `yue_docs_${version}`)
}

function generateZip(name, list, zip = new JSZip()) {
  for (let file of list)
    addFileToZip(zip, `out/Release/${file}`, 'out/Release')
  addLicenseToZip(zip)
  writeZipToFile(zip, `${name}_${version}_${targetOs}_${targetCpu}`)
}

function addFileToZip(zip, file, base, prefix = '', suffix = '') {
  const stat = fs.statSync(file)
  if (stat.isDirectory()) {
    const subfiles = fs.readdirSync(file)
    for (let sub of subfiles)
      addFileToZip(zip, `${file}/${sub}`, base)
  } else if (stat.isFile()) {
    let options = {binary: true}
    if (process.platform !== 'win32') {
      try {
        fs.accessSync(file, fs.constants.X_OK)
        options.unixPermissions = '755'
      } catch (e) {
        options.unixPermissions = '644'
      }
    }
    const extname = path.extname(file)
    const filename = path.basename(file, extname) + suffix + extname
    let rp = path.relative(base, file)
    rp = path.join(prefix, path.dirname(rp), filename)
    if (process.platform === 'win32') {
      // Some unzip tools force using / as file delimiter.
      rp = rp.replace(/\\/g, '/')
    }
    zip.file(rp, fs.readFileSync(file), options)
  }
  return zip
}

const licenses = collectLicenses()
function addLicenseToZip(zip) {
  zip.file('LICENSE', collectLicenses())
}

function writeZipToFile(zip, name) {
  zip.generateNodeStream({
      streamFiles: true,
      compression: 'DEFLATE',
      compressionOptions: {
        level: 9
      }}).pipe(fs.createWriteStream(`out/Dist/${name}.zip`))
}

function strip(file) {
  // TODO(zcbenz): Use |file| command to determine type.
  if (!file.endsWith('.so') && path.basename(file) != 'yue')
    return
  // TODO(zcbenz): Copy the debug symbols out before striping.
  let strip = 'strip'
  if (targetCpu == 'arm')
    strip = 'arm-linux-gnueabihf-strip'
  else if (targetCpu == 'arm64')
    strip = 'aarch64-linux-gnu-strip'
  execSync(`${strip} ${file}`)
}

function collectLicenses() {
  const licenses = [
    { path: 'LICENSE' },
    { path: 'LICENSE.chromium' },
  ]
  const places = [
    'third_party',
    'base/third_party',
  ]
  for (const place of places) {
    for (const lib of fs.readdirSync(place)) {
      const license = path.join(place, lib, 'LICENSE')
      if (!fs.existsSync(license))
        continue
      licenses.push({path: license, name: `${place}/${lib}`})
    }
  }
  const separator = '-'.repeat(80)
  return licenses.map((f) => {
    const content = fs.readFileSync(f.path)
    if (f.name)
      return f.name + ':\n\n' + content
    else
      return content
  }).join(`\n\n${separator}\n\n\n`)
}
