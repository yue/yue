#!/usr/bin/env node

// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

const https = require('https')
const fs    = require('fs')
const path  = require('path')

const sha1 = {
  linux: 'f9c665db33ad9a4653b2467fc34fbf5a4e3fb4f2',
  darwin: '0508bb515ce789ff2858ba5a2c0042cbd22afa13',
  win32: 'ef11542024a22846422b213316715a4e77f7e009',
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
