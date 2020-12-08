#include <stdio.h>
#include <time.h>
#include <stdlib.h> 
#include <unistd.h>
#include <pthread.h>
#include <termios.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>



/** Implementation of khbit.c **/ 
/** Source: "Beginning Linux Programming", from Wrox Press **/
/** Start kbhit.c **/
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

/** Alternative to getchar **/
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

/** Start khbit.c **/
 

/** GLOBAL VARIABLES **/
char key;
int r, q, p, s, rs;
struct TIME{
  int hour;
  int min;
  int sec;
};

/** Thread Function for to catch keyboard hits **/
void* signals(void *arg){
  r = 0;
  q = 0;
  p = 0;

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
    struct tm *start_time;
    time_t start;
    pthread_t tsignals;
    pid_t p;

    int total_alarm;
    int alarm_hour, alarm_second, alarm_minute;   

    // Menu 
    printf("Welcome to StopWatch! \n");
    printf("Press -> S or s to start/restart the timer\n");
    printf("Press -> R or r to reset the timer\n");
    printf("Press -> P or p to pause the timer\n");
    printf("Press -> A or a to set an alarm\n");
    printf("Press -> Q or q to quit the timer\n");


    p = fork();    
    if (p == 0){ // Child Process

      while(1){
        if(kbhit()){
          key = readch();     

          // Setting Alarm
          if(key == 'A' || key == 'a'){
            printf("\rSet Alarm Timer\n");
            printf("\rSet Hour: ");
            scanf("\r%d", &alarm_hour);
            printf("\rSet Minute: ");
            scanf("\r%d", &alarm_minute);
            printf("\rSet Second: ");
            scanf("\r%d", &alarm_second);
            printf("\rAlarm Rings in: %02d: %02d: %02d \n", alarm_hour, alarm_minute, alarm_second);
            // sleep(1);

            // Converting user input time to seconds
            total_alarm = ((alarm_hour*3600) + (alarm_minute*60) + (alarm_second));
            printf("Press S! \n");
          }   

          if(key == 'S' || key == 's'){
            s = 1;

            // Setting alarm to ring after User-set Time runs out
            // This function calls the SIGALRM signal
            alarm(total_alarm);   // Starts from when user presses S to start
            time(&start); // get the current time
            start_time = localtime(&start); 
            curr_time.hour = start_time->tm_hour;
            curr_time.min = start_time->tm_min;
            curr_time.sec = start_time->tm_sec;

            while(1){
                time_t end;
                struct tm *end_time;
                time(&end);
                end_time = localtime(&end);

                pthread_create(&tsignals, NULL, &signals, NULL); // Create thread signal
                pthread_join(tsignals, NULL);

                // Check if r == 1 of reset
                if (s == 1 && p == 0){   
                  if(r == 1){ // reset the time to start time
                    curr_time.hour = start_time->tm_hour;
                    curr_time.min = start_time->tm_min;
                    curr_time.sec = start_time->tm_sec;
                  }               

                  system("clear");
                  diffTime(curr_time, *end_time, &diff);
                  printf("Timer: \t\t\t\t %02d:%02d:%02d\n",diff.hour, diff.min, diff.sec); 
                  sleep(1);
                }
                // Pause
                else if (p == 1 && s == 0){ 
                  kill(p, SIGSTOP);
                }   

                // Killing the process
                if(q == 1){
                  kill(p, SIGKILL);
                }
            }
        }
        // Killing the process
        else if(key == 'Q' || key == 'q'){
          kill(p, SIGKILL);
        }
      }
    } 

  } else if(p > 0){ // Parent Process
    wait(NULL);    
    exit(EXIT_SUCCESS);
  }     
}
