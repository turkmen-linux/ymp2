# Test files
if get_option('library')
    foreach src : run_command('find', '.', '-type', 'f', '-iname', '*.c',check:true).stdout().strip().split('\n')
        name = src.replace('./','')
        name = name.replace('.c','')
        name = name.replace('/','_')
        executable(name,
            [src]+start,
            link_with: libymp,
            link_args: nostdlib,
        )
    endforeach
endif
if get_option('misc-langs')
    vala_example = custom_target('ymp-vala',
        input: 'ymp.vala',
        output: 'ymp-vala',
        build_by_default: true,
        depends : [libymp],
        command: ['valac',
            '--pkg', 'ymp',
            '--vapidir=../include/',
            '-X', '-lymp',
            '-X', '-L'+meson.current_build_dir()+'/../',
            '-X', '-I../include',
            '@INPUT@',
            '--output=@OUTPUT@'
        ]
    )
    cpp_example = custom_target('ymp-cpp',
        input: 'ymp.cpp',
        output: 'ymp-cpp',
        build_by_default: true,
        depends : [libymp],
        command: ['g++',
            '-lymp',
            '-L'+meson.current_build_dir()+'/../',
            '-I../include',
            '@INPUT@',
            '--output=@OUTPUT@'
        ]
    )
endif
