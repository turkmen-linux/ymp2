# Test files
if get_option('library')
    foreach src : run_command('find', '.', '-type', 'f', '-iname', '*.c',check:true).stdout().strip().split('\n')
        name = src.replace('./','')
        name = name.replace('.c','')
        name = name.replace('/','_')
        executable(name, [src]+start, link_with: libymp, link_args: nostdlib)
    endforeach
endif

