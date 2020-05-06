#!/usr/bin/env node

// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {version, targetOs, execSync, spawnSync, mkdir} = require('./common')

const fs = require('fs')
const os = require('os')
const path = require('path')
const extract = require('./libs/extract-zip')

// Our work dir.
const zipname = `libyue_${version}_${targetOs}`
const tmppath = path.join(os.tmpdir(), zipname)

// Bulid and package.
console.log('Building libyue...')
execSync('node scripts/build.js out/Release')
console.log('Zipping and unzipping libyue...')
execSync('node scripts/create_dist.js')
extract(`out/Dist/${zipname}.zip`, {dir: tmppath}, runTests)

function runTests(error) {
  if (error) {
    console.error(error)
    process.exit(1)
  }
  process.chdir(tmppath)
  generateProject()
  buildProject()
  console.log(tmppath)
}

function generateProject() {
  if (process.platform == 'linux' || process.platform == 'darwin') {
    mkdir('build')
    execSync('cmake ..', {cwd: 'build'})
  } else if (process.platform == 'win32') {
    mkdir('build_x64')
    mkdir('build_Win32')
    spawnSync('cmake', ['..', '-G', 'Visual Studio 15 2017 Win64'], {cwd: 'build_x64'})
    spawnSync('cmake', ['..', '-G', 'Visual Studio 15 2017'], {cwd: 'build_Win32'})
  }
}

function buildProject() {
  if (process.platform == 'linux' || process.platform == 'darwin') {
    execSync(`make -j ${os.cpus().length}`, {cwd: 'build'})
  } else if (process.platform == 'win32') {
    const vsPaths = [
      'C:\\Program Files (x86)\\Microsoft Visual Studio\\2017\\Community\\MSBuild\\15.0\\Bin',
      'C:\\Program Files (x86)\\Microsoft Visual Studio\\2017\\Enterprise\\MSBuild\\15.0\\Bin',
      process.env.PATH
    ]
    const env = Object.assign(process.env, {PATH: vsPaths.join(path.delimiter)})
    for (const platform of ['x64', 'Win32']) {
      spawnSync(
        'msbuild',
        ['Yue.sln',
          '/maxcpucount:' + os.cpus().length,
          '/p:Configuration=Release',
          '/p:Platform=' + platform],
        {cwd: 'build_' + platform, env})
    }
  }
}
