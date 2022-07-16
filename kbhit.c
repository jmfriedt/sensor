/*#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
 
int kbhit(void)
{
  struct termios oldt, newt;
  int ch;
  int oldf;
 
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
 
  ch = getchar();
 
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);
 
  if(ch != EOF)
  {
    ungetc(ch, stdin);
    return 1;
  }
 
  return 0;
}
 
int main(void)
{
  while(!kbhit())
    puts("Press a key!");
  printf("You pressed '%c'!\n", getchar());
  return 0;
}


//////////////
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>

volatile int alarm_happened, ctrl_c_happened, io_happened;

void handle_alarm(int xxx)
{ alarm_happened=1; }

void handle_ctrl_c(int xxx)
{ ctrl_c_happened=1; }

void handle_io(int xxx)
{ io_happened=1; }

void main(void)
{ alarm_happened=0;
  ctrl_c_happened=0;
  io_happened=0;
  signal(SIGALRM, handle_alarm);
  signal(SIGINT, handle_ctrl_c);
  struct termios buf;
  tcgetattr(0, &buf);
  buf.c_lflag &= ~(ECHO | ICANON);
  buf.c_cc[VMIN]=1;
  buf.c_cc[VTIME]=0;
  tcsetattr(0, TCSAFLUSH, &buf);
  signal(SIGIO, handle_io);
  int savedflags=fcntl(0, F_GETFL, 0);
  fcntl(0, F_SETFL, savedflags | O_ASYNC | O_NONBLOCK );
  fcntl(0, F_SETOWN, getpid());
  alarm(8);
  while (1)
  {
    printf(".");
    fflush(stdout);
    sleep(5);

    if (alarm_happened)
    { printf("ALARM!");
      fflush(stdout);
      alarm_happened=0; }

    if (io_happened)
    { int c=getchar();
      printf(" IO(%c) ", c);
      fflush(stdout);
      io_happened=0; }

    if (ctrl_c_happened)
    { printf("INTERRUPT!");
      fflush(stdout);
      ctrl_c_happened=0; } }

  printf("\nAll Done\n"); }
