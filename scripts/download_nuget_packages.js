#!/usr/bin/env node

// Copyright 2020 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {spawnSync, sleep, streamPromise} = require('./common')
const {webview2Version} = require('./config')

const path  = require('path')
const fs    = require('fs-extra')
const fetch = require('node-fetch')

const nugetVersion = 'v5.5.1'
const nugetPackages = {
  'Microsoft.Web.WebView2': webview2Version
}

const nuget = path.join('third_party', 'nuget.exe')

main()

async function main() {
  if (!fs.existsSync(nuget)) {
    await downloadNuget()
    await sleep(1000)
  }
  downloadPackages()
}

async function downloadNuget() {
  const url = `https://dist.nuget.org/win-x86-commandline/${nugetVersion}/nuget.exe`
  const res = await fetch(url)
  await streamPromise(res.body.pipe(fs.createWriteStream(nuget)))
}

function downloadPackages() {
  for (const name in nugetPackages)
    spawnSync(nuget, ['install', name,
                      '-Version', nugetPackages[name],
                      '-OutputDirectory', 'third_party'])
}
