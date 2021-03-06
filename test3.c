#include <stdio.h>
#include <time.h>
#include <stdlib.h> 
#include <unistd.h>
#include <pthread.h>
#include <termios.h>
#include <signal.h>

/* Needs to be fixed: It doesn't stop when CTRL+C => Needs to be explicitly implemented */
/* If anyone know how to link the library with the c program during execution?? */
/* For now, I added the functions here for reference and see if it works */

/** Start khbit.c **/
static struct termios initial_settings, new_settings;
static int peek_character = -1;

void init_keyboard()
{
    tcgetattr(0,&initial_settings);
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

    if (peek_character != -1) return 1;
    new_settings.c_cc[VMIN]=0;
    tcsetattr(0, TCSANOW, &new_settings);
    nread = read(0,&ch,1);
    new_settings.c_cc[VMIN]=1;
    tcsetattr(0, TCSANOW, &new_settings);
    if(nread == 1)
    {
        peek_character = ch;
        return 1;
    }
    return 0;
}

int readch()
{
char ch;

    if(peek_character != -1)
    {
        ch = peek_character;
        peek_character = -1;
        return ch;
    }
    read(0,&ch,1);
    return ch;
}

/* End khbit.c */
 

char key;
int r;
int q;
int p;
int s;
int rs;

struct TIME{
  int hour;
  int min;
  int sec;
};

/** Thread Function for to catch R or r **/
void* signals(void *arg){
  r = 0;
  q = 0;
  p = 0;
  rs = 0;

  if(kbhit()){
      key = readch();
      if(key == 'R' || key == 'r'){
        r = 1;
      }
      else if(key == 'Q' || key == 'q'){
        q = 1;
      }
      else if(key == 'p' || key == 'P'){
        p = 1;
        s = 0;
      }
       else if(key == 'S' || key == 's'){
        s = 1;
        p = 0;
      }
    }
  pthread_exit(NULL);
};

//Thread Function to catch q to exit the program


/** Function to calculate the time difference **/
void diffTime(struct TIME start,
              struct tm stop,
              struct TIME *diff) {
   while ( start.sec > stop.tm_sec) {
      --stop.tm_min;
      stop.tm_sec += 60;
   }
   
   diff->sec = stop.tm_sec - start.sec;
   while (start.min > stop.tm_min) {
      --stop.tm_hour;
      stop.tm_min += 60;
   }
   diff->min = stop.tm_min - start.min;
   diff->hour = stop.tm_hour - start.hour;
}

int main(void){
    struct TIME curr_time, diff, pause, pausediff, diff2;
    time_t start;
    time(&start);
    struct tm *start_time = localtime(&start);
    pthread_t tsignals;

    pausediff.hour = 0;
    pausediff.min = 0;
    pausediff.sec = 0;    
    
    curr_time.hour = start_time->tm_hour;
    curr_time.min = start_time->tm_min;
    curr_time.sec = start_time->tm_sec;

    // Menu 
    printf("Welcome to StopWatch\n");
    printf("Press -> S or s to start/restart the timer\n");
    printf("Press -> R or r to reset the timer\n");
    printf("Press -> P or p to pause the timer\n");
    printf("Press -> Q or q to quit the timer\n");

    while(1){
      if(kbhit()){
        key = readch();
      
        if(key == 'S' || key == 's'){
          s = 1;
          while(1){
              time_t end;
              struct tm *end_time;
              time(&end);
              end_time = localtime(&end);

              pthread_create(&tsignals, NULL, &signals, NULL);
              pthread_join(tsignals, NULL);

              // Check if r == 1 of reset
              if (s == 1 && p == 0){
                if(r == 1){
                curr_time.hour = start_time->tm_hour;
                curr_time.min = start_time->tm_min;
                curr_time.sec = start_time->tm_sec;
                }
                else if(q==1){ 
                  exit(1);          
                }
                
                system("clear");
                diffTime(curr_time, *end_time, &diff);
                printf("StopWatch: \t\t\t\t %02d:%02d:%02d\n",diff.hour, diff.min, diff.sec); 
                sleep(1);
              }
              
              else if (p == 1 && s == 0){ 
                pause.hour = diff.hour;
                pause.min = diff.min;
                pause.sec = diff.sec;

                system("clear");
                printf("StopWatch: \t\t\t\t %02d:%02d:%02d\n",pause.hour, pause.min, pause.sec);
              }                
          }
      }else if(key == 'Q' || key == 'q'){
        exit(0);
      }
    }
  }  
}
