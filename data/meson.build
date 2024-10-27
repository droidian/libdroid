schemas = [
  'eu.medesimo.Libdroid.gschema.xml'
]

schemas_dir = get_option('prefix') / get_option('datadir') / 'glib-2.0' / 'schemas'

install_data(schemas, install_dir: schemas_dir)
#meson.add_install_script('glib-compile-schemas', schemas_dir)