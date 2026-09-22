#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <core/logger.h>
#include <core/ymp.h>
#include <utils/array.h>
#include <utils/file.h>
#include <utils/gui.h>
#include <utils/string.h>

static char *str_or_def(const char *val, const char *def) {
    char *f = get_value(val);
    if (f != NULL && strlen(f) > 0) {
        return strdup(f);
    }
    return strdup(def);
}

static char *gitconfig_value(const char *variable) {
    const char *home = getenv("HOME");
    if (!home) {
        return strdup("");
    }
    char *gitconfig = build_string("%s/.gitconfig", home);
    char *ret = strdup("");
    if (isfile(gitconfig)) {
        char *data = readfile(gitconfig);
        if (data) {
            char **lines = split(data, "\n");
            for (size_t i = 0; lines[i]; i++) {
                if (strstr(lines[i], variable) && strstr(lines[i], "=")) {
                    char **parts = split(lines[i], "=");
                    if (parts[1]) {
                        char *t = trim(parts[1]);
                        free(ret);
                        ret = t;
                    }
                    for (size_t j = 0; parts[j]; j++) {
                        free(parts[j]);
                    }
                    free(parts);
                    break;
                }
                free(lines[i]);
            }
            free(lines);
            free(data);
        }
    }
    free(gitconfig);
    return ret;
}

static int template_main(char **args) {
    (void) args;
    char *name = str_or_def("name", "");
    if (strlen(name) == 0) {
        warning(_("Variable '%s' is not defined. please use --%s\n"), "name", "name");
        free(name);
        return 1;
    }
    char *version = str_or_def("version", "1.0");
    char *homepage = str_or_def("homepage", "https://example.org");
    char *description = str_or_def("description", "Package description missing");
    char *email_d = gitconfig_value("email");
    char *maint_d = gitconfig_value("name");
    char *email = str_or_def("email", email_d);
    char *maintainer = str_or_def("maintainer", maint_d);
    char *license = str_or_def("license", "GPLv3");
    char *source = str_or_def("source", " ");
    char *depends = str_or_def("depends", " ");
    char *makedepends = str_or_def("makedepends", " ");
    free(email_d);
    free(maint_d);

    array *out = array_new();
    array_add(out, "#!/usr/bin/env bash\n");
    array_add(out, build_string("name='%s'\n", name));
    array_add(out, "release='1'\n");
    array_add(out, build_string("version='%s'\n", version));
    array_add(out, build_string("url='%s'\n", homepage));
    array_add(out, build_string("description='%s'\n", description));
    array_add(out, build_string("email='%s'\n", email));
    array_add(out, build_string("maintainer='%s'\n", maintainer));
    array_add(out, build_string("license=('%s')\n", license));
    array_add(out, build_string("source=('%s')\n", source));
    array_add(out, build_string("depends=(%s)\n", depends));
    array_add(out, build_string("makedepends=(%s)\n", makedepends));
    array_add(out, "sha256sums=()\n");
    array_add(out, "group=()\n");
    array_add(out, "uses=()\n");
    array_add(out, build_string("arch=('%s')\n\n", ARCH));
    array_add(out, build_string("cd $name-$version\n\n", ""));
    const char *buildtype = get_value("build-type");
    if (!buildtype || strlen(buildtype) == 0 || iseq(buildtype, "autotool")) {
        array_add(out, "setup () {\n    ./configure --prefix=/usr \\\n     --libdir=/usr/lib64/\n}\n\n");
        array_add(out, "build () {\n    make $jobs\n}\n\n");
        array_add(out, "package () {\n    make install $jobs\n}\n\n");
    } else if (iseq(buildtype, "cmake")) {
        array_add(out, "setup () {\n    mkdir build\n    cd build\n    cmake -DCMAKE_INSTALL_PREFIX=/usr \\\n        -DCMAKE_INSTALL_LIBDIR=/usr/lib64 ..\n}\n\n");
        array_add(out, "build () {\n    cd build\n    make $jobs\n}\n\n");
        array_add(out, "package () {\n    cd build\n    make install $jobs\n}\n\n");
    } else if (iseq(buildtype, "meson")) {
        array_add(out, "setup () {\n    meson setup build --prefix=/usr \\\n        --libdir=/usr/lib64/\n        -Ddefault_library=both\n}\n\n");
        array_add(out, "build () {\n    ninja -C build $jobs\n}\n\n");
        array_add(out, "package () {\n    ninja -C build install $jobs\n}\n\n");
    } else {
        array_add(out, "setup () {\n    :\n}\n\nbuild () {\n    :\n}\n\npackage () {\n    :\n}\n\n");
    }
    size_t len = 0;
    char **parts = array_get(out, &len);
    char *data = join("", parts);
    for (size_t i = 0; i < len; i++) {
        free(parts[i]);
    }
    free(parts);
    array_unref(out);

    if (get_bool("ask")) {
        print("%s\n", _("Please check ympbuild:"));
        print("%s\n", data);
        if (!gui_yes_no(_("Template"), _("Is it OK ?"), false)) {
            free(data);
            goto template_free;
        }
    }
    char *output = get_value("output");
    char target[4096];
    if (output && strlen(output) > 0) {
        if (output[0] == '/') {
            snprintf(target, sizeof(target), "%s", output);
        } else {
            char cwd[4096];
            if (!getcwd(cwd, sizeof(cwd))) {
                snprintf(target, sizeof(target), "%s", output);
            } else {
                char *t = build_string("%s/%s", cwd, output);
                strncpy(target, t, sizeof(target) - 1);
                target[sizeof(target) - 1] = '\0';
                free(t);
            }
        }
    } else {
        if (!getcwd(target, sizeof(target))) {
            target[0] = '.';
            target[1] = '\0';
        }
    }
    info(_("Creating template: %s\n"), target);
    create_dir(target);
    char *ympbuild = build_string("%s/ympbuild", target);
    writefile(ympbuild, data);
    free(ympbuild);
    free(data);
template_free:
    free(name);
    free(version);
    free(homepage);
    free(description);
    free(email);
    free(maintainer);
    free(license);
    free(source);
    free(depends);
    free(makedepends);
    return 0;
}

void template_init(OperationManager *manager) {
    Operation op;
    op.name = "template";
    op.alias = "t";
    op.description = _("Create ympbuild from template.");
    op.min_args = 0;
    op.help = help_new();
    help_add_parameter(op.help, "--name", _("package name"));
    help_add_parameter(op.help, "--version", _("package version"));
    help_add_parameter(op.help, "--homepage", _("package homepage"));
    help_add_parameter(op.help, "--description", _("package description"));
    help_add_parameter(op.help, "--depends", _("package dependencies"));
    help_add_parameter(op.help, "--makedepends", _("package build dependencies"));
    help_add_parameter(op.help, "--email", _("package creator email"));
    help_add_parameter(op.help, "--maintainer", _("package maintainer"));
    help_add_parameter(op.help, "--license", _("package license"));
    help_add_parameter(op.help, "--source", _("package source"));
    help_add_parameter(op.help, "--build-type", _("package build-type (autotool cmake meson)"));
    help_add_parameter(op.help, "--output", _("ympbuild output directory"));
    op.call = (callback) template_main;
    operation_register(manager, op);
}
