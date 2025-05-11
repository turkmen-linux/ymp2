# Test files

arch = run_command('uname', '-m' ,check:true).stdout().strip()
start = meson.current_source_dir()+'/../src/start/'+arch+'.s'

foreach src : run_command('find', '.', '-type', 'f', '-iname', '*.c',check:true).stdout().strip().split('\n')
    name = src.replace('./','')
    name = name.replace('.c','')
    name = name.replace('/','_')
    executable(name, [src, start], link_with: libymp)
endforeach

