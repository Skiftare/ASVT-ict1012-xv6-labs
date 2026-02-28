#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"



void
find(char *path, char *target, char *exec_cmd, char **exec_args, int exec_argc)
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
      if(exec_cmd && exec_args){
        int pid = fork();
        if (pid < 0) {
          fprintf(2, "find: fork failed\n");
        } else if (pid == 0) {
          // Собираем args: копируем exec_args + добавляем path + NULL
          char *args[32];  // достаточно для xv6
          int i;
          for(i = 0; i < exec_argc; i++)
            args[i] = exec_args[i];  // копируем ["/grep", "hello"]
          args[i++] = path;          // добавляем путь к файлу
          args[i] = 0;               // NULL-терминатор
          
          exec(args[0], args);       // exec("/grep", ["/grep", "hello", "./a/b", 0])
          fprintf(2, "find: exec %s failed\n", args[0]);
          exit(1);
        } else {
          int status;
          wait(&status);
        }
      } else {
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

      find(buf, target, exec_cmd, exec_args, exec_argc);
    }
  }

  close(fd);
}

// В main():
int
main(int argc, char *argv[])
{
  char *exec_cmd = 0;
  char **exec_args = 0;  // массив аргументов для exec
  
  if (argc == 3) {
    // просто find path name
  } else if (argc >= 5 && strcmp(argv[3], "-exec") == 0) {
    exec_cmd = argv[4];  // сама команда, например "/grep"
    exec_args = &argv[4];  // указатель на начало: ["/grep", "hello", 0]
    // Но нам нужно добавить путь к файлу в конец и поставить NULL
  } else {
    fprintf(2, "usage: find <path> <name>\n");
    fprintf(2, "or: find <path> <name> -exec <cmd> [args]\n");
    exit(1);
  }

  find(argv[1], argv[2], exec_cmd, exec_args, argc - 4); // передаём кол-во exec-аргументов
  exit(0);
}