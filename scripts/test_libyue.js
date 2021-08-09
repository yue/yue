#!/usr/bin/env node

// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {version, clang, targetCpu, targetOs, execSync, spawnSync} = require('./common')

const os = require('os')
const path = require('path')
const fs = require('fs-extra')
const extract = require('extract-zip')
const useTmpDir = require('use-tmp-dir')

const zipname = `libyue_${version}_${targetOs}`

console.log('Building libyue...')
execSync('node scripts/create_source_dist.js')

useTmpDir(async (tmpDir) => {
  console.log('Unzipping libyue...')
  await extract(`out/Dist/${zipname}.zip`, {dir: tmpDir})
  process.chdir(tmpDir)
  runTests()
  process.chdir(path.dirname(__dirname))
})

function runTests() {
  if (targetOs != 'win') {  // use make
    generateProject('Release')
    generateProject('Debug')
  } else {  // use VS
    generateProject()
  }
  buildProject('Release')
  buildProject('Debug')
  // On CI, we have to remove the zip file on non-x64 platforms to avoid
  // uploading the source dist unnecessarily.
  if (process.env.CI == 'true' && targetCpu != 'x64')
    fs.removeSync(`out/Dist/${zipname}.zip`)
}

function generateProject(config) {
  if (targetOs == 'win') {
    const buildDir = `build_${targetCpu}`
    fs.ensureDirSync(buildDir)
    const platform = targetCpu == 'x64' ? 'x64' : 'Win32'
    const args = ['-S', '.', '-G', 'Visual Studio 16 2019', '-B', buildDir, '-A', platform]
    if (clang)
      args.push('-T', 'ClangCL')
    spawnSync('cmake', args)
  } else {
    const buildDir = `build_${config}`
    fs.ensureDirSync(buildDir)
    execSync(`cmake -DCMAKE_BUILD_TYPE=${config} ..`, {cwd: buildDir})
  }
}

function buildProject(config) {
  const cpus = os.cpus().length
  if (targetOs == 'win') {
    if (clang) {
      execSync(`cmake --build build_${targetCpu} --config ${config} --parallel ${cpus}`)
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
          `/maxcpucount:${cpus}`,
          `/p:Configuration=${config}`,
          `/p:Platform=${platform}`],
        {cwd: `build_${targetCpu}`, env})
    }
  } else {
    execSync(`make -j ${cpus}`, {cwd: `build_${config}`})
  }
}
