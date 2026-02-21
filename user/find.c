#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"



void
find(char *path, char *target, char *exec_cmd)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if ((fd = open(path, O_RDONLY)) < 0) {
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }

  if (fstat(fd, &st) < 0) {
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }

  if (st.type == T_FILE) {
    char *fname = path + strlen(path);
    while (fname > path && *(fname - 1) != '/')
      fname--;
    close(fd);
    
    if (strcmp(fname, target) == 0) {
      if(exec_cmd){
        int pid = fork();
        if (pid < 0) {
          fprintf(2, "find: fork failed\n");
        } else if (pid == 0) {
          // Вроде так вызов идёт
          char *args[3];
          args[0] = exec_cmd;
          args[1] = path;
          args[2] = 0;
          exec(exec_cmd, args);
          fprintf(2, "find: exec %s failed\n", exec_cmd);
          exit(1);
        } else {
          int status;
          wait(&status);
          if(status){
            //не 0 это плохо
            fprintf(2, "exec %s command returned non-zero value %d \n", exec_cmd, status);
          }
        }
      }
      else{
        printf("%s\n", path);
      }
    }
    return;
  }

  if (st.type == T_DIR) {
    if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
      fprintf(2, "find: path too long\n");
      close(fd);
      return;
    }

    strcpy(buf, path);
    p = buf + strlen(buf);
    *p++ = '/';

    while (read(fd, &de, sizeof(de)) == sizeof(de)) {
      if (de.inum == 0)
        continue;

      if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
        continue;

      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = '\0';

      find(buf, target, exec_cmd);
    }
  }

  close(fd);
}

int
main(int argc, char *argv[])
{
  char *exec_cmd = 0;
  
  if (argc == 3) {
    exec_cmd = 0;
  } else if (argc == 5 && strcmp(argv[3], "-exec") == 0) {
    exec_cmd = argv[4];
  } else {
    fprintf(2, "usage: find <path> <name>\n");
    fprintf(2, "or: find <path> <name> -exec <cmd>\n");
    exit(1);
  }

  find(argv[1], argv[2], exec_cmd);
  exit(0);
}