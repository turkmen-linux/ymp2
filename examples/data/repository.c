#include <core/ymp.h>

#include <data/repository.h>

#include <utils/file.h>
#include <utils/string.h>

int main(int argc, char** argv){
    (void) argc; (void)argv;
    (void)ymp_init();
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

    return 0;
}