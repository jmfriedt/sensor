#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>                  /* declaration of bzero() */
#include <fcntl.h>
#include <termios.h>
int init_rs232();
void free_rs232();
void sendcmd(int,char*);
struct termios oldtio,newtio;

#define BAUDRATE B57600
// #define BAUDRATE B19200
#define HC11DEVICE "/dev/ttyUSB0"
