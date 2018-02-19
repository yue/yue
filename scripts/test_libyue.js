#!/usr/bin/env node

// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {argv, version, targetCpu, targetOs, execSync, mkdir} = require('./common')

const fs = require('fs')
const os = require('os')
const path = require('path')
const extract = require('./libs/extract-zip')

// Our work dir.
const zipname = `libyue_${version}_${targetOs}_${targetCpu}`
const tmppath = path.join(os.tmpdir(), zipname)

// Bulid and package.
console.log('Building libyue...')
execSync('node scripts/build.js out/Release')
execSync('node scripts/build.js out/Debug')
console.log('Zipping and unzipping libyue...')
execSync('node scripts/create_dist.js')
extract(`out/Dist/${zipname}.zip`, {dir: tmppath}, runTests)

function runTests(error) {
  if (error) {
    console.error(error)
    process.exit(1)
  }
  if (targetCpu !== 'x64' && targetOs !== 'win') {
    return
  }
  process.chdir(tmppath)
  generateProject()
  buildProject()
  console.log(tmppath)
}

function generateProject() {
  if (process.platform == 'linux') {
    mkdir('Release')
    execSync('cmake -D CMAKE_BUILD_TYPE=Release ..', {cwd: 'Release'})
    mkdir('Debug')
    execSync('cmake -D CMAKE_BUILD_TYPE=Debug ..', {cwd: 'Debug'})
  } else if (process.platform == 'darwin') {
    mkdir('build')
    execSync('cmake .. -G Xcode', {cwd: 'build'})
  } else if (process.platform == 'win32') {
    mkdir('build')
    if (targetCpu == 'x64')
      execSync('cmake .. -G "Visual Studio 15 2017 Win64"', {cwd: 'build'})
    else if (targetCpu == 'x86')
      execSync('cmake .. -G "Visual Studio 15 2017"', {cwd: 'build'})
  }
}

function buildProject() {
  if (process.platform == 'linux') {
    execSync('make', {cwd: 'Release'})
    execSync('make', {cwd: 'Debug'})
  } else if (process.platform == 'darwin') {
    execSync('xcodebuild -configuration Release', {cwd: 'build'})
    execSync('xcodebuild -configuration Debug', {cwd: 'build'})
  } else if (process.platform == 'win32') {
    const platform = targetCpu == 'x64' ? 'x64' : 'Win32'
    const msbuild = 'C:\\Program Files (x86)\\Microsoft Visual Studio\\2017\\Community\\MSBuild\\15.0\\Bin\\MSBuild.exe'
    execSync(`"${msbuild}" YueSampleApp.sln /p:Configuration=Release /p:Platform=${platform}`,
             {cwd: 'build'})
    execSync(`"${msbuild}" YueSampleApp.sln /p:Configuration=Debug /p:Platform=${platform}`,
             {cwd: 'build'})
  }
}
