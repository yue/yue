// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

const https = require('https')
const fs    = require('fs')
const path  = require('path')

const buildtools = path.resolve(__dirname, '..', 'third_party', 'buildtools')
const platform = process.platform === 'win32' ?  'win' :
                 process.platform === 'darwin' ?  'mac' :
                 process.platform === 'linux' ? 'linux64' : 'unkown'
const filename = process.platform === 'win32' ? 'gn.exe' : 'gn'
const gnPath = path.join(buildtools, platform, filename)

downloadGn(getGnSha1(gnPath), gnPath)

function getGnSha1(gnPath) {
  return fs.readFileSync(gnPath + '.sha1').toString().trim()
}

function downloadGn(sha1, target) {
  const url = `https://storage.googleapis.com/chromium-gn/${sha1}`
  const file = fs.createWriteStream(target)
  file.on('finish', () => fs.chmodSync(target, 0755))
  https.get(url, (response) => response.pipe(file))
}
