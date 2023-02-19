#!/usr/bin/env node

// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {version, targetCpu, targetOs, strip} = require('./common')
const {createZip} = require('./zip_utils')

const fs = require('fs-extra')

// Strip the binaries on Linux.
if (targetOs == 'linux')
  strip('out/Release/gui.node')

// Create zip archive of the node module.
fs.ensureDirSync('out/Dist')
const zip = createZip({withLicense: true})
if (targetOs == 'win')
  zip.addFile('out/Release/WebView2Loader.dll', 'out/Release')
zip.addFile('out/Release/gui.node', 'out/Release')
   .writeToFile(`napi_yue_${version}_${targetOs}_${targetCpu}`)
