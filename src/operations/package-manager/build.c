#include <config.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <core/logger.h>
#include <core/ymp.h>
#include <data/build.h>
#include <utils/file.h>
#include <utils/process.h>
#include <utils/string.h>

static char *git_clone(const char *git_url) {
    char *destdir = get_value("DESTDIR");
    char *git = which("git");
    char *ret = build_string("%s/%s/.cache/%s", destdir, BUILD_DIR, basename((char *) git_url));
    char *args[] = { git, "clone", "--depth=1", (char *) git_url, ret, NULL };
    int rc = run_args(args);
    free(git);
    if (rc == 0) {
        return ret;
    }
    free(ret);
    return NULL;
}

static int build(void **args) {
    for (size_t i = 0; args[i]; i++) {
        char *target = get_value("output");

        if (strlen(target) == 0) {
            target = args[i];
        }
        char *git_path = NULL;
        if (startswith(args[i], "git://") || endswith(args[i], ".git")) {
            git_path = git_clone(args[i]);
            if (git_path) {
                args[i] = git_path;
            } else {
                warning(_("Failed to fetch git repository\n"));
                return 1;
            }
        }
        char *cache = build_source_from_path(args[i]);
        // create source package
        char *spkg = create_package(cache);
        char *sname = ympbuild_source_filename(args[i]);
        char *target_sfile = build_string("%s/%s", target, sname);
        (void) move_file(spkg, target_sfile);

        debug("Bulid cache %s\n", cache);
        if (!isdir(cache)) {
            return 1;
        }
        const char *fbuild = build_binary_from_path(cache);
        if (fbuild == NULL) {
            free(cache);
            return 1;
        }
        char *pkg = create_package(fbuild);
        debug("Output package %s %s %d\n", pkg, args[i], i);

        char *pname = ympbuild_package_filename(args[i]);
        // move binary package file to output
        char *target_pfile = build_string("%s/%s", target, pname);
        create_dir(target);
        (void) move_file(pkg, target_pfile);
        // free memory
        free(pkg);
        free(pname);
        free(sname);
        free(cache);
        // install package
        if (get_bool("install")) {
            char *iargs[] = { target_pfile, NULL };
            int status = operation_main(global->manager, "install", iargs);
            if (status != 0) {
                free(target_pfile);
                free(target_sfile);
                return status;
            }
        }
        free(target_pfile);
        free(target_sfile);
        if (git_path) {
            free(git_path);
        }
    }
    return 0;
}

void build_init(OperationManager *manager) {
    Operation op;
    op.name = "build";
    op.description = _("Build package");
    op.alias = "bi:make";
    op.help = help_new();
    help_add_parameter(op.help, "--install", _("install after build"));
    help_add_parameter(op.help, "--output", _("output directory for built packages"));
    help_add_string(op.help, _("FILE... : ympbuild files or directories to build"));
    op.call = (callback) build;
    op.min_args = 1;
    operation_register(manager, op);
}
