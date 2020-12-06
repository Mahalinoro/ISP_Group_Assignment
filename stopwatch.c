#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <termios.h>
#include <stdbool.h>
#include <signal.h>

/* Needs to be fixed: It doesn't stop when CTRL+C => Needs to be explicitly implemented */
/* If anyone know how to link the library with the c program during execution?? */
/* For now, I added the functions here for reference and see if it works */

/** Start khbit.c **/
/** External library that allows the program to detect a keyboard hit**/
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

/* End khbit.c */

char key;
int r;
int p;
int q;

struct TIME
{
  int hour;
  int min;
  int sec;
};
/** Thread Function to catch spacebar for pause/play function **/
void *pausePlay(void *arg)
{
  bool pause = *(bool*)arg;
  p = 0;
  if (kbhit())
  {
    key = readch();
    if (key == ' ')
    {
      p = 1;
     
    }
  }
  pthread_exit(NULL);
}
/** Thread Function to catch q for quit **/
void *quit(void *arg)
{
  q = 0;
  if (kbhit())
  {
    key = readch();
    if (key == 'q' || key == 'Q')
    {
      q = 1;
    }
  }
  pthread_exit(NULL);
}
/** Thread Function for to catch R or r **/
void *reset(void *arg)
{
  r = 0;

  if (kbhit())
  {
    key = readch();
    if (key == 'R' || key == 'r')
    {
      r = 1;
    }
  }
  pthread_exit(NULL);
};

/** Function to calculate the time difference **/
void diffTime(struct TIME start,
              struct tm stop,
              struct TIME *diff)
{
  while (start.sec > stop.tm_sec)
  {
    --stop.tm_min;
    stop.tm_sec += 60;
  }

  diff->sec = stop.tm_sec - start.sec;
  while (start.min > stop.tm_min)
  {
    --stop.tm_hour;
    stop.tm_min += 60;
  }
  diff->min = stop.tm_min - start.min;
  diff->hour = stop.tm_hour - start.hour;
}

int main(void)
{
  struct TIME curr_time, diff;
  time_t start;
  time(&start);
  struct tm *start_time = localtime(&start);
  pthread_t treset;
  pthread_t t_pause_play;
  pthread_t t_quit;
  bool pause = false;

  curr_time.hour = start_time->tm_hour;
  curr_time.min = start_time->tm_min;
  curr_time.sec = start_time->tm_sec;
  printf("Welcome to the Digital Clock. Press spacebar for pause/play,\n");
  printf("r to reset and q to quit/close the program\n ");
  printf("**********************************************\n");
  while (1)
  {
    time_t end;
    struct tm *end_time;
    time(&end);
    end_time = localtime(&end);
    struct tm pausetime;
    pthread_create(&treset, NULL, &reset, NULL);
    pthread_create(&t_quit, NULL, &quit, NULL);
    pthread_create(&t_pause_play, NULL, &pausePlay, &pause);
    pthread_join(treset, NULL);
    pthread_join(t_pause_play, NULL);
    pthread_join(t_quit, NULL);

    // Check if r == 1 of reset
    if (r == 1)
    {
      curr_time.hour = start_time->tm_hour;
      curr_time.min = start_time->tm_min;
      curr_time.sec = start_time->tm_sec;
    }
    // Check if p == 1 for pause/play
    if (p == 1 && pause == true)
    {
      pausetime.tm_hour = curr_time.hour;
      pausetime.tm_min = curr_time.min;
      pausetime.tm_sec = curr_time.sec;
    }

    if (p == 1 && pause == false)
    {
    }
    
    if(q == 1){
      exit(1);
    }
    system("clear");
    diffTime(curr_time, *end_time, &diff);
    printf("StopWatch: \t\t\t\t %02d:%02d:%02d\n", diff.hour, diff.min, diff.sec);
    sleep(1);
  };
  return 0;
}
