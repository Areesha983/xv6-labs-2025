#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/param.h"

int do_exec = 0;
char *execargv[MAXARG];
int execargc = 0;

/* simple regex from grep.c */
int matchhere(char*, char*);
int matchstar(int, char*, char*);

int
match(char *re, char *text)
{
  if(re[0] == '^')
    return matchhere(re+1, text);
  do{  // must look at empty string
    if(matchhere(re, text))
      return 1;
  }while(*text++ != '\0');
  return 0;
}

int
matchhere(char *re, char *text){
  if(re[0] == '\0')
    return 1;
  if(re[1] == '*')
    return matchstar(re[0], re+2, text);
  if(re[0] == '$' && re[1] == '\0')
    return *text == '\0';
  if(*text!='\0' && (re[0]=='.' || re[0]==*text))
    return matchhere(re+1, text+1);
  return 0;
}

int
matchstar(int c, char *re, char *text)
{
  do{  // a * matches zero or more instances
    if(matchhere(re, text))
      return 1;
  }while(*text!='\0' && (*text++==c || c=='.'));
  return 0;
}

/* find implementation */
void
find(char *path, char *pattern)
{
  int fd;
  struct stat st;
  struct dirent de;
  char buf[512], *p;

  if((fd = open(path, O_RDONLY)) < 0)
    return;
  if(fstat(fd, &st) < 0){
    close(fd);
    return;
  }

  if(st.type == T_FILE){
    // check current file
    p = path + strlen(path);
    while(p > path && *p != '/') p--;
    if(*p == '/') p++;
    if(match(pattern, p)){
      if(!do_exec){
        printf("%s\n", path);
      } else {
        if(fork() == 0){
          char *argv2[MAXARG];
          for(int i = 0; i < execargc; i++)
            argv2[i] = execargv[i];
          argv2[execargc] = path;
          argv2[execargc+1] = 0;
          exec(execargv[0], argv2);
          fprintf(2, "exec %s failed\n", execargv[0]);
          exit(1);
        } else {
          wait(0);
        }
      }
    }
  }

  if(st.type == T_DIR){
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      close(fd);
      return;
    }
    strcpy(buf, path);
    p = buf + strlen(buf);
    *p++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0)
        continue;
      if(strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
        continue;
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      find(buf, pattern);
    }
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  if(argc < 3){
    fprintf(2, "usage: find path pattern [-exec cmd ...]\n");
    exit(1);
  }
  if(argc > 3 && strcmp(argv[3], "-exec") == 0){
    do_exec = 1;
    execargc = argc - 4;
    for(int i = 0; i < execargc; i++)
      execargv[i] = argv[i+4];
    execargv[execargc] = 0;
  }

  find(argv[1], argv[2]);
  exit(0);
}

