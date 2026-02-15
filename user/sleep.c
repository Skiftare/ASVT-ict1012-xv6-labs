#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int i;

  if(argc < 2){
    fprintf(2, "Usage: sleep ticks\n");
    exit(1);
  }
  i = 0;
  int flag = 1;
  while(flag == 1){
    if(argv[1][i] == '\0'){
      break;
    }
    if(argv[1][i] - '0' < 0 || argv[1][i] - '9' > 0){
      flag = 0;
    }
    i++;
  }
  if(flag == 0){
    fprintf(2, "Usage: sleep ticks, ticks is a NATURAL NUMBER of ticks to sleep\n");
    exit(1);
  }
  i--;
  flag = i;
  i = 0;
  i = -1;
  int second_flag = 0;

  while(flag--){
    if( i > 10*i+argv[1][flag]){
      second_flag = 1;
      break;
    }
    i = 10*i+argv[1][flag];
  }
  if(second_flag == 1){
    fprintf(2, "Amount of ticks should be in INT range of values\n");
    exit(1);
  }

  i = atoi(argv[1]);
  pause(i);
  


  exit(0);
}
