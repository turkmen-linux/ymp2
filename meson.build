project(
  'ymp', 
  'c',
  version: '2.0.0'
)

add_project_arguments('-I'+meson.current_source_dir()+'/include', '-pthread',  language: 'c')
add_project_arguments('-fvisibility=hidden', '-Dvisible=__attribute__((visibility("default")))',  language: 'c')
add_project_arguments('-DPATH_MAX=1024', '-pthread', '-fPIC', language: 'c')
add_project_arguments('-Wall', '-Wextra', '-Werror',  language: 'c')


srcs = []
deps = [
    dependency('libarchive', required:true),
    dependency('libcurl', required:true),
    dependency('openssl', required:true)
]


foreach src : run_command('find', 'src', '-type', 'f', '-iname', '*.c',check:true).stdout().strip().split('\n')
    srcs += src
endforeach

subdir('src')


# Main library
libymp = library('ymp', srcs, dependencies: deps, c_args: ['-DLIBYMP'], link_args: ['-nostdlib', '-lc'])

subdir('docs')
subdir('examples')

executable('ymp-cli', ['src/core/cli.c', start], link_with: libymp, link_args: ['-nostdlib', '-lc'])
