#include <ls.h>


#define COLOR_RESET   "\033[0m"
#define COLOR_DIR     "\033[1;34m"
#define COLOR_EXEC    "\033[1;32m"
#define COLOR_LINK    "\033[1;36m"

void ls_(char* pre_result, int* status, const char** args, size_t num_args) {
    int show_all = 0;
    int long_format = 0;
    const char* target_dir = ".";

    *status = 1;
    pre_result[0] = '\0';

    for (size_t i = 0; i < num_args; i++) {
        if (strcmp(args[i], "-a") == 0) {
            show_all = 1;
        } else if (strcmp(args[i], "-l") == 0) {
            long_format = 1;
        } else if (args[i][0] == '-') {
            sprintf(pre_result, "ls: invalid argument %s\n", args[i]);
            *status = 0;
            return;
        } else {
            target_dir = args[i];
        }
    }

    DIR* dir = opendir(target_dir);
    if (!dir) {
        snprintf(pre_result, 1024, "ls: cannot access '%s': %s\n", target_dir, strerror(errno));
        *status = 0;
        return;
    }

    struct dirent* entry;
    char path[1024];
    char temp[2048];
    struct stat st;

    while ((entry = readdir(dir)) != NULL) {
        if (!show_all && entry->d_name[0] == '.') continue;

        snprintf(path, sizeof(path), "%s/%s", target_dir, entry->d_name);
        if (lstat(path, &st) == -1) continue;

        const char* color_start = "";
        const char* color_end = COLOR_RESET;

        if (S_ISDIR(st.st_mode)) {
            color_start = COLOR_DIR;
        } else if (S_ISLNK(st.st_mode)) {
            color_start = COLOR_LINK;
        } else if (st.st_mode & S_IXUSR) {
            color_start = COLOR_EXEC;
        }

        if (long_format) {
            // 权限字符串
            char perms[11] = "----------";
            if (S_ISDIR(st.st_mode)) perms[0] = 'd';
            if (S_ISLNK(st.st_mode)) perms[0] = 'l';
            if (st.st_mode & S_IRUSR) perms[1] = 'r';
            if (st.st_mode & S_IWUSR) perms[2] = 'w';
            if (st.st_mode & S_IXUSR) perms[3] = 'x';
            if (st.st_mode & S_IRGRP) perms[4] = 'r';
            if (st.st_mode & S_IWGRP) perms[5] = 'w';
            if (st.st_mode & S_IXGRP) perms[6] = 'x';
            if (st.st_mode & S_IROTH) perms[7] = 'r';
            if (st.st_mode & S_IWOTH) perms[8] = 'w';
            if (st.st_mode & S_IXOTH) perms[9] = 'x';

            struct passwd* pw = getpwuid(st.st_uid);
            struct group* gr = getgrgid(st.st_gid);

            char timebuf[64];
            strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", localtime(&st.st_mtime));

            snprintf(temp, sizeof(temp),
                     "%s %2ld %s %s %6ld %s %s%s%s\n",
                     perms, st.st_nlink,
                     pw ? pw->pw_name : "unknown",
                     gr ? gr->gr_name : "unknown",
                     (long)st.st_size, timebuf,
                     color_start, entry->d_name, color_end);
        } else {
            snprintf(temp, sizeof(temp), "%s%s%s  ", color_start, entry->d_name, color_end);
        }

        strncat(pre_result, temp, 8192 - strlen(pre_result) - 1);
    }

    if (!long_format)
        strcat(pre_result, "\n");

    closedir(dir);
}