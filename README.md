# Linux-Razer-Naga
Source code for handling Razer Naga related things. 

These programs are written in C, are tested thoroughly, and work beautfully, but you might need to tweak a couple things since these programs work close to hardware.



#Breakdown of razer_naga_sidekey_driver.c 

If you are new to C, don't understand the basics of structures, or don't understand basic file I/O, this might be a little hard to follow so bear with me. I'm going to try to go through the code from top to bottom.

A quick side note: **since this program works with files outside the home directory, you need to execute it with sudo.** This program is intended to run as a startup application. On my linux distro I have a literal "Startup Applications" app that does just that. I type in a command and it executes it on startup. The main problem is the sudo part, so I have this command in my startup applications: 
```
gnome-terminal -e "sudo Razer/razer-naga-sidekey-driver"
```
The 'gnome-terminal -e' opens up a terminal (you may use a different terminal so you may have to replace 'gnome-terminal') and executes the command in quotations. Since this program turns into a daemon, the terminal will automatically exit out, but only after the password to sudo is entered. This leads to the final problem, I need to type in my sudo password on every startup. If computer security is a priority for you, then you will just have to live with this or do some fancy stuff, otherwise, you can type in the terminal: "sudo visudo" and add the following line to the sudo section so that you don't have to type in your password when calling sudo with this file: ``` <your username> ALL=NOPASSWD: <location of this file> ```

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
If you are curious about low-level linux i/o, just do a quick google search of the functions open() read() write() and close(). In summary, I created a function at the end of the source code file called "signal_handler" that handles signals by closing the file being operated on. A signal is something a computer passes to every program when unique events occur such as a computer shutting down, in which the computer will send a SIGTERM signal, which is a polite way to ask the program to terminate before it kills the PID. SIGINT is the signal sent when you ctrl+z or ctrl+d a program running in console. Hopefully you know the dangers of not closing an opened file when you are done with it. 


