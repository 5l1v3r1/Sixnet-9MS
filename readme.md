# Sixnet ET-9MS Hacking
## Getting started

So you want to mangle packets using a Sixnet ET-9MS network switch. Great.

Prerequisites:

- ELDK 4.1 from Denx - download from here: ftp://ftp.denx.de/pub/eldk/4.1/arm-linux-x86/distribution/README.html . We installed this on Fedora Core 7, which worked well.
- Libpcap headers from Libpcap 0.9.4. Other versions may work, unless there was an API change.
- libswm.so and libswfile.so from your network switch (see how to retrieve these files in the 'rooting' section below).

## Rooting your switch

To make changes to your switch firmware, you first need to root your switch.

The best way to root the switch is to use a directory traversal bug in the web server interface.

We have included root.tar.gz file in the /exploit/ directory. Upload this file. It will create a backdoor account in the switch with username 'backdoor' and password 'dblabs'.

You may then telnet or SSH to the switch and will have a root shell.

## Retrieving libswm and libswfile

These shared libraries are stored in /usr/lib/ . I recommend transferring them off of the switch using netcat:

&#35; cd /usr/lib

&#35; nc <your computer IP> 4444 < libswm.so.1.4.0

&#35; nc <your computer IP> 4444 < libswfile.so

Set up a netcat listener on port 4444 on your workstation to receive the files:

$ nc -l -p 4444 > libswm.so

$ nc -l -p 4444 > libswfile.so

## Compiling and linking

The following exports are needed to successfully compile the swm-test.c program
(TODO)

