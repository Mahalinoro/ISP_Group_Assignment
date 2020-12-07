/** The program lags when resetting or quitting, you have to press the key more than once
 *  Code needs to be divided into threads
 *  Pause and play yet to be implemented
 **/



#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
// for read()
static struct termios initial_settings, new_settings;
static int peek_character = -1;
void init_keyboard()
{
  tcgetattr(0, &initial_settings);
  new_settings = initial_settings;
  new_settings.c_lflag &= ~ICANON;
  new_settings.c_lflag &= ~ECHO;
  new_settings.c_lflag &= ~ISIG;
  new_settings.c_cc[VMIN] = 1;
  new_settings.c_cc[VTIME] = 0;
  tcsetattr(0, TCSANOW, &new_settings);
}
void close_keyboard()
{
  tcsetattr(0, TCSANOW, &initial_settings);
}
int kbhit()
{
  unsigned char ch;
  int nread;
  if (peek_character != -1)
      return 1;
  new_settings.c_cc[VMIN] = 0;
  tcsetattr(0, TCSANOW, &new_settings);
  nread = read(0, &ch, 1);
  new_settings.c_cc[VMIN] = 1;
  tcsetattr(0, TCSANOW, &new_settings);
  if (nread == 1)
  {
      peek_character = ch;
      return 1;
  }
  return 0;
}
int readch()
{
  char ch;
  if (peek_character != -1)
  {
      ch = peek_character;
      peek_character = -1;
      return ch;
  }
  read(0, &ch, 1);
  return ch;
}
/** End of kbhit **/
int p = 0;
int r = 0;
int s = 0;
void *listener(void *arg)
{
  if (kbhit())
  {
      char c = getchar();
      // Start program
      if (c == 's')
      {
          s = 1;
      }
      // Closes program
      if (c == 'q')
      {
          exit(1);
      }
      // Reset function
      if (c == 'r')
      {
          r = 1;
      }
      // Pause and Play function
      if (c == ' ')
      {
          p = 1;
      }
  }
}
int main(void)
{
  printf("Welcome to the Digital Clock \n");
  printf("Press s to start, q to quit,\n");
  printf("spacebar to pause/play and r to reset\n");
  pthread_t listenThread;
  int i, x, y;
 
  while(1){
   if(kbhit()){
     char c = readch();
     if (c == 's' || c == 'S'){
        //  pthread_create(&listenThread, NULL, &listener, NULL);
        //           pthread_join(listenThread, NULL);
        for (i = 0; i < 24; i++)
       {
           for (x = 0; x < 60; x++)
           {
               for (y = 0; y < 60; y++)
               {
                   system("clear");
                   printf("Press q to quit,");
  printf("spacebar to pause/play and r to reset\n");
                   printf("\rTime: %d: %d: %d\n", i, x, y); 
                   sleep(1);
                   if(kbhit()){
                       char ch = getchar();
                       if(ch == 'r'){
                           y = 0;
                           i = 0;
                           x = 0;
                       }
                       if (ch == 'q'){
                           exit(1);
                       }
                                          }
               }
           }
       }
     }
   }
  }
 
   return 0;
}
 

