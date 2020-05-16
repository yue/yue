#!/usr/bin/env node

// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {version, targetCpu, targetOs, execSync, spawnSync} = require('./common')

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
    if (targetCpu == 'x86') {
      fs.ensureDirSync('build_Win32')
      spawnSync('cmake', ['..', '-G', 'Visual Studio 15 2017'],
                {cwd: 'build_Win32'})
    } else {
      fs.ensureDirSync('build_x64')
      spawnSync('cmake', ['..', '-G', 'Visual Studio 15 2017 Win64'],
                {cwd: 'build_x64'})
    }
  } else {
    fs.ensureDirSync('build')
    execSync('cmake ..', {cwd: 'build'})
  }
}

function buildProject() {
  if (targetOs == 'win') {
    const vsPaths = [
      'C:\\Program Files (x86)\\Microsoft Visual Studio\\2017\\Community\\MSBuild\\15.0\\Bin',
      'C:\\Program Files (x86)\\Microsoft Visual Studio\\2017\\Enterprise\\MSBuild\\15.0\\Bin',
      process.env.PATH
    ]
    const env = Object.assign(process.env, {PATH: vsPaths.join(path.delimiter)})
    const platform = targetCpu == 'x64' ? 'x64' : 'Win32'
    spawnSync(
      'msbuild',
      ['Yue.sln',
        '/maxcpucount:' + os.cpus().length,
        '/p:Configuration=Release',
        '/p:Platform=' + platform],
      {cwd: 'build_' + platform, env})
  } else {
    execSync(`make -j ${os.cpus().length}`, {cwd: 'build'})
  }
}
