const {clang, targetCpu, targetOs} = require('./common')

// The version of nuget WebView2 package.
const webview2Version = '1.0.622.22'

// The versions of language bindings to test against.
const nodeVersions = [
  '14.19.1',
  '16.15.0',
  '18.14.0',
  '19.6.0',
]
const electronVersions = [
  '21.4.1',
  '22.3.0',
  '23.1.0',
]
const luaVersions = [
  '5.3.6',
  '5.4.3',
  '5.1.5',
]

// The version of gn.
const gnVersion = '0.9.4'

// The common build configurations.
const gnConfig = [
  `is_clang=${clang}`,
  `target_cpu="${targetCpu}"`,
  `lua_version="v${luaVersions[0]}"`,
  'fatal_linker_warnings=false',
]
if (targetOs == 'win') {
  gnConfig.push(`webview2_version="${webview2Version}"`)
}

// The build configuration for sysroot.
const gnSysrootConfig = [
  'use_sysroot=true',
  'debian_platform="sid"',
  'target_sysroot_dir="//third_party/"',
]

module.exports = {
  webview2Version,
  nodeVersions,
  electronVersions,
  luaVersions,
  gnVersion,
  gnConfig,
  gnSysrootConfig,
}
