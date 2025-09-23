#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/param.h"

int do_exec = 0;
char *execargv[MAXARG];
int execargc = 0;

void
find(char *path, char *target) {
  int fd;
  struct stat st;
  struct dirent de;
  char buf[512], *p;

  if((fd = open(path, O_RDONLY)) < 0) return;
  if(fstat(fd, &st) < 0) { close(fd); return; }

  // extract filename
  p = path + strlen(path);
  while(p > path && *p != '/') p--;
  if(*p == '/') p++;
  if(strcmp(p, target) == 0) {
    if(!do_exec) {
      printf("%s\n", path);
    } else {
      // fork + exec
      if(fork() == 0) {
        char *argv2[MAXARG];
        for(int i = 0; i < execargc; i++)
          argv2[i] = execargv[i];
        argv2[execargc] = path; // add the found file
        argv2[execargc+1] = 0;
        exec(execargv[0], argv2);
        fprintf(2, "exec %s failed\n", execargv[0]);
        exit(1);
      } else {
        wait(0);
      }
    }
  }

  if(st.type == T_DIR) {
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) { close(fd); return; }
    strcpy(buf, path);
    char *q = buf + strlen(path);
    *q++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)) {
      if(de.inum == 0) continue;
      if(strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0) continue;
      memmove(q, de.name, DIRSIZ);
      q[DIRSIZ] = 0;
      find(buf, target);
    }
  }
  close(fd);
}

int
main(int argc, char *argv[]) {
  if(argc < 3) {
    fprintf(2, "usage: find path name [-exec cmd ...]\n");
    exit(1);
  }
  if(argc > 3 && strcmp(argv[3], "-exec") == 0) {
    do_exec = 1;
    execargc = argc - 4;
    for(int i = 0; i < execargc; i++)
      execargv[i] = argv[i+4];
    execargv[execargc] = 0;
  }
  find(argv[1], argv[2]);
  exit(0);
}

