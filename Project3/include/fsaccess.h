//Filename: fsaccess.h

//Team Members:Sean Kennedy and Tyler Heald

//UTD_ID: 2021388327 and 2021360768

//NetID: smk170630 and tch170130

//Class: CS 5348.001

//Project: Project 3

// smk170630 | Sean Kennedy

#ifndef	FSACCESS_H
#define	FSACCESS_H

#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<stdlib.h>

#define FREE_SIZE 152  
#define I_SIZE 200
#define BLOCK_SIZE 1024    
#define ADDR_SIZE 11
#define INPUT_SIZE 256

// Superblock Structure

typedef struct {
  unsigned short isize;	// 2 bytes (number of blocks devoted to i-list)
  unsigned short fsize; // 2 bytes (first block not potentially available for allocation to file)
  unsigned short nfree; // 2 bytes (number of blocks in the free list)
  unsigned short ninode; // 2 bytes (number of free i-numbers in the inode array)
  unsigned int free[2000]; // 4 * 152 = 608 bytes (contains block numbers of free blocks)
  unsigned short inode[2000]; // 2 * 200 = 400 bytes (list of free inodes)
  char flock; // 1 byte
  char ilock; // 1 byte
  unsigned short fmod; // 2 bytes
  unsigned short time[2]; // 2 * 2 = 4 bytes
} superblock_type; // Adds up to 1024
// Padding is added to make it 1028 initially,
// so we need to rearrange the elements to be
// in groups of 8 bytes.

superblock_type superBlock;

// I-Node Structure

typedef struct {
unsigned short flags; // 2 bytes
unsigned short nlinks; // 2 bytes
unsigned short uid; // 2 bytes
unsigned short gid; // 2 bytes
unsigned int size; // 4 bytes
unsigned int addr[ADDR_SIZE]; // 4 * 11 = 44 bytes
unsigned short actime[2]; // 2 * 2 = 4 bytes
unsigned short modtime[2]; // 2 * 2 = 4 bytes
} inode_type; // Adds up to 64 bytes
// No issues with padding, so the actual
// space used is 64 bytes here as well

inode_type inode;

typedef struct {
  unsigned short inode; // 2 bytes
  unsigned char filename[14]; // 14 bytes
} dir_type; // Adds up to 16 bytes
// Actual amount used it also 16 bytes

dir_type root;

int fileDescriptor ;		//file descriptor 
// Note: These flags are all expressed in octal
unsigned short inode_alloc_flag;
unsigned short dir_flag;
unsigned short dir_large_file;
unsigned short dir_access_rights; // User, Group, & World have all access privileges 
unsigned short INODE_SIZE; // inode has been doubled

// File declarations
int preInitialization();
int initfs(char* path, unsigned short total_blcks,unsigned short total_inodes);
void add_block_to_free_list( int blocknumber , unsigned int *empty_buffer );
void create_root();

// Current state
dir_type current_dir; // Which iNode is currently being used

// User Commands
int fs_mkdir();
int fs_rmdir();
int fs_ls();
int fs_pwd();
int fs_cd();

// Colors
#define RESET "\033[0m"
#define RED "\033[31m"
#define BLUE "\033[34m"

#endif	/* fsaccess.h */
