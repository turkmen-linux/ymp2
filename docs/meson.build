# Doxygen configuration
doxygen = find_program('doxygen', required: false)

if doxygen.found()
    doxygen_output = 'docs'
    doxygen_file = meson.current_source_dir()+'/Doxyfile'

    # Custom target to generate documentation
    doxygen_target = custom_target('doxygen',
        command: [doxygen, doxygen_file],
        output: 'index.html',
        build_by_default: true
    )
endif
