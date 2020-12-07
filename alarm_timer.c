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
int a;
int alarm_hour, alarm_minute, alarm_second;

struct TIME{
  int hour;
  int min;
  int sec;
};

/** Thread Function for to catch R or r **/
void* reset(void *arg){
  r = 0;

  if(kbhit()){
      key = readch();
      if(key == 'R' || key == 'r'){
        r = 1;
      }
    }
  pthread_exit(NULL);
};

//Thread Function to catch q to exit the program

void* stop(void *arg){
  q = 0;

  if(kbhit()){
      key = readch();
      if(key == 'Q' || key == 'q'){
        q = 1;
      }
    }
  pthread_exit(NULL);
};

void* set_alarm(void *arg){
  a = 0;

  if(kbhit()){
      key = readch();
      if(key == 'A' || key == 'a'){
        a = 1;
      }
    }
  pthread_exit(NULL);
};

void alarm_handler(int signal_number){
  printf("Alarm is Ringing\n");
  alarm(0);
}

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
    struct TIME curr_time, diff;
    time_t start;
    time(&start);
    struct tm *start_time = localtime(&start);
    pthread_t treset;
    pthread_t talarm;
    pthread_t tstop;

    curr_time.hour = start_time->tm_hour;
    curr_time.min = start_time->tm_min;
    curr_time.sec = start_time->tm_sec;


    while(1){
        time_t end;
        struct tm *end_time;
        time(&end);
        end_time = localtime(&end);

        pthread_create(&treset, NULL, &reset, NULL);
	      pthread_create(&tstop,NULL, &stop, NULL);
        pthread_join(treset, NULL);
        


        // Check if r == 1 of reset
        if(r == 1){
          curr_time.hour = start_time->tm_hour;
          curr_time.min = start_time->tm_min;
          curr_time.sec = start_time->tm_sec;
        }

      

        system("clear");
        printf("S --> Start / P --> Pause / R --> Reset / E --> Exit / A --> Alarm\n");
        diffTime(curr_time, *end_time, &diff);
        printf("StopWatch: %02d:%02d:%02d\n", diff.hour, diff.min, diff.sec);        
        sleep(1);
      
        // Alarm Thread
        pthread_create(&talarm, NULL, &set_alarm, NULL);

      if (a==1){
        pthread_join(talarm, NULL);
        printf("Set Alarm Timer!");
        printf("Set Hour Timer!");
        scanf("%d", &alarm_hour);
        printf("Set Minute Timer!");
        scanf("%d", &alarm_minute);
        printf("Set Second Timer!");
        scanf("%d", &alarm_second);
        printf("Alarm will ring in: %02d: %02d: %02d \n\n", alarm_hour, alarm_minute, alarm_second);
        sleep(2);

        // Converting set alarm to seconds
        int total_alarm = (3600*alarm_hour) + (60*alarm_minute) + (alarm_second);
        alarm(total_alarm);
        // if (curr_time.hour==alarm_hour&&curr_time.min==alarm_minute&&curr_time.sec==alarm_second){
        //   alarm(1);
        // }
      }

      if(q==1){

        kill(getpid(), SIGKILL);
      }
    };
    return 0;
}
