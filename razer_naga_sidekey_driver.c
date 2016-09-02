#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include "/usr/include/linux/input.h"

#define INPUT_PATH "/dev/input/event9" //event9 is naga sidekey input file. The event# may be different elsewhere

#define KEYBIND_1       "F1"
#define KEYBIND_2       "F2"
#define KEYBIND_3       "F3"
#define KEYBIND_4       "F4"
#define KEYBIND_5       "F5"
#define KEYBIND_6       "F6"
#define KEYBIND_7       "F7"
#define KEYBIND_8       "F8"
#define KEYBIND_9       "F9"
#define KEYBIND_10      "F10"
#define KEYBIND_11      "F11"
#define KEYBIND_12      "F12"

static void signal_handler(const int sig);

static int fd_to_be_closed;

/*Precondition: must have "xdotool" installed via "sudo apt-get install xdotool"
 *This program is intended to run on startup via the command 'gnome-terminal -e "sudo <location of this file>"'
 *To give this executable sudo perms, run "sudo visudo" in shell, 
 *  then at end of file type "<your username> ALL=NOPASSWD: <location of this file>" 
 */
int main(void){
    const char *keybind[12] ={KEYBIND_1,KEYBIND_2,KEYBIND_3,KEYBIND_4,KEYBIND_5,KEYBIND_6,
                               KEYBIND_7,KEYBIND_8,KEYBIND_9,KEYBIND_10,KEYBIND_11,KEYBIND_12};
    int fd;
    if((fd = open(INPUT_PATH,O_RDONLY)) == -1){
        printf("File Doesn't Exist\n");
        exit(0);
    }
    fd_to_be_closed = fd;
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    ioctl(fd,EVIOCGRAB,1);
    daemon(0,1);

    //set curr to 4 because, for whatever reason, 4 is the first # given from keycode (doesn't have any relevance here)
    int prev, curr = 4, last_successful_code = -1;
    //see "/usr/include/linux/input.h" for structure interface
    struct input_event sieobj;

    //ignore the logic of testing each iteration. A lot of behind the scenes tweaking went in to make this work
    while(1){
        if(read(fd, &sieobj, sizeof(struct input_event)) != EOF){
            prev = curr;
            curr = sieobj.code;

            //if this conditional is true, "prev - 1" contains the keycode to be enacted upon
            if(prev != -1 && curr == 0){
                //manually adding in non-intentional-button-repetition negation
                if(last_successful_code == -1){
                     last_successful_code = prev;
                }else if(last_successful_code == prev){
                    last_successful_code = -1;
                    continue;
                }else{
                    last_successful_code = -1;
                }
                
                char buff[40];
                snprintf(buff,40,"xdotool key %s",keybind[prev-2]);
                //making the system call to simulate keyboard input of keybinds 
                system(buff);
            }
        }
        
        //Giving cpu a 1 millisecond break
        usleep(1000);
    }

    return 0;
}


//Handles external interrupts and closes input file
static void signal_handler(const int sig){
    if(sig == SIGTERM || sig == SIGINT){
        close(fd_to_be_closed);
        exit(0);
    }
}
