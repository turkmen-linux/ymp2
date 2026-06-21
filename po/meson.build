if get_option('gettext')
    if get_option('musl')
        l_args += ['-lintl']
    endif
    i18n = import('i18n')
    i18n.gettext(meson.project_name(), preset: 'glib')
    add_project_arguments('-DYMP_GETTEXT', language: 'c')
    add_project_arguments('-DLOCALEDIR="'+get_option('prefix')+'/'+get_option('localedir')+'"', language: 'c')
endif