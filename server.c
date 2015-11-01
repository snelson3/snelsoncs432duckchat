#include <stdio.h>

const int DEBUG = 5;

void debug (char *msg, int priority)
{
  int i;
  if (priority <= DEBUG) {
    for (i = 0; i < priority; i++) fprintf(stderr," ");
    fprintf(stderr, "%s\n",msg);
  }
}

void debugn(char *msg, int n, int priority)
{
  int i;
  if (priority <= DEBUG){
    for (i = 0; i < priority; i++) fprintf(stderr, " ");
    fprintf(stderr, "%s%d\n",msg,n);
  }
}

int main(int argc, char *argv[]) {
   fprintf(stderr,"hello world");
}
