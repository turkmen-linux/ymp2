if get_option('gettext')
    i18n = import('i18n')
    i18n.gettext(meson.project_name(), preset: 'glib')
    add_project_arguments('-DYMP_GETTEXT', language: 'c')
    add_project_arguments('-DLOCALEDIR="'+get_option('prefix')+'/'+get_option('localedir')+'"', language: 'c')
endif