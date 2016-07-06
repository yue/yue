#!/usr/bin/env node

// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

const https = require('https')
const fs    = require('fs')
const path  = require('path')

const sha1 = {
  linux: '058d7ef9ba391dc384c478acc7e78652aa09f1b5',
  darwin: 'e640c66cd1f2f5c5ba97abb82f6d35cc22fbaef9',
  win32: '9650b4ea6657e98ae3677f53b3b4b881eafed468',
}

const buildtools = path.resolve(__dirname, '..', 'buildtools')
for (const platform in sha1) {
  const filename = platform === 'win' ? 'gn.exe' : 'gn'
  const gnPath = path.join(buildtools, platform, filename)
  downloadGn(sha1[platform], gnPath)
}

function downloadGn(sha1, target) {
  const url = `https://storage.googleapis.com/chromium-gn/${sha1}`
  const file = fs.createWriteStream(target)
  file.on('finish', () => fs.chmodSync(target, 0755))
  https.get(url, (response) => response.pipe(file))
}
