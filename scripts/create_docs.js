#!/usr/bin/env node

// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {version} = require('./common')
const {createZip} = require('./zip_utils')

const path   = require('path')
const fs     = require('fs-extra')
const marked = require('marked')
const yaml   = loadYaml()
const pug    = loadPug()
const hljs   = loadHighlight()

// Supported languages.
const langs = ['cpp', 'lua', 'js']

// The renderer that can filter language specific paragraphs.
const renderer = new marked.Renderer()
const oc = renderer.code.bind(renderer)
renderer.code = (code, lang) => (!langs.includes(lang) || lang == renderer.lang) ? oc(code, lang) : ''

// Automatically convert inline code.
renderer.codespan = (code) => {
  const match = code.match(/^&lt;\!(.*)&gt;(.*)$/)
  if (match)
    code = parseInlineCode(renderer.lang, match[1], match[2])
  return code.startsWith('<') ? code : `<code>${code}</code>`
}

// Support highlighting in markdown.
marked.setOptions({
  renderer,
  highlight: (code, lang) => hljs.highlightAuto(code, [lang]).value
})

// Dir to put generated files.
const gendir = path.join('out', 'Dist', 'docs')

// Iterate API documents for different languages.
for (let lang of langs) {
  renderer.lang = lang

  const langdir = path.join(gendir, lang)
  fs.emptyDirSync(langdir)

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
  const html = pug.renderFile('docs/layout/index.pug', {
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
    fs.ensureDirSync(guidedir)
    const html = pug.renderFile('docs/layout/guide.pug', {
      page: 'guide',
      doc: guide,
      filters: { 'css-minimize': cssMinimize },
    })
    fs.writeFileSync(path.join(guidedir, `${guide.id}.html`), html)
  }

  // Read API docs and generate HTML pages.
  for (let doc of docs) {
    const apidir = path.join(langdir, 'api')
    fs.ensureDirSync(apidir)
    // Output JSON files.
    fs.writeFileSync(path.join(apidir, `${doc.id}.json`),
                     JSON.stringify(doc, null, '  '))
    // Output HTML pages.
    const html = pug.renderFile('docs/layout/api.pug', {
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

// Create zip.
createZip()
  .addFile('out/Dist/docs', 'out/Dist/docs')
  .writeToFile(`yue_docs_${version}`)

// Load js-yaml from its browserify pack.
function loadYaml() {
  const vm = require('vm')
  const script = new vm.Script(fs.readFileSync('third_party/bundled_node_modules/yaml.js'))
  const sandbox = {}
  script.runInNewContext(sandbox)
  return sandbox.jsyaml
}

// Load pug.js from its browserify pack.
function loadPug() {
  const vm = require('vm')
  const script = new vm.Script(fs.readFileSync('third_party/bundled_node_modules/pug.js'))
  const sandbox = {fs: fs}
  script.runInNewContext(sandbox)
  return sandbox.require('pug')
}

// Load hightlight.js from its browserify pack.
function loadHighlight() {
  const vm = require('vm')
  const script = new vm.Script(fs.readFileSync('third_party/bundled_node_modules/highlight.js'))
  const sandbox = {}
  sandbox.window = sandbox
  script.runInNewContext(sandbox)
  return sandbox.hljs
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

  if (doc.enums)
    parseEnums(lang, doc)

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

// Parse declarations of enums.
function parseEnums(lang, doc) {
  for (const e of doc.enums) {
    if (lang == 'cpp')
      e.name = doc.name + '::' + e.name
    else
      e.name = `"${camelToDash(e.name)}"`
  }
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
  let stringTypes = ['Accelerator', 'KeyboardCode']
  if (node.type == 'refcounted' || wrappedTypes.includes(node.name))
    node.type = 'Class'
  else if (node.type == 'enum class' || stringTypes.includes(node.name))
    node.type = lang == 'lua' ? 'string' : 'String'
  else if (node.type == 'enum')
    node.type = lang == 'lua' ? 'integer' : 'Integer'
  else
    node.type = lang == 'lua' ? 'table' : 'Object'
}

// Parse the C++ signature string.
function parseSignature(lang, str) {
  const signature = {}
  let match = str.match(/^(\w+)\((.*)\)$/)
  if (match) {
    // Constructor type.
    signature.name = match[1]
    signature.parameters = match[2]
  } else {
    match = str.match(/^(.*) (\w+)\((.*)\).*$/)
    if (!match)
      console.error(`Failed to parse ${str}`)
    if (match[1] != 'void')
      signature.returnType = parseType(lang, match[1])
    signature.name = match[2]
    signature.parameters = match[3]
  }
  signature.name = parseName(lang, signature.name)
  signature.parameters = parseParameters(lang, signature.parameters)
  const parameters = signature.parameters.map((param) => param.name)
  signature.shortStr = `${signature.name}(${parameters.join(', ')})`
  if (lang == 'cpp')
    signature.str = str
  else
    signature.str = signature.shortStr
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
  let parameters = []
  let first = 0;
  let depth = 0
  for (let i = 0; i < str.length; ++i) {
    if (str[i] == '(') {
      depth++;
    } else if (str[i] == ')') {
      depth--;
    } else if (depth == 0 && str[i] == ',') {
      parameters.push(parseParam(lang, str.substring(first, i)))
      first = i + 1
    }
  }
  parameters.push(parseParam(lang, str.substring(first)))
  return parameters
}

// Parse param.
function parseParam(lang, str) {
  const space = str.lastIndexOf(' ')
  return {
    type: parseType(lang, str.substr(0, space).trim()),
    name: parseName(lang, str.substr(space + 1).trim())
  }
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
  // Get the matched type, do not use regexp as it can not do longest match.
  const leftAngle = type.name.indexOf('<')
  const rightAngle = type.name.lastIndexOf('>')
  if (leftAngle > 0 && rightAngle > leftAngle) {
    const matchedType = type.name.substring(leftAngle + 1, rightAngle)
    type.name = type.name.substr(0, leftAngle)
    if (type.name == 'std::tuple') {
      type.args = matchedType.split(',').map((t) => parseType(lang, t.trim()))
    } else if (type.name == 'scoped_refptr') {
      type.name = matchedType
    } else if (type.name == 'base::Optional' || type.name == 'absl::optional') {
      type.name = matchedType
      type.nullable = true
    }
    // For js we want to pass some more information for TypeScript.
    if (lang == 'js') {
      if (type.name == 'std::vector' || type.name == 'std::set')
        type.elementType = parseType(lang, matchedType)
    }
  }
  // No need to convert types for C++.
  let builtin = true
  if (lang == 'cpp') {
    let builtins = [ 'bool', 'float', 'char', 'uint32_t', 'unsigned', 'int',
                     'size_t', 'void' ]
    builtin = type.name.startsWith('std::') ||
              type.name.startsWith('base::') ||
              type.name.endsWith('...') ||
              builtins.includes(type.name)
  }
  // Convertbuilt-in types for differnt languages.
  if (lang == 'lua') {
    switch (type.name) {
      case 'bool': type.name = 'boolean'; break
      case 'float': type.name = 'number'; break
      case 'std::string': type.name = 'string'; break
      case 'std::wstring': type.name = 'string'; break
      case 'char': type.name = 'string'; break
      case 'uint32_t': type.name = 'integer'; break
      case 'unsigned': type.name = 'integer'; break
      case 'int': type.name = 'integer'; break
      case 'Dictionary': type.name = 'table'; break
      case 'Array': type.name = 'table'; break
      case 'Function': type.name = 'function'; break
      case 'Buffer': type.name = 'string'; break
      case 'Any': type.name = 'any'; break
      case 'std::function': type.name = 'function'; break
      case 'std::vector': type.name = 'table'; break
      case 'std::set': type.name = 'table'; break
      case 'base::FilePath': type.name = 'string'; break
      case 'base::Time': type.name = 'number'; break
      case 'base::Value': type.name = 'any'; break
      default: builtin = false
    }
  } else if (lang == 'js') {
    switch (type.name) {
      case 'bool': type.name = 'Boolean'; break
      case 'float': type.name = 'Number'; break
      case 'std::string': type.name = 'String'; break
      case 'std::wstring': type.name = 'String'; break
      case 'char': type.name = 'String'; break
      case 'uint32_t': type.name = 'Integer'; break
      case 'unsigned': type.name = 'Integer'; break
      case 'int': type.name = 'Integer'; break
      case 'Dictionary': type.name = 'Object'; break
      case 'Array': type.name = 'Array'; break
      case 'Function': type.name = 'Function'; break
      case 'Buffer': type.name = 'Buffer'; break
      case 'Any': type.name = 'Any'; break
      case 'std::function': type.name = 'Function'; break
      case 'std::vector': type.name = 'Array'; break
      case 'std::set': type.name = 'Array'; break
      case 'base::FilePath': type.name = 'String'; break
      case 'base::Time': type.name = 'Date'; break
      case 'base::Value': type.name = 'Any'; break
      default: builtin = false
    }
  }
  // Custom types usually have 1-to-1 maps.
  if (!builtin)
    type.id = type.name.toLowerCase().replace(/::/g, '_')
  // C++ uses full type as name.
  if (lang == 'cpp')
    type.name = str
  return type
}

// Convert inline code.
function parseInlineCode(lang, type, code) {
  switch (type) {
    case 'name': {
      return parseName(lang, code)
    }
    case 'method': {
      const [type, member, separator] = separateTypeAndMember(lang, code)
      const typeInfo = parseType(lang, parseName(lang, type))
      const signature = parseShortSignature(lang, member)
      return `<code><a class="type" href="${typeInfo.id}.html#${signature.id}">${typeInfo.name}${separator}${signature.str}</a></code>`
    }
    case 'type': {
      const info = parseType(lang, code)
      return `<code><a class="type" href="${info.id}.html">${info.name}</a></code>`
    }
    case 'enum class': {
      return parseEnumClass(lang, code)
    }
    case 'enum': {
      if (lang == 'cpp')
        return code
      const [type, member, separator] = separateTypeAndMember(lang, code)
      const typeInfo = parseType(lang, type)
      return `<code><a class="type" href="${typeInfo.id}.html#${parseName('lua', member)}">${typeInfo.name}${separator}${upperCaseToCamelCase(member)}</a></code>`
    }
    case '':
      return code
    default:
      throw Error(`Unknown type ${type} in inline code`)
  }
}

// Parse simplified signature string.
function parseShortSignature(lang, raw) {
  const match = raw.match(/^(\w+)\((.*)\)$/)
  if (!match)
    throw new Error(`Invalid simplified signature: ${raw}`)
  const name = parseName(lang, match[1])
  const parameters = []
  for (const p of match[2].split(',')) {
    if (p == '')
      continue
    const name = parseName(lang, p)
    if (name != '')
      parameters.push(name)
  }
  const str = `${name}(${parameters.length > 0 ? parameters.join(', ') : ''})`
  let id = parseName('lua', name)
  for (let param of parameters)
    id += '-' + parseName('lua', param)
  return {id, str}
}

// Convert enum class values.
function parseEnumClass(lang, name) {
  if (lang == 'cpp')
    return name
  return `"${camelToDash(name)}"`
}

// Convert a camelCase to dash string.
function camelToDash(str) {
  let ret = ''
  let prevLowerCase = false
  for (let s of str) {
    const isUpperCase = s.toUpperCase() == s
    if (isUpperCase && prevLowerCase)
      ret += '-'
    ret += s
    prevLowerCase = !isUpperCase
  }
  return ret.replace(/-+/g, '-').toLowerCase()
}

// Convert a UPPER_CASE to camelCase.
function upperCaseToCamelCase(str) {
  return str.toLowerCase().split('_')
            .reduce((a, b) => a + b.charAt(0).toUpperCase() + b.slice(1))
}

// Separator type and member.
function separateTypeAndMember(lang, code) {
  let parts
  let separator
  for (separator of ['.', '::']) {
    if (code.includes(separator)) {
      parts = code.split(separator)
      break
    }
  }
  if (lang != 'cpp')
    separator = '.'
  return parts.concat(separator)
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
  const signature = parseSignature('lua', str)
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
