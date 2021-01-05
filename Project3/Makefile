# Filename: fs_directories.c
# Team Members:Sean Kennedy and Tyler Heald
# UTD_ID: 2021388327 and 2021360768
# NetID: smk170630 and tch170130
# Class: CS 5348.001
# Project: Project 3

# Files
INCL 	= $(wildcard include/*.h)
SRC		=	$(wildcard src/*.c)
OBJ		=	$(SRC:src/%.c=obj/%.o)
EXE		= bin/fsaccess.exe

# Compiler, Linker Defines
CC			= gcc --std=c99
CFLAGS	=	-Wall -I include -g
LDFLAGS	=	-o $(EXE)

all: $(EXE)

clean:
	rm $(OBJ) $(EXE) *.fs

obj/%.o: src/%.c
	$(CC) -c $(CFLAGS) src/$*.c -o obj/$*.o

$(EXE): $(OBJ)
	$(CC) $(LDFLAGS) $(OBJ)

$(OBJ): $(INCL)
