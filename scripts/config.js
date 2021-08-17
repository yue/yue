const {clang, targetCpu, targetOs} = require('./common')

// The version of nuget WebView2 package.
const webview2Version = '1.0.622.22'

// The versions of Node.js to build against.
const nodeVersions = [
  '12.22.4',
  '14.17.4',
  '16.6.0',
]
const electronVersions = [
  '12.0.16',
  '13.1.8',
  '14.0.0-beta.19',
  '15.0.0-alpha.4',
]

// The version of gn.
const gnVersion = '0.7.1'

// The common build configurations.
const gnConfig = [
  `target_cpu="${targetCpu}"`,
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
                'use_xcode_clang=true')
  if (targetCpu == 'arm64')
    gnConfig.push('mac_sdk_min="11.0"')
  else
    gnConfig.push('mac_sdk_min="10.15"')
} else if (targetOs == 'win') {
  gnConfig.push(`webview2_version="${webview2Version}"`)
}

// The build configuration for sysroot.
const gnSysrootConfig = [
  'use_sysroot=true',
  'target_sysroot_dir="//third_party/"',
  'debian_platform="stretch"'
]

module.exports = {
  webview2Version,
  nodeVersions,
  electronVersions,
  gnVersion,
  gnConfig,
  gnSysrootConfig,
}
