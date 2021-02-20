# Makefile for Writing Make Files Example
 
# *****************************************************
# Variables to control Makefile operation
 
CC = g++
CFLAGS = -std=c++11 -Wall -g
 
# ****************************************************
# Targets needed to bring the executable up to date
 
main: main.o
	$(CC) $(CFLAGS) -o main main.o
 
# The main.o target can be written more simply
 
main.o: main.cpp
	$(CC) $(CFLAGS) -c main.cpp

clean:
	rm -f main.o main
