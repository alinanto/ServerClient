#!/bin/bash

#Script to compile both server and client programs
#Then opens server program in a new terminal
#Waits for 3 seconds
#Opens the client program in new terminal

gcc server.c -o "bin/server.o" -lm -w
gcc client.c -o "bin/client.o" -lm -w

gnome-terminal -e "bin/server.o 4567"
clear
sleep 3
gnome-terminal -e "bin/client.o LOCALHOST 4567"
clear
