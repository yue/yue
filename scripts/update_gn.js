#!/usr/bin/env node

// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const https = require('https')
const fs    = require('fs')
const path  = require('path')

const sha1 = {
  linux: '2d79b4acffac0b084f250a9699f670d41f164864',
  darwin: 'b8db422af70c716836c579dad9153c687ab54f7c',
  win32: '3b6a35d309aeeba61ba945d91752d1d65acf3d4f',
}

const buildtools = path.resolve(__dirname, '..', 'tools', 'build')
for (const platform in sha1) {
  const filename = platform === 'win32' ? 'gn.exe' : 'gn'
  const gnPath = path.join(buildtools, platform, filename)
  downloadGn(sha1[platform], gnPath)
}

function downloadGn(sha1, target) {
  const url = `https://storage.googleapis.com/chromium-gn/${sha1}`
  const file = fs.createWriteStream(target)
  file.on('finish', () => fs.chmodSync(target, 0755))
  https.get(url, (response) => response.pipe(file))
}
