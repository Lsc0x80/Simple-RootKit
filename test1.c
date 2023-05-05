#include <stdio.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

char* getcurproc() {
    char* proc_path = malloc(32);
    sprintf(proc_path, "/proc/%d/", getpid());
    printf("proc_path = %s", proc_path);
    return proc_path;
}
int main() 
{
    char cur_pid[32];
    sprintf(cur_pid, "%d", getpid());
    printf(cur_pid);
    printf("%s", getcurproc());
}

