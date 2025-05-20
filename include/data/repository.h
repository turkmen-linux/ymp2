typedef struct {
    char* uri;
    char** packages;
    char** sources;
} Repository;


Repository* repository_new();
void repository_load_from_index(Repository* repo, const char* path);