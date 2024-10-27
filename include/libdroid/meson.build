libdroid_headers = [
  'leds.h',
  'libdroid.h',
]

version_split = meson.project_version().split('.')
version_conf = configuration_data()
version_conf.set('VERSION', meson.project_version())
version_conf.set('MAJOR_VERSION', version_split[0])
version_conf.set('MINOR_VERSION', version_split[1])
version_conf.set('MICRO_VERSION', version_split[2])

configure_file(
          input: 'libdroid-version.h.in',
         output: 'libdroid-version.h',
  configuration: version_conf,
        install: true,
    install_dir: get_option('includedir') / 'libdroid'
)

gnome = import('gnome')
libdroid_gir = gnome.generate_gir(libdroid_lib,
            nsversion: api_version,
            namespace: 'Droid',
        symbol_prefix: 'droid',
    identifier_prefix: 'Droid',
             sources:  libdroid_headers,
             includes: [ 'Gio-2.0' ],
              install: true,
)


install_headers(libdroid_headers, subdir: 'libdroid')