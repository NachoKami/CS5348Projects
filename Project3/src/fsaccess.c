//Filename: fsaccess.c

//Team Members:Sean Kennedy and Tyler Heald

//UTD_ID: 2021388327 and 2021360768

//NetID: smk170630 and tch170130

//Class: CS 5348.001

//Project: Project 3

/***********************************************************************
 
 This program allows user to do two things: 
   1. initfs: Initilizes the file system and redesigning the Unix file system to accept large 
      files of up tp 4GB, expands the free array to 152 elements, expands the i-node array to 
      200 elemnts, doubles the i-node size to 64 bytes and other new features as well.
   2. Quit: save all work and exit the program.
   
 User Input:
     - initfs (file path) (# of total system blocks) (# of System i-nodes)
     - q
     
 File name is limited to 14 characters.
 ***********************************************************************/

#include "fsaccess.h"
#include "fs_file.h"

// Note: These flags are all expressed in octal
unsigned short inode_alloc_flag = 0100000;
unsigned short dir_flag = 040000;
unsigned short dir_large_file = 010000;
unsigned short dir_access_rights = 000777; // User, Group, & World have all access privileges 
unsigned short INODE_SIZE = 64; // inode has been doubled

int main() {

	// Setting the starting directory to root
	current_dir.inode = 1;
	strcpy(current_dir.filename,"");
 
  char input[INPUT_SIZE];
  char *splitter;
  unsigned int numBlocks = 0, numInodes = 0;
  char *filepath;
  printf("Size of super block = %lu , size of i-node = %lu size of dir_type = %lu\n",sizeof(superBlock),sizeof(inode),sizeof(dir_type));
  printf("Enter command:\n");
  
  while(1) {
	
		// Indicator that the system is ready for input.
		// printf("currentNode: %d $ ",current_dir.inode);
		printf("$ ");
  
    scanf(" %[^\n]s", input);
    splitter = strtok(input," "); // Gets the next token

		// 2 possible commands...
		// initfs: Initialize the filesystem
		// q: Record the current state to disk
		// 	  and close the program.
    
    if(strcmp(splitter, "initfs") == 0){
    
				// Preinitializes the filesystem
				// Arguments...
				// filepath
				// #blocks
				// #inodes
        preInitialization();
        splitter = NULL;
                       
    } 
		else if (strcmp(splitter, "q") == 0) {
   
			 // If the file-descriptor has been opened from initfs,
			 // write superBlock to the correct BLOCK location
			 // (block #2)
       lseek(fileDescriptor, BLOCK_SIZE, 0);
       write(fileDescriptor, &superBlock, BLOCK_SIZE);
       return 0;
     
    } 
		else if (strcmp(splitter, "mkdir") == 0) {

			// If v6-dir does not exist, create the directory and set its first two entries to . and ..
			//
			// Arguments
			// v6-dir
			fs_mkdir();
			splitter = NULL;

		} 
		else if (strcmp(splitter, "rmdir") == 0) {

			// Remove the directory specified (dir, in this case).
			//
			// Arguments
			// dir
			fs_rmdir();
			splitter = NULL;

		} 
		else if (strcmp(splitter, "ls") == 0) {

			// List the contents of the current directory.
			fs_ls();
			splitter = NULL;

		}	
		else if (strcmp(splitter, "pwd") == 0) {

			// List the full pathname of the current directory.
			fs_pwd();
			splitter = NULL;

		}	
		else if (strcmp(splitter, "cd") == 0) {

			// Change the current (working) directory to the dirname.
			fs_cd();
			splitter = NULL;

		}	
		else if (strcmp(splitter, "cpin") == 0) {

			// Change the current (working) directory to the dirname.
			fs_cpin();
			splitter = NULL;

		}
    		else if(strcmp(splitter, "cpout") == 0) {
			//copy the specified v6 file to an exterior path
			fs_cpout();
			splitter = NULL;
		}
		else if(strcmp(splitter, "rm") == 0) {
			//remove file
			rm();
			splitter = NULL;
		}	
		else if (strcmp(splitter, "open") == 0) {

			// Change the current (working) directory to the dirname.
			fs_open();
			splitter = NULL;

		}	
    		else if(strcmp(splitter, "listBlocks") == 0) {
			//DEBUG function to list the entries of blocks (non-large file)
			ls_help();
			splitter = NULL;
		}
		else { // Function provided is not valid

			printf( RED "ERROR: " RESET "%s is not a valid command.\n", splitter);
			splitter = NULL;

		}
  }
}
