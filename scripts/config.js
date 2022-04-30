const {clang, targetCpu, targetOs} = require('./common')

// The version of nuget WebView2 package.
const webview2Version = '1.0.622.22'

// The versions of language bindings to build against.
const nodeVersions = [
  '14.19.1',
  '16.15.0',
  '17.9.0',
]
const electronVersions = [
  '15.5.2',
  '16.2.3',
  '17.4.1',
  '18.1.0',
]
const luaVersions = [
  '5.3.6',
  '5.4.3',
  '5.1.5',
]

// The version of gn.
const gnVersion = '0.8.2'

// The common build configurations.
const gnConfig = [
  `target_cpu="${targetCpu}"`,
  `lua_version="v${luaVersions[0]}"`,
  'use_allocator="none"',
  'use_allocator_shim=false',
  'use_partition_alloc=false',
  'fatal_linker_warnings=false',
]
if (clang) {
  gnConfig.push('is_clang=true',
                'clang_update_script="//building/tools/update-clang.py"')
} else {
  gnConfig.push('is_clang=false')
}
if (targetOs == 'mac') {
  gnConfig.push('mac_deployment_target="10.10.0"',
                'mac_sdk_min="11.0"',
                'use_xcode_clang=true')
} else if (targetOs == 'win') {
  gnConfig.push(`webview2_version="${webview2Version}"`)
}

// The build configuration for sysroot.
const gnSysrootConfig = [
  'use_sysroot=true',
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
