# Linux-Razer-Naga
Source code for handling Razer Naga related things. 

These programs are written in C, are tested thoroughly, and work beautfully, but you might need to tweak a couple things since these programs work close to hardware.



#Breakdown of razer_naga_sidekey_driver.c 

If you are new to C, don't understand the basics of structures, or don't understand basic file I/O, this might be a little hard to follow so bear with me. I'm going to try to go through the code from top to bottom.

A quick side note: this program is intended to run as a startup application. On my linux distro I have a literal "Startup Applications" app that does just that. This program, once compiled, is meant to be 

**95% of the time, if you try compiling this code word for word, it will not work as intended**. This is because the "INPUT_PATH", or the path to the file that reads a certain HID, is not the correct file that reads from the sidekeys of the Naga. The event number will be different for nearly everyone. Try going through the list of event numbers until you find the one that corresponds to the sidekeys of the Naga. If you use this program to find it, make sure to comment out the daemon statement ```//daemon(0,1);``` so that you can easily (ctrl+z) kill the program. I included code to close the opened file if an event such as this occurs (I will talk more about signals later) so that you regain access to certain HID functionality. For me the "event9" file was the correct input file: 
```c
#define INPUT_PATH "/dev/input/event9"
```

Next up are the keybinds. These keybinds will be the keys associated with the pressing of the 1-12 sidekeys on your Naga. **You will need "xdotool" (sudo apt-get install xdotool), a program that simulates keyboard input based on shell commands, for this program to work.** The basic shell command this program uses is "xdotool key KEYBIND_#". In the source code, you can change the keybinds in the macro quotes to whatever you want, for me I like to have the F1-F12 keys bound. **You can have multiple modifiers bound to any key. For example, you could do:**
```c
#define KEYBIND_4       "ctrl+alt+F4"
```


