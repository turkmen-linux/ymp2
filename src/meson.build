# Generate ctx.c file
ctx_c = '#include <core/ymp.h>\n'
foreach ops : run_command('find', 'operations','-type','f',check:true).stdout().strip().split('\n')
    name = ops.split('/')[-1].split('.')[0]
    ctx_c = ctx_c + 'extern void '+name+'_init(OperationManager *manager);\n'
endforeach
ctx_c += '\nvoid ctx_init(OperationManager *manager){\n'
foreach ops : run_command('find', 'operations','-type','f',check:true).stdout().strip().split('\n')
    name = ops.split('/')[-1].split('.')[0]
    ctx_c += '  '+name+'_init(manager);\n'
endforeach
ctx_c += '}\n\n'
run_command('cc', '../data/urcc.c', '-o', meson.current_build_dir()+'/urcc', check:true)
foreach data : run_command('find', '../data','-type','f', '-iname', '*.sh', '-o', '-iname', '*.txt' ,check:true).stdout().strip().split('\n')
    name = data.substring(8).replace('.','_').replace('-','_')
    ctx_c += run_command(meson.current_build_dir()+'/urcc', data, name).stdout().strip()+'\n'
endforeach

ctx_c += '#include <string.h>\n'
ctx_c += '__attribute__((section(".resource"))) char* resource(const char* name){\n'
foreach data : run_command('find', '../data','-type','f', '-iname', '*.sh', '-o', '-iname', '*.txt' ,check:true).stdout().strip().split('\n')
    name = data.substring(8).replace('.','_').replace('-','_')
    data = data.substring(7)
    ctx_c += '    if(strcmp(name, ":'+data+'") == 0) {\n'
    ctx_c += '        return (char*)'+name+';\n' 
    ctx_c += '    }\n'
endforeach
ctx_c += 'return "";\n}'

configure_file(
  input: 'ctx.c.in',
  output: 'ctx.c',
  configuration: {
    'CTX': ctx_c
  }
)

srcs += [meson.current_build_dir()+'/ctx.c']

foreach src : run_command('find', '.', '-type', 'f', '-iname', '*.c',check:true).stdout().strip().split('\n')
    srcs += 'src/' + src
endforeach

if get_option('library')
    add_project_arguments('-fvisibility=hidden', '-Dvisible=__attribute__((visibility("default")))',  language: 'c')
else
    add_project_arguments('-fvisibility=hidden', '-Dvisible=', language: 'c')
endif

add_project_arguments('-I'+meson.current_source_dir()+'/../include',  language: 'c')
add_project_arguments('-DPATH_MAX=1024', '-pthread', '-fPIC', '-fno-plt', language: 'c')
