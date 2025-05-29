project(
  'ymp', 
  'c',
  version: '2.0.0'
)
architecture = run_command('uname', '-m', check:true).stdout().strip()

add_project_arguments('-I'+meson.current_source_dir()+'/include', '-pthread',  language: 'c')
add_project_arguments('-fvisibility=hidden', '-Dvisible=__attribute__((visibility("default")))',  language: 'c')
add_project_arguments('-DPATH_MAX=1024', '-pthread', '-fPIC', language: 'c')
add_project_arguments('-Wall', '-Wextra', '-Werror',  language: 'c')

if get_option('debug')
    add_project_arguments('-DDEBUG',  language: 'c')
endif

srcs = []
deps = [
    dependency('libarchive', required:true),
    dependency('libcurl', required:true),
    dependency('openssl', required:true)
]


configure_file(
  input: 'include/config.h.in',
  output: 'config.h',
  configuration: {
    'VERSION': meson.project_version(),
    'DEBUG': get_option('debug') ? '1' : '0',
    'BUILD_DIR': '/tmp/ymp-build/',
    'DISTRODIR': '/lib/turkman',
    'STORAGE': '/var/lib/ymp/',
  }
)

foreach src : run_command('find', 'src', '-type', 'f', '-iname', '*.c',check:true).stdout().strip().split('\n')
    srcs += src
endforeach

subdir('src')

start = [meson.current_source_dir()+'/src/start/'+architecture+'.s']
nostdlib = ['-nostdlib', '-lc']
if not get_option('startfiles')
    start = []
    nostdlib = []
endif

# Main library
libymp = library('ymp', srcs, dependencies: deps, c_args: ['-DLIBYMP', '-Dinvisible=extern'], link_args: nostdlib)

subdir('docs')
subdir('examples')

executable('ymp-cli', ['src/core/cli.c']+start, link_with: libymp, link_args: nostdlib)
