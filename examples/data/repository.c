#include <stdio.h>

#include <core/ymp.h>

#include <data/repository.h>

#include <utils/file.h>
#include <utils/string.h>

int main(int argc, char** argv){
    (void) argc; (void)argv;
    Ymp* ymp = ymp_init();
    Repository *repo = repository_new();
    char* index =  "index:\n"
    "  address: http://example.com/$uri\n"
    "  package:\n"
    "    name: hello\n"
    "    version: 1.0\n"
    "    release: 1\n"
    "    uri: /hello.ymp\n"
    "  source:\n"
    "    name: hello\n"
    "    version: 1.0\n"
    "    release: 1\n"
    "    uri: /hello.ymp\n";
    writefile("index.yaml", index);
    repository_load_from_index(repo, "index.yaml");

    Package** pkgs = repo->packages;
    for(size_t i=0; i< repo->package_count;i++){
        printf("%s %d\n", pkgs[i]->name, pkgs[i]->is_source);
    }
    Repository *repo2 = repository_new();
    char* index2 = "index:\n"
      "  address: https://gitlab.com/turkman/packages/binary-repo/-/raw/master/$uri\n"
      "  package:\n"
      "    name: sex\n"
      "    version: 1.0\n"
      "    release: 1\n"
      "    uri: /s/sex/sex_1.0_1_x86_64.ymp\n";
    repository_load_from_data(repo2, index2);
    variable_set_value(ymp->variables, "DESTDIR", "./");
    repository_download_package(repo2, "sex", false);
    return 0;
}
