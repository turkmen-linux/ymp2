#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <dirent.h>

#include <core/operations.h>
#include <core/variable.h>
#include <core/logger.h>
#include <core/ymp.h>


#include <utils/string.h>
#include <utils/file.h>

static Ymp* y;


typedef struct {
    size_t start;
    size_t end;
} Range;


#define swrite(A, B) write(A, B, strlen(B))

#define BUFFER_SIZE 1024*1024

static void serve_file(int client_fd, FILE* file, size_t fsize, size_t start, size_t end){
    (void)end;
    char buffer[BUFFER_SIZE]; //1mb buffer
    int bytesRead = 0;
    // send header
    const char* header = "HTTP/1.1 200 OK\n" \
        "Content-Type: text/plain\n" ;
    if(swrite(client_fd, header) < 0){
        return;
    }
    // send size
    char* msg = build_string("Content-Length: %ld\n", fsize);
    if(swrite(client_fd, msg) < 0){
        return;
    }
    // finish response body
    if(swrite(client_fd, "\n") < 0){
        return;
    }
    free(msg);
    // go start bit
    if(fsize > start){
        fseek(file, start, SEEK_SET);
    }
    // send content
    while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        if(write(client_fd, buffer, bytesRead) < 0){
            break;
        }
    }
}

static void list_directory(int client_fd, const char* dir_path, const char* serve) {
    DIR *dir = opendir(dir_path);
    if (dir == NULL) {
        printf("Failed to open directory: %s\n", dir_path);
        return;
    }

    // Send HTTP header
    const char* header = "HTTP/1.1 200 OK\n" \
                         "Content-Type: text/html\n\n";
    if (write(client_fd, header, strlen(header)) < 0) {
        closedir(dir);
        return;
    }

    // Start HTML response
    char* msg = build_string("<html><body><h1>Directory Listing for /%s</h1><ul>\n", dir_path+strlen(serve));
    if(swrite(client_fd, msg) < 0){
        return;
    }
    free(msg);

    // Read directory entries
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        const char* em = "&#x1F4C1;";
        char* file_path = build_string("%s/%s", dir_path, entry->d_name);
        char* file_link = build_string("/%s/%s", dir_path+strlen(serve), entry->d_name);
        if(isfile(file_path)){
            em = "&#x1F4C4;";
        }
        free(file_path);
        size_t skip = 0;
        if(strlen(file_link) > 2){
            for(skip=0; file_link[skip+1] && file_link[skip] == '/' && file_link[skip+1] == '/'; skip++){}
        }
        char* msg = build_string("<br>%s<a href=\"%s\">%s</a></li>\n", em, file_link+skip, entry->d_name);
        free(file_link);
        if(swrite(client_fd, msg) < 0){
            return;
        }
        free(msg);
    }

    // Close the unordered list and HTML tags
    msg = strdup("</ul></body></html>\n");
    if(swrite(client_fd, msg) < 0){
        return;
    }
    free(msg);

    closedir(dir);
}

static void* handle_client(void* arg){
    const char* serve = variable_get_value(y->variables, "source");
    int client_fd = *(int*)arg;
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    // Read the request from the client
    char* res;
    char* path = "/";
    int bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
    if(bytes_read < 0){
        printf("Failed to read from client");
        close(client_fd);
        return NULL;
    }
    // parse request body
    char** lines = split(buffer, "\n");
    Range r;
    r.start = 0;
    r.end = 0;
    for(size_t i=0; lines[i]; i++){
        debug("fd: %d line: %ld data: %s\n", client_fd, i, lines[i]);
        // fetch get request url
        if(strncmp(lines[i], "GET ", 4) == 0){
            path = strdup(lines[i]+4);
            // use first word before space
            for(size_t j=0; path[j]; j++){
                if(path[j] == ' '){
                    path[j] = '\0';
                    break;
                }
            }
        }
        if(strncmp(lines[i], "Range: bytes=", 13) == 0){
            char* range_str = lines[i]+13;
            int k;
            // find - char
            for(k=0; range_str[k] != '-'; k++){}
            // start bits
            if(k >= 0){
                range_str[k] = '\0';
                r.start = atoi(range_str);
                range_str += k+1;
            }
            // end bits
            if(strlen(range_str) > 0){
                r.end = atoi(range_str);
            }

        }
        free(lines[i]);
    }
    path = build_string("%s/%s", serve, path);
    path = realpath(path, NULL);
    if(path == NULL){
        res = "HTTP/1.1 404 Not Found\n";
        goto write_response;
    }
    free(lines);
    // check path is valid
    printf("GET: %s\n", path);
    if(isfile(path)){
        FILE *file = fopen(path, "rb");
        if (file == NULL) {
            printf("Failed to open file: %s\n", path);
        } else {
            size_t fsize = filesize(path);
            serve_file(client_fd, file, fsize, r.start, r.end);
            fclose(file);
        }
        goto free_handle_client;
    } else if(isdir(path)){
        list_directory(client_fd, path, serve);
        goto free_handle_client;
    }
    res = "HTTP/1.1 200 OK\n" \
    "Content-Type: text/plain\n\n" \
    "Hello World";
    goto write_response;

    write_response:

    if(swrite(client_fd, res) < 0){
        goto free_handle_client;
    }


    free_handle_client:

    // free memory
    free(path);
    close(client_fd);
    return NULL;
}

static int httpd(char** args){
    if(strlen(variable_get_value(y->variables, "source")) == 0){
        variable_set_value(y->variables, "source", "/");
    }
    (void)args;
    int port = 8000;
    // set port if defined
    if(!iseq(variable_get_value(y->variables, "port"), "")){
        port = atoi(variable_get_value(y->variables, "port"));
    }
    struct sockaddr_in addr;
    int addrlen = sizeof(addr);
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0) {
        printf("Error opening socket\n");
        return 1;
    }
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = 0;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_family = AF_INET;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int)) < 0) {
        perror("setsockopt(SO_REUSEPORT) failed");
    }
    if(bind(fd, (struct sockaddr *)&addr,sizeof(struct sockaddr_in) ) < 0) {
        printf("Error binding socket\n");
        return 1;
    }
    if ((listen(fd, 3)) < 0) {
        printf("Listen failed...\n");
        return 1;
    }
    while (true){
        int client_fd = accept(fd, (struct sockaddr *)&addr, (socklen_t*)&addrlen);
        pthread_t th;
        if (pthread_create(&th, NULL, handle_client, &client_fd) != 0) {
            perror("Failed to create thread");
            continue;
        }
    }
    return 0;
}

visible void plugin_init(Ymp* ymp){
    y = ymp;
    Operation op;
    op.name = "httpd";
    op.description = "simple http server";
    op.call = (callback)httpd;
    op.alias = NULL;
    op.help = help_new();
    help_add_parameter(op.help, "--source", "serve directory. (default /)");
    help_add_parameter(op.help, "--port", "server tcp port (default 8000)");
    op.min_args = 0;
    operation_register(y->manager, op);
}
