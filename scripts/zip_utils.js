// Copyright 2020 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const path = require('path')
const fs = require('fs')
const JSZip = require('../third_party/bundled_node_modules/jszip')

class ZipFile {
  constructor(options = {withLicense: false}) {
    this.zip = new JSZip()
    if (options.withLicense)
      this.zip.file('LICENSE', collectLicenses())
  }

  addFile(file, base = '') {
    const stat = fs.statSync(file)
    if (stat.isDirectory()) {
      const subfiles = fs.readdirSync(file)
      for (let sub of subfiles)
        this.addFile(`${file}/${sub}`, base)
    } else if (stat.isFile()) {
      let options = {binary: true}
      if (process.platform != 'win32') {
        try {
          fs.accessSync(file, fs.constants.X_OK)
          options.unixPermissions = '755'
        } catch (e) {
          options.unixPermissions = '644'
        }
      }
      let rp = path.relative(base, file)
      // Some unzip tools force using / as file delimiter.
      if (process.platform == 'win32')
        rp = rp.replace(/\\/g, '/')
      this.zip.file(rp, fs.readFileSync(file), options)
    }
    return this
  }

  writeToFile(name, dir = 'out/Dist') {
    this.zip.generateNodeStream({
      streamFiles: true,
      compression: 'DEFLATE',
      compressionOptions: {
        level: 9
      }}).pipe(fs.createWriteStream(`${dir}/${name}.zip`))
  }
}

function createZip(options) {
  return new ZipFile(options)
}

function collectLicenses() {
  const licenses = [
    { path: 'LICENSE' },
    { path: 'LICENSE.chromium' },
  ]
  const places = [
    'third_party',
    'base/third_party',
  ]
  for (const place of places) {
    for (const lib of fs.readdirSync(place)) {
      const license = path.join(place, lib, 'LICENSE')
      if (!fs.existsSync(license))
        continue
      licenses.push({path: license, name: `${place}/${lib}`})
    }
  }
  const separator = '-'.repeat(80)
  return licenses.map((f) => {
    const content = fs.readFileSync(f.path)
    if (f.name)
      return f.name + ':\n\n' + content
    else
      return content
  }).join(`\n\n${separator}\n\n\n`)
}

module.exports = {createZip}
