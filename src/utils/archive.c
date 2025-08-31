#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <archive.h>
#include <archive_entry.h>
#include <fcntl.h>

#include <libgen.h>

#include <utils/error.h>
#include <utils/string.h>
#include <utils/file.h>
#include <utils/archive.h>
#include <core/ymp.h>

#include <core/logger.h>

visible Archive* archive_new(){
    Archive *data = calloc(1, sizeof(Archive));
    data->add_list_size = 0;
    data->errors = array_new();
    data->a = array_new();
    return data;
}

visible void archive_unref(Archive* data){
    free(data->errors);
    free(data->a);
    free(data);
}

visible void archive_load(Archive *data, const char* path) {
    debug("archive load:  %s\n", path);
    data->archive_path = strdup(path);
    archive_set_type(data, "zip", "none");
}

static void archive_load_archive(Archive *data) {
    data->archive = archive_read_new();
    archive_read_support_filter_all(data->archive);
    archive_read_support_format_all(data->archive);
    if (archive_read_open_filename(data->archive, data->archive_path, 10240) != ARCHIVE_OK) {
        char* error_msg = build_string("Failed to open archive: %s", archive_error_string(data->archive));
        error_add(error_msg);
    }
}

visible void archive_set_target(Archive *data, const char* target){
    debug("set archive target:  %s\n", target);
    data->target_path = strdup(target);
}

visible bool archive_is_archive(Archive *data, const char *path) {
    debug("check is archive:  %s\n", path);
    data->archive = archive_read_new();
    archive_read_support_filter_all(data->archive);
    archive_read_support_format_all(data->archive);
    int result = archive_read_open_filename(data->archive, path, 10240);
    archive_read_close(data->archive);
    archive_read_free(data->archive);
    return result == ARCHIVE_OK;
}

visible char** archive_list_files(Archive *data, size_t* len) {
    debug("list archive files\n");
    archive_load_archive(data);
    struct archive_entry *entry;
    while (archive_read_next_header(data->archive, &entry) == ARCHIVE_OK) {
        array_add(data->a,archive_entry_pathname(entry));
        archive_read_data_skip(data->archive);
    }
    archive_read_close(data->archive);
    archive_read_free(data->archive);

    return array_get(data->a, len);
}

visible void archive_add(Archive *data, const char *path) {
    debug("archive add file: %s\n", path);
    array_add(data->a, path);
}

visible void archive_create(Archive *data){
    size_t len;
    archive_write(data, data->archive_path, array_get(data->a, &len));
}

static void archive_extract_fn(Archive *data, const char *path, bool all) {
    archive_load_archive(data);
    struct archive_entry *entry;
    while (archive_read_next_header(data->archive, &entry) == ARCHIVE_OK) {
        const char *entry_path = archive_entry_pathname(entry);
        char *target_file = NULL;
        if(data->target_path == NULL){
            error(3);
        }
        if(strlen(entry_path) == 0){
            continue;
        } else if (strcmp(entry_path, path) != 0 && !all) {
            continue;
        }
        target_file = build_string("%s/%s", data->target_path, entry_path);
        info("Extract: %s\n", entry_path);
        /* Check if entry is a directory */
        mode_t mode = archive_entry_filetype(entry);
        if (S_ISDIR(mode)) {
            /* Create the directory if it doesn't exist */
            if (access(target_file, F_OK) != -1) {
                continue;
            }
            create_dir(target_file);
            continue;
        }
        char* dir = strdup(target_file);
        dirname(dir);
        if (!isdir(dir)) {
            create_dir(dir);
        }
        if(issymlink(target_file) || isfile(target_file)){
            unlink(target_file);
        }
        if (S_ISLNK(mode)) {
            if(isdir(target_file)){
                continue;
            }
            const char *link_target = archive_entry_symlink(entry);
            if (link_target != NULL) {
                if (symlink(link_target, target_file) != 0) {
                    char* error_msg = build_string("Failed to create symbolic link: %s -> %s", target_file, link_target);
                    error_add(error_msg);
                    error(3);
                }
                continue;
            }
        }else if (S_ISREG(mode)){
            FILE *file = fopen(target_file, "wb");
            if (file == NULL) {
                char* error_msg = build_string("Failed to open file for writing: %s", target_file);
                error_add(error_msg);
                error(3);
            }
            char buffer[4096];
            ssize_t size;
            while ((size = archive_read_data(data->archive, buffer, sizeof(buffer))) > 0) {
                fwrite(buffer, 1, size, file);
            }
            fclose(file);
            chmod(target_file, 0755);
        } else {
            printf("Skip unsupported archive entry: %s", entry_path);
        }
    }
    archive_read_close(data->archive);
    archive_read_free(data->archive);
}


visible void archive_extract_all(Archive *data) {
    archive_extract_fn(data, "",true);
}
visible void archive_extract(Archive *data, const char* path) {
    archive_extract_fn(data, path, false);
}

visible char* archive_readfile(Archive *data, const char *file_path) {
    debug("archive read file: %s\n", file_path);
    archive_load_archive(data);
    struct archive_entry *entry;
    char *ret = NULL;
    while (archive_read_next_header(data->archive, &entry) == ARCHIVE_OK) {
        const char *entry_path = archive_entry_pathname(entry);
        if (strcmp(entry_path, file_path) != 0)
           continue;
        size_t size = archive_entry_size(entry);
        ret = (char *)malloc(size + 1);
        if (ret == NULL) {
           char* error_msg = build_string("Memory allocation failed");
           error_add(error_msg);
        }
        ssize_t bytes_read = archive_read_data(data->archive, ret, size);
        if (bytes_read < 0) {
           char* error_msg = build_string("Failed to read file: %s", archive_error_string(data->archive));
           free(ret);
           error_add(error_msg);
        }
        ret[bytes_read] = '\0';
        break;
    }
    archive_read_close(data->archive);
    archive_read_free(data->archive);
    return ret;
}

#include <sys/stat.h>


visible void archive_set_type(Archive *data, const char* form, const char* filt){
    debug("archive type changed: %s %s\n", form, filt);
    if(strcmp(form,"zip")==0)
        data->aformat=zip;
    else if(strcmp(form,"tar")==0)
        data->aformat=tar;
    else if(strcmp(form,"p7zip")==0)
        data->aformat=p7zip;
    else if(strcmp(form,"cpio")==0)
        data->aformat=cpio;
    else if(strcmp(form,"ar")==0)
        data->aformat=ar;

    if(strcmp(filt,"none")==0)
        data->afilter=filter_none;
    else if(strcmp(filt,"gzip")==0)
        data->afilter=filter_gzip;
    else if(strcmp(filt,"xz")==0)
        data->afilter=filter_xz;
}

visible void archive_write(Archive *data, const char *outname, char **filename) {
  struct archive *a;
  struct archive_entry *entry;
  struct stat st;
  char buff[8192];
  int len;
  int fd;
  int e;

  a = archive_write_new();
  /* compress format */
  if(data->afilter == filter_gzip){
      archive_write_add_filter_gzip(a);
  }else if(data->afilter == filter_xz){
      archive_write_add_filter_xz(a);
  }else{
      archive_write_add_filter_none(a);
  }
  /* archive format */
  if(data->aformat == tar){
      e = (archive_write_set_format_gnutar(a) != ARCHIVE_OK);
  }else if (data->aformat == p7zip){
      e = (archive_write_set_format_7zip(a) != ARCHIVE_OK);
  }else if (data->aformat == cpio){
      e = (archive_write_set_format_cpio(a) != ARCHIVE_OK);
  }else if (data->aformat == ar){
      e = (archive_write_set_format_ar_bsd(a) != ARCHIVE_OK);
  }else{
      e = (archive_write_set_format_zip(a) != ARCHIVE_OK);
  }
  if (e){
      error_add("Libarchive error!");
      return;
  }

  archive_write_open_filename(a, outname);
  entry = NULL;
  while (*filename) {
    debug("archive write : %s\n", filename[0]);
    if(!isexists(*filename)){
        filename++;
        continue;
    }
    lstat(*filename, &st);
    entry = archive_entry_new();
    archive_entry_set_pathname(entry, *filename);
    archive_entry_set_size(entry, st.st_size);
    if (S_ISBLK(st.st_mode)) {
        /* block device */
        archive_entry_set_filetype(entry, AE_IFBLK);
    } else if (S_ISCHR(st.st_mode)) {
        /* character device */
        archive_entry_set_filetype(entry, AE_IFCHR);
    } else if (S_ISDIR(st.st_mode)) {
        /* directory */
        archive_entry_set_filetype(entry, AE_IFDIR);
    } else if (S_ISDIR(st.st_mode)) {
        /* FIFO/pipe */
        archive_entry_set_filetype(entry, AE_IFIFO);
    } else if (S_ISLNK(st.st_mode)) {
        char link[PATH_MAX];
        len = readlink(*filename,link,sizeof(link));
        if(len < 0){
            error_add("Failed to create archive");
            break;
        }
        link[len] = '\0';
        archive_entry_set_filetype(entry, AE_IFLNK);
        archive_entry_set_symlink(entry, link);
    } else if (S_ISREG(st.st_mode)) {
        /* regular file */
        archive_entry_set_filetype(entry, AE_IFREG);
    } else if (S_ISSOCK(st.st_mode)) {
        /* socket */
        archive_entry_set_filetype(entry, AE_IFSOCK);
     } else {
        /* unknown */
        archive_entry_set_filetype(entry, AE_IFREG);
        error_add("Failed to create archive");
    }
    if(has_error()){
        error(2);
    }
    archive_entry_set_perm(entry, 0644);
    archive_write_header(a, entry);
    fd = open(*filename, O_RDONLY);
    len = read(fd, buff, sizeof(buff));
    while ( len > 0 ) {
        archive_write_data(a, buff, len);
        len = read(fd, buff, sizeof(buff));
    }
    close(fd);
    filename++;
  }
  archive_entry_free(entry);
  archive_write_close(a);
  archive_write_free(a);
}

