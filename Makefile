# Makefile for Writing Make Files Example
 
# *****************************************************
# Variables to control Makefile operation
 
CC = g++
CFLAGS = -std=c++14 -Wall -O3 -ffast-math -march=native -fwhole-program
 
# ****************************************************
# Targets needed to bring the executable up to date
 
main: main.o
	$(CC) $(CFLAGS) -o main main.o -lSDL2
 
# The main.o target can be written more simply
 
main.o: main.cpp vec3.h ray.h hittable.h triangle.h sphere.h aabb.h bvh.h material.h
	$(CC) $(CFLAGS) -c main.cpp

clean:
	rm -f main.o main
