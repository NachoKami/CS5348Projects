//Filename: utils.h

//Team Members:Sean Kennedy and Tyler Heald

//UTD_ID: 2021388327 and 2021360768

//NetID: smk170630 and tch170130

//Class: CS 5348.001

//Project: Project 3

#ifndef UTILS_H
#define UTILS_H

#include "fsaccess.h"

int path_to_dir(char *path, dir_type base, dir_type * output);
int getFreeINode();
int getFreeDataBlockAddress();

#endif
