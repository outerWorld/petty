# *petty*


## Description
It is a small library for self use. It's written by c++ language, however, c language style is also used. I want this project can help me work more effectively, and i would like it help you too.

## history
###  ~2013-08-18 
I need capture data from ethernet devices for data collecting, however, everytime i wrote an application, even small one, i need write the same code to call libpcap, so i encapsulated it for conveniency. 

* xcap
* cmdline
* tlog
* buff

### 2013-08-20 ~ 2013-08-25
For special purpose, i need to involve to the running programs,  to get its status, or to change the state of some *SWITCHES* to control the running process flow, so i need a *xshell* class. The xshell class was designed to accept multiple visitors and support multi-threads.   

* xshell
* xcmd
* cmd_base
* xsh_cmd
* xcontext
