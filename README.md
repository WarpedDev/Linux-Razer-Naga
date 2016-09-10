# Linux-Razer-Naga
Source code for handling Razer Naga related things. 



#Breakdown of razer_naga_sidekey_driver.c 

###Known Issues:
1. Daemon is terminated on system suspension.
2. Trouble making this program run on startup without having to type in sudo password without having to worry about security risks.

If you are new to C, don't understand the basics of structures, or don't understand basic file I/O, this might be a little hard to follow so bear with me. I'm going to try to go through the code from top to bottom.

A quick side note: **since this program works with files outside the home directory, you need to execute it with sudo.** This program is intended to run as a startup application. On my linux distro I have a literal "Startup Applications" app that does just that. I type in a command and it executes it on startup. The main problem is the sudo part, so I have this command in my startup applications: 
```
gnome-terminal -e "sudo Razer/razer-naga-sidekey-driver"
```
The 'gnome-terminal -e' opens up a terminal (you may use a different terminal so you may have to replace 'gnome-terminal') and executes the command in quotations. Since this program turns into a daemon, the terminal will automatically exit out, but only after the password to sudo is entered. This leads to the final problem, I need to type in my sudo password on every startup. **If computer security is a priority for you, and it should be, then you will just have to live with this or do some other fancy stuff.** As reddit user jtsiomb puts it: "It's a very bad idea, for a program running as root, to exec other programs without full absolute path. It's a matter of time before someone installs this setuid-root to skip the sudo requirement, without examining (or understanding) the source code, and creates a perfect privilege escalation vector." If you still don't care about computer security risks, then you can type in the terminal: "sudo visudo" and add the following line to the sudo section so that you don't have to type in your password when calling sudo with this file: ``` <your username> ALL=NOPASSWD: <location of this file> ```

**95% of the time, if you try compiling this code word for word, it will not work as intended**. This is because the "INPUT_PATH", or the path to the file that reads a certain HID, is not the correct file that reads from the sidekeys of the Naga. The event number will be different for nearly everyone. Try going through the list of event numbers until you find the one that corresponds to the sidekeys of the Naga. If you use this program to find it, make sure to comment out the daemon statement ```//daemon(0,1);``` so that you can easily (ctrl+z) kill the program. I included code to close the opened file if an event such as this occurs (I will talk more about signals later) so that you regain access to certain HID functionality. For me the "event9" file was the correct input file: 
```c
#define INPUT_PATH "/dev/input/event9"
```

Next up are the keybinds. These keybinds will be the keys associated with the pressing of the 1-12 sidekeys on your Naga. **You will need "xdotool" (sudo apt-get install xdotool), a program that simulates keyboard input based on shell commands, for this program to work.** The basic shell command this program uses is "xdotool key KEYBIND_#". In the source code, you can change the keybinds in the macro quotes to whatever you want, for me I like to have the F1-F12 keys bound. **You can have multiple modifiers bound to any key. For example, you could do:**
```c
#define KEYBIND_4       "ctrl+alt+F4"
```

Next is signal handling, low-level i/o with error handling, and daemons.
```c
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
```
If you are curious about low-level linux i/o, just do a quick google search of the functions open() read() write() and close(). In summary, I created a function at the end of the source code file called "signal_handler" that handles signals by closing the file being operated on. A signal is something a computer passes to every program when unique events occur such as a computer shutting down, in which the computer will send a SIGTERM signal, which is a polite way to ask the program to terminate before it kills the PID. SIGINT is the signal sent when you ctrl+z or ctrl+d a program running in console. Hopefully you know the dangers of not closing an opened file when you are done with it. The next statement is a doozie: ```ioctl(fd,EVIOCGRAB,1);```, just know that this function makes the opened file only readable by this program. This function is core, without it the computer would read the number pressed on the Naga, as well as the keybind. Finally, ```daemon(0,1);``` makes this program turn into a background process. The parameters don't impact the functionality of the program, it just helps with debugging. Do a quick google search if you are curious as to what each means.

Admitingly, the rest of the program is a little hard to follow and could be written better. I will focus more on the logic rather than what was actually written.
```c
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
```
The logic of the above code is as follows: This loop is infinite, but each iteration is guaranteed to wait at least 1 millisecond so the cpu doesn't go crazy. It's worth noting that the ```usleep(1000)``` doesn't make much of a difference in the program since no operation is executed unless the conditional is met, but it's good to have (after some testing, this program takes up 0.0% of cpu; no need to worry). Note that the read() function is reading a structure type defined in "/usr/include/linux/input.h". This structure type consists of a few cool things, but the only one relevant is the keycode element. 

If you try creating a program to solely read the keycodes of the event# input file and print them out to the screen, you will note that a single button push results in 3 structures being written to the input file by the OS. The keycode of the first structure is always 4 for me, the keycode of the second structure is the one that designates which button was pressed+1 (so if I press the sidekey 2 on my naga, I would see the keycode "3"), and the keycode of the 3rd structure is always 0. When a sidekey is held, 2 structures are written to the input file for every moment it is held. The keycode of the first structure designates which button was pressed+1, and the keycode in the second structure is always 0. Did you notice the pattern? Every 0 is preceded by the keycode that needs to be enacted upon. This means that for each iteration, you would have to keep track of the previous keycode, and see if the current keycode is 0. If it is 0, then the previous keycode is the keycode you want to work with. 

Finally, after creating a string with the proper terminal command for xdotool paired with the correct KEYBIND_#, The function system() is called, which is basically a function that executes the string in its parameters as if it were executed on the command line.

Hopefully you benefited from this explanation ;p
