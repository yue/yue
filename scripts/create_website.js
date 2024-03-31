#!/usr/bin/env node

// Copyright 2024 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {version} = require('./common')

const cp   = require('node:child_process')
const fs   = require('fs-extra')
const path = require('node:path')
const util = require('node:util')
const cmp  = require('semver-compare')
const pug  = require('pug')
const {pipeline} = require('node:stream/promises')

// Async exec.
const exec = util.promisify(cp.exec)

const buildDir = path.join('out', 'Dist', 'website')

async function main() {
  fs.emptyDirSync(buildDir)

  const tags = await getTags()
  fs.writeFile(path.join(buildDir, 'index.html'),
               pug.renderFile('docs/layout/website_index.pug',
                              {latest: tags[0], versions: tags}))

  await Promise.all(tags.map(extractDocs))
  await fs.copy(path.join(buildDir, 'docs', tags[0]),
                path.join(buildDir, 'docs', 'latest'))
}

async function extractDocs(tag) {
  const url = `https://github.com/yue/yue/releases/download/${tag}/yue_docs_${tag}.zip`
  const targetDir = path.join(buildDir, 'docs', tag)
  await fs.ensureDir(targetDir)
  const zipPath = path.join(targetDir, 'docs.zip')
  const stream = fs.createWriteStream(zipPath)
  const response = await fetch(url)
  await pipeline(response.body, stream)
  await exec('unzip docs.zip', {cwd: targetDir})
  await fs.remove(zipPath)
}

async function getTags() {
  const response = await fetch(`https://api.github.com/repos/yue/yue/releases`)
  const releases = await response.json()
  const tags = []
  for (const release of releases) {
    if (release.assets.find(a => a.name.startsWith('yue_docs_')))
      tags.push(release.tag_name)
  }
  return filterVersions(tags)
}

// Only show one doc for one minor release.
function filterVersions(versions) {
  const result = []
  let minorVer = null
  for (const v of versions) {
    const m = v.substr(0, v.lastIndexOf('.'))
    if (m == minorVer)
      continue
    minorVer = m
    result.push(v)
  }
  return result.slice(0, 8)
}

main()
