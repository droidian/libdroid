api_version = '0'
soversion = '0'

libdroid_sources = [
  'binder.c',
  'leds.c',
  'leds-backend.c',
  'leds-backend-aidl.c',
  'leds-backend-hidl.c',
  'settings.c',
]

libdroid_deps = [
  dependency('gio-2.0'),
  dependency('libgbinder'),
]

libdroid_lib = shared_library('droid-' + api_version,
  libdroid_sources,
  soversion: soversion,
  dependencies: libdroid_deps,
  link_args: ['-Wl,--version-script,' + meson.current_source_dir() / 'libdroid.syms'],
  install: true,
)

pkg = import('pkgconfig')
pkg.generate(
  description: 'GLib-friendly helpers to interact with Android HALs',
    libraries: libdroid_lib,
         name: 'libdroid',
     filebase: 'libdroid-' + api_version,
      version: meson.project_version(),
      subdirs: 'libdroid',
     requires: ['gio-2.0', 'libgbinder'],
  install_dir: get_option('libdir') / 'pkgconfig'
)