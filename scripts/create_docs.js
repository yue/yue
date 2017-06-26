#!/usr/bin/env node

// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {version} = require('./common')

const fs     = require('fs')
const path   = require('path')
const marked = require('./libs/marked')
const JSZip  = require('./libs/jszip')
const yaml   = loadYaml()
const pug    = loadPug()
const hljs   = loadHighlight()

// Supported languages.
const langs = ['cpp', 'lua', 'js']

// The renderer that can filter language specific paragraphs.
const renderer = new marked.Renderer()
const oc = renderer.code.bind(renderer)
renderer.code = (code, lang) => (!langs.includes(lang) || lang == renderer.lang) ? oc(code, lang) : ''

// Support highlighting in markdown.
marked.setOptions({
  renderer,
  highlight: (code, lang) => hljs.highlightAuto(code, [lang]).value
})

// Dir to put generated files.
const gendir = path.join('out', 'Release', 'gen', 'docs')

// Iterate API documents for different languages.
for (let lang of langs) {
  renderer.lang = lang

  const langdir = path.join(gendir, lang)
  mkdir(langdir)

  // Parse all API docs.
  const docs = fs.readdirSync('docs/api').reduce((docs, file) => {
    if (!file.endsWith('.yaml')) return docs
    const name = path.basename(file, '.yaml')
    const doc = yaml.load(fs.readFileSync(`docs/api/${file}`))
    const langDoc = pruneDocTree(lang, doc)
    if (!langDoc) return docs
    langDoc.id = name
    return docs.concat(langDoc)
  }, [])

  // Parse all guides.
  const guides = []
  for (const dir of ['docs/guides', `docs/guides/${lang}`]) {
    for (const file of fs.readdirSync(dir)) {
      if (!file.endsWith('.md')) continue
      const content = String(fs.readFileSync(`${dir}/${file}`))
      const guide = parseMarkdown(lang, content)
      guide.id = path.basename(file, '.md')
      guides.push(guide)
    }
  }
  guides.sort((a, b) => b.priority - a.priority)

  // Generate the index page.
  const html = pug.renderFile('docs/template/index.pug', {
    page: 'index',
    doc: { name: 'Docs' },
    lang,
    version,
    types: docs,
    guides,
    markdown: marked,
    imarkdown: inlineMarkdown,
    filters: { 'css-minimize': cssMinimize },
  })
  fs.writeFileSync(path.join(langdir, `index.html`), html)

  // Generate pages for guides.
  for (let guide of guides) {
    const guidedir = path.join(langdir, 'guides')
    mkdir(guidedir)
    const html = pug.renderFile('docs/template/guide.pug', {
      page: 'guide',
      doc: guide,
      filters: { 'css-minimize': cssMinimize },
    })
    fs.writeFileSync(path.join(guidedir, `${guide.id}.html`), html)
  }

  // Read API docs and generate HTML pages.
  for (let doc of docs) {
    const apidir = path.join(langdir, 'api')
    mkdir(apidir)
    // Output JSON files.
    fs.writeFileSync(path.join(apidir, `${doc.id}.json`),
                     JSON.stringify(doc, null, '  '))
    // Output HTML pages.
    const html = pug.renderFile('docs/template/api.pug', {
      name: doc.name,
      page: 'api',
      doc,
      lang,
      version,
      types: docs,
      markdown: marked,
      imarkdown: inlineMarkdown,
      filters: { 'css-minimize': cssMinimize },
    })
    fs.writeFileSync(path.join(apidir, `${doc.id}.html`), html)
  }
}

// Load js-yaml from its browserify pack.
function loadYaml() {
  const vm = require('vm')
  const script = new vm.Script(fs.readFileSync(`${__dirname}/libs/yaml.js`))
  const sandbox = {}
  script.runInNewContext(sandbox)
  return sandbox.jsyaml
}

// Load pug.js from its browserify pack.
function loadPug() {
  const vm = require('vm')
  const script = new vm.Script(fs.readFileSync(`${__dirname}/libs/pug.js`))
  const sandbox = {fs: fs}
  script.runInNewContext(sandbox)
  return sandbox.require('pug')
}

// Load hightlight.js from its browserify pack.
function loadHighlight() {
  const vm = require('vm')
  const script = new vm.Script(fs.readFileSync(`${__dirname}/libs/highlight.js`))
  const sandbox = {}
  sandbox.window = sandbox
  script.runInNewContext(sandbox)
  return sandbox.hljs
}

// Make dir and ignore error.
function mkdir(dir) {
  if (fs.existsSync(dir)) return
  mkdir(path.dirname(dir))
  fs.mkdirSync(dir)
}

// Read markdown and parse yaml header if there is one.
function parseMarkdown(lang, content) {
  let lines = content.split('\n')
  let result = {}
  if (lines[0] == '---') {
    let i = 1
    for (; i < lines.length; ++i)
      if (lines[i] == '---') break
    if (i == lines.length)
      throw new Error('Invalid markdown header')
    result = yaml.load(lines.slice(1, i).join('\n'))
    lines = lines.slice(i + 1)
  }
  if (result.priority === undefined)
    result.priority = 0
  result.content = marked(lines.join('\n'))
  result.name = result.content.substring(result.content.indexOf('>') + 1,
                                         result.content.indexOf('</h1>'))
  return result
}

// Parse |doc| tree and only keep nodes for |lang|.
function pruneDocTree(lang, doc) {
  if (doc.lang && !doc.lang.includes(lang))
    return null

  doc = JSON.parse(JSON.stringify(doc))

  if (doc.inherit)
    doc.inherit = parseType(lang, doc.inherit)

  if (lang != 'cpp')
    convertModuleAndType(lang, doc)

  doc.detail = parseDetail(lang, doc)

  const categories = [ 'global_functions', 'constructors', 'class_properties',
                       'class_methods', 'properties', 'methods', 'events',
                       'delegates' ]
  for (let category of categories) {
    if (!doc[category]) continue
    let nodes = []
    for (let node of doc[category]) {
      if (!node.lang || (node.lang && node.lang.includes(lang)))
        nodes.push(pruneNode(lang, node))
    }
    if (nodes.length > 0)
      doc[category] = nodes
    else
      delete doc[category]
  }

  return doc
}

// Convert C++ representation to |lang| representation.
function pruneNode(lang, node) {
  node = JSON.parse(JSON.stringify(node))

  node.detail = parseDetail(lang, node)

  delete node.lang
  // Recursively prune the Dictionary type parameters.
  if (node.parameters) {
    for (let param in node.parameters) {
      let descriptor = node.parameters[param]
      if (descriptor.properties) {
        for (let i in descriptor.properties)
          descriptor.properties[i] = pruneNode(lang, descriptor.properties[i])
      }
    }
  }

  if (node.signature) {
    node.id = generateIdForSignature(node.signature)
    node.signature = parseSignature(lang, node.signature)
    if (node.parameters) {
      mergePrameters(node.signature, node.parameters)
      delete node.parameters
    }
  } else if (node.property) {
    node.id = parseParam('lua', node.property).name
    Object.assign(node, parseParam(lang, node.property))
    delete node.property
  } else if (node.callback) {
    node.id = generateIdForSignature(node.callback)
    node.callback = parseSignature(lang, node.callback)
    if (node.parameters) {
      mergePrameters(node.callback, node.parameters)
      delete node.parameters
    }
  }

  return node
}

// Convert C++ namespace and type to script modules.
function convertModuleAndType(lang, node) {
  if (node.component == 'gui') {
    if (lang == 'lua')
      node.module = 'yue.gui'
    else if (lang == 'js')
      node.module = 'gui'
  }

  let wrappedTypes = ['Painter', 'App', 'Lifetime', 'Signal']
  let stringTypes = ['Accelerator', 'KeyboardCode', 'Font::Weight']
  let integerTypes = ['Color']
  if (node.type == 'refcounted' || wrappedTypes.includes(node.name))
    node.type = 'Class'
  else if (node.type == 'enum class' || stringTypes.includes(node.name))
    node.type = lang == 'lua' ? 'string' : 'String'
  else if (node.type == 'enum' || integerTypes.includes(node.name))
    node.type = lang == 'lua' ? 'integer' : 'Integer'
  else
    node.type = lang == 'lua' ? 'table' : 'Object'
}

// Parse the C++ signature string.
function parseSignature(lang, str) {
  let signature = {}
  let match = str.match(/^(\w+)\((.*)\)$/)
  if (match) {
    // Constructor type.
    signature.name = match[1]
    signature.parameters = match[2]
  } else {
    match = str.match(/^(.*) (\w+)\((.*)\).*$/)
    if (match[1] != 'void')
      signature.returnType = parseType(lang, match[1])
    signature.name = match[2]
    signature.parameters = match[3]
  }
  signature.name = parseName(lang, signature.name)
  signature.parameters = parseParameters(lang, signature.parameters)
  if (lang == 'cpp') {
    signature.str = str
  } else {
    let parameters = signature.parameters.map((param) => param.name)
    signature.str = `${signature.name}(${parameters.join(', ')})`
  }
  return signature
}

// Parse detailed information of a node.
function parseDetail(lang, node) {
  if (node.lang_detail && node.lang_detail[lang]) {
    if (!node.detail)
      node.detail = ''
    node.detail += '\n' + node.lang_detail[lang]
  }
  delete node.lang_detail
  return node.detail
}

// Parse parameters.
function parseParameters(lang, str) {
  if (str == '') return []
  let parameters = str.split(',').map(parseParam.bind(null, lang))
  return parameters
}

// Parse param.
function parseParam(lang, str) {
  let match = str.trim().match(/(.+) (\w+)/)
  return { type: parseType(lang, match[1]), name: parseName(lang, match[2]) }
}

// Convert method name from C++ to |lang|.
function parseName(lang, str) {
  if (lang == 'cpp')
    return str
  else if (lang == 'lua')
    return str.replace(/_/g, '').toLowerCase()
  else if (lang == 'js')
    return (str[0].toLowerCase() + str.substr(1)).replace(/_([a-z])/g, (m, w) => {
      return w.toUpperCase()
    })
}

// Convert type name from C++ to |lang|.
function parseType(lang, str) {
  // Strip C++ qualifiers.
  let type = { name: str }
  if (type.name.startsWith('const '))
    type.name = type.name.substr('const '.length)
  if (type.name.endsWith('*') || type.name.endsWith('&'))
    type.name = type.name.substr(0, type.name.length - 1)
  // Parse the templates.
  let match = type.name.match(/^(.*)<(.*)>$/)
  if (match) {
    type.name = match[1]
    if (type.name == 'std::tuple') {
      type.template = match[1]
      type.args = match[2].split(',').map((t) => parseType(lang, t.trim()))
    } else if (type.name == 'scoped_refptr') {
      type.name = match[2]
    }
  }
  // No need to convert types for C++.
  let builtin = true
  if (lang == 'cpp') {
    let builtins = [ 'bool', 'float', 'std::string', 'char', 'uint32_t',
                     'unsigned', 'int', 'Args...', 'base::Callback' ]
    builtin = builtins.includes(type.name)
  }
  // Convertbuilt-in types for differnt languages.
  if (lang == 'lua') {
    switch (type.name) {
      case 'bool': type.name = 'boolean'; break
      case 'float': type.name = 'number'; break
      case 'std::string': type.name = 'string'; break
      case 'char': type.name = 'string'; break
      case 'uint32_t': type.name = 'integer'; break
      case 'unsigned': type.name = 'integer'; break
      case 'int': type.name = 'integer'; break
      case 'Dictionary': type.name = 'table'; break
      case 'Array': type.name = 'table'; break
      case 'Function': type.name = 'function'; break
      case 'base::Callback': type.name = 'function'; break
      case 'base::FilePath': type.name = 'string'; break
      default: builtin = false
    }
  } else if (lang == 'js') {
    switch (type.name) {
      case 'bool': type.name = 'Boolean'; break
      case 'float': type.name = 'Number'; break
      case 'std::string': type.name = 'String'; break
      case 'char': type.name = 'String'; break
      case 'uint32_t': type.name = 'Integer'; break
      case 'unsigned': type.name = 'Integer'; break
      case 'int': type.name = 'Integer'; break
      case 'Dictionary': type.name = 'Object'; break
      case 'Array': type.name = 'Array'; break
      case 'Function': type.name = 'Function'; break
      case 'base::Callback': type.name = 'Function'; break
      case 'base::FilePath': type.name = 'String'; break
      default: builtin = false
    }
  }
  // Custom types usually have 1-to-1 maps.
  if (!builtin)
    type.id = type.name.toLowerCase().replace('::', '_')
  // C++ uses full type as name.
  if (lang == 'cpp')
    type.name = str
  return type
}

// Put the extra parameter descriptions into signature object.
function mergePrameters(signature, parameters) {
  for (let param of signature.parameters) {
    if (parameters[param.name])
      Object.assign(param, parameters[param.name])
  }
}

// Generate a readable ID from signature string.
function generateIdForSignature(str) {
  // Convert to all lowercase names.
  let signature = parseSignature('lua', str)
  let id = signature.name
  for (let param of signature.parameters)
    id += `-${param.name}`
  return id
}

// A simple CSS minimize function.
function cssMinimize(str) {
  let lines = str.split('\n')
  return lines.map((line) => line.trim()).join('')
}

// Strip p tag around markdown result.
function inlineMarkdown(str) {
  let markdown = marked(str)
  return markdown.substr(3, markdown.length - 8)
}
