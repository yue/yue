#!/usr/bin/env node

// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {version, clang, targetCpu, targetOs, execSync, spawnSync} = require('./common')

const os = require('os')
const path = require('path')
const fs = require('fs-extra')
const extract = require('extract-zip')

// Our work dir.
const zipname = `libyue_${version}_${targetOs}`
const tmppath = path.join(os.tmpdir(), zipname)

// Bulid and package.
console.log('Building libyue...')
execSync('node scripts/create_source_dist.js')
console.log('Unzipping libyue...')
fs.removeSync(tmppath)
extract(`out/Dist/${zipname}.zip`, {dir: tmppath}, runTests)

function runTests(error) {
  if (error) {
    console.error(error)
    process.exit(1)
  }
  process.chdir(tmppath)
  generateProject()
  buildProject()
  // On CI, we have to remove the zip file on non-x64 platforms to avoid
  // uploading the source dist unnecessarily.
  if (process.env.CI == 'true' && targetCpu != 'x64')
    fs.removeSync(`out/Dist/${zipname}.zip`)
  console.log(tmppath)
}

function generateProject() {
  if (targetOs == 'win') {
    const buildDir = `build_${targetCpu}`
    fs.ensureDirSync(buildDir)
    const platform = targetCpu == 'x64' ? 'x64' : 'Win32'
    const args = ['-S', '.', '-G', 'Visual Studio 16 2019', '-B', buildDir, '-A', platform]
    if (clang)
      args.push('-T', 'ClangCL')
    spawnSync('cmake', args)
  } else {
    fs.ensureDirSync('build')
    execSync('cmake ..', {cwd: 'build'})
  }
}

function buildProject() {
  const cpus = os.cpus().length
  if (targetOs == 'win') {
    if (clang) {
      execSync(`cmake --build build_${targetCpu} --config Debug --parallel ${cpus}`)
    } else {
      const vsPaths = [
        'C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\MSBuild\\Current\\Bin',
        'C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Enterprise\\MSBuild\\Current\\Bin',
        process.env.PATH
      ]
      const env = Object.assign(process.env, {PATH: vsPaths.join(path.delimiter)})
      const platform = targetCpu == 'x64' ? 'x64' : 'Win32'
      spawnSync(
        'msbuild',
        ['Yue.sln',
          '/maxcpucount:' + cpus,
          '/p:Configuration=Release',
          '/p:Platform=' + platform],
        {cwd: `build_${targetCpu}`, env})
    }
  } else {
    execSync(`make -j ${cpus}`, {cwd: 'build'})
  }
}
