#!/usr/bin/env node

// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {exec} = require('child_process')
const {lstatSync, readdirSync} = require('fs')
const path = require('path')

require('./common')

// Do not lint following files.
const blacklist = [
  path.join('nativeui', 'mac', 'container_mac.h'),
  path.join('nativeui', 'mac', 'nu_application_delegate.h'),
  path.join('nativeui', 'win', 'util', 'msg_util.h'),
]

const sourceFiles = listFiles(['yue', 'lua', 'nativeui'])
.filter((name) => {
  return name.endsWith('.cc') || name.endsWith('.h')
})
.filter((name) => {
  return !blacklist.includes(name)
})

// Call cpplint.
const cpplint = path.join('tools', 'cpplint.py')
const child = exec(`python ${cpplint} ${sourceFiles.join(' ')}`)
child.stderr.on('data', (chunck) => {
  if (chunck.startsWith('Done processing'))
    return
  process.stdout.write(chunck)
})

// Util function to list files recursively
function listFiles(dirs) {
  result = []
  for (const dir of dirs) {
    const subdirs = readdirSync(dir)
    for (const sub of subdirs) {
      const stat = lstatSync(path.join(dir, sub))
      if (stat.isFile())
        result.push(path.join(dir, sub))
      else if (stat.isDirectory())
        result = result.concat(listFiles([path.join(dir, sub)]))
    }
  }
  return result
}
