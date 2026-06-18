project(
  'DiscImageCreator',
  'c',
  'cpp',
  license : 'Apache-2.0',
  default_options : ['buildtype=release', 'prefix=/usr', 'cpp_std=c++11']
)

conf_data = configuration_data()

if target_machine.system() in ['linux', 'darwin']
  build_date = run_command('date', '+%Y%m%d').stdout().strip()
  build_time = run_command('date', '+%H%M%S').stdout().strip()
  libarchive_dep = dependency('libarchive', required: true, method: 'pkg-config')
  zlib_dep = dependency('zlib', required: true, method: 'pkg-config')
  openssl_dep = dependency('openssl', required: true, method: 'pkg-config')
  add_project_arguments('-include', 'defineForLinux.h', language : 'cpp')
  inc = [include_directories('DiscImageCreator/'), include_directories('DiscImageCreator/_external'), include_directories('DiscImageCreator/_linux')]
endif

deps = [libarchive_dep, zlib_dep, openssl_dep]

if host_machine.system() == 'darwin'
  applefw_dep = dependency(
    'appleframeworks',
    modules: ['CoreFoundation', 'IOKit'],
    required: true
  )
  deps += applefw_dep
endif

conf_data.set('build_date', build_date)
conf_data.set('build_time', build_time)

builddatetime_h = configure_file(
  input: 'DiscImageCreator/buildDateTime.h.in',
  output: 'buildDateTime.h',
  configuration: conf_data,
)

executable(
  meson.project_name(),
  sources: [
    builddatetime_h,
    'DiscImageCreator/DiscImageCreator.cpp',
    'DiscImageCreator/_external/abgx360.c',
    'DiscImageCreator/_external/aes.c',
    'DiscImageCreator/_external/aesni.c',
    'DiscImageCreator/_external/crc16ccitt.c',
    'DiscImageCreator/_external/crc32.c',
    'DiscImageCreator/_external/ECCEDC.cpp',
    'DiscImageCreator/_external/md5c.c',
    'DiscImageCreator/_external/NonStandardFunction.cpp',
    'DiscImageCreator/_external/platform_util.c',
    'DiscImageCreator/_external/prngcd.cpp',
    'DiscImageCreator/_external/reedSolomonGF64.cpp',
    'DiscImageCreator/_external/rijndael-alg-fst.c',
    'DiscImageCreator/_external/sha1.c',
    'DiscImageCreator/_external/sha224-256.c',
    'DiscImageCreator/_external/sha384-512.c',
    'DiscImageCreator/_external/tinyxml2.cpp',
    'DiscImageCreator/_external/xxhash.c',
    'DiscImageCreator/_external/libunshield/component.c',
    'DiscImageCreator/_external/libunshield/converter.c',
    'DiscImageCreator/_external/libunshield/directory.c',
    'DiscImageCreator/_external/libunshield/file.c',
    'DiscImageCreator/_external/libunshield/file_group.c',
    'DiscImageCreator/_external/libunshield/helper.c',
    'DiscImageCreator/_external/libunshield/libunshield.c',
    'DiscImageCreator/_external/libunshield/log.c',
    'DiscImageCreator/_external/ps3auth/auth_service.c',
    'DiscImageCreator/_external/ps3auth/crypto_backend_openssl.c',
    'DiscImageCreator/_external/ps3auth/data_unwrap.c',
    'DiscImageCreator/_external/ps3auth/drive_proto.c',
    'DiscImageCreator/_external/ps3auth/drive_transport_execscsi.cpp',
    'DiscImageCreator/_external/ps3auth/odd_ctx.c',
    'DiscImageCreator/_external/ps3auth/odd_tables.c',
    'DiscImageCreator/_external/ps3auth/session_keys.c',
    'DiscImageCreator/_linux/defineForLinux.cpp',
    'DiscImageCreator/calcHash.cpp',
    'DiscImageCreator/check.cpp',
    'DiscImageCreator/convert.cpp',
    'DiscImageCreator/execIoctl.cpp',
    'DiscImageCreator/execScsiCmd.cpp',
    'DiscImageCreator/execScsiCmdforCD.cpp',
    'DiscImageCreator/execScsiCmdforCDCheck.cpp',
    'DiscImageCreator/execScsiCmdforDVD.cpp',
    'DiscImageCreator/execScsiCmdforFileSystem.cpp',
    'DiscImageCreator/execTapeCmd.cpp',
    'DiscImageCreator/fix.cpp',
    'DiscImageCreator/get.cpp',
    'DiscImageCreator/init.cpp',
    'DiscImageCreator/output.cpp',
    'DiscImageCreator/outputFileSystem.cpp',
    'DiscImageCreator/outputIoctlLog.cpp',
    'DiscImageCreator/outputScsiCmdLog.cpp',
    'DiscImageCreator/outputScsiCmdLogforCD.cpp',
    'DiscImageCreator/outputScsiCmdLogforDVD.cpp',
    'DiscImageCreator/set.cpp',
    'DiscImageCreator/xml.cpp',
  ],
  include_directories: inc,
  dependencies: deps,
  cpp_args: [ '-Wno-unknown-pragmas'],
  c_args: [ '-Wno-unknown-pragmas'],
  install: true,
  install_dir: get_option('bindir'),
)

# copy assets into build directory for easier testing
fs = import('fs')
fs.copyfile('Release_ANSI/driveOffset.txt')
fs.copyfile('Release_ANSI/default.dat')

install_data(
  sources: [
    'Release_ANSI/default.dat',
    'Release_ANSI/driveOffset.txt',
  ],
  install_dir: get_option('datadir') / meson.project_name(),
)
