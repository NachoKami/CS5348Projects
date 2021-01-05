//Filename: fs_directories.c

//Team Members:Sean Kennedy and Tyler Heald

//UTD_ID: 2021388327 and 2021360768

//NetID: smk170630 and tch170130

//Class: CS 5348.001

//Project: Project 3

#include "fsaccess.h"
#include "utils.h"
#include "regex.h"

int fs_mkdir_helper(int parentDirectory, char* dirName) {

	inode_type tempINode;
	lseek(fileDescriptor, (2 * BLOCK_SIZE) + ( (parentDirectory - 1) * sizeof(inode_type) ), 0);
	read(fileDescriptor, &tempINode, INODE_SIZE);

	dir_type *temp_dir = malloc(sizeof(dir_type));
	// Search linearly through current directory assuming we do NOT have a large file
	for (int i = 0; i < 8; i++) {
		// if (tempINode.addr[i] == 0) {
		// 	printf("Need to allocate a new directory, and need a new free block.\n");
		// 	return 0; // For Now
		// }
		lseek(fileDescriptor, tempINode.addr[i], 0);
		for (int j = 0; j < 16; j++) {

			if (read(fileDescriptor,temp_dir,sizeof(dir_type)) == sizeof(dir_type) &&
					temp_dir->inode == 0) {
				// Directory does not exist
				
				// Take free iNode number
				int newINodeNumber = getFreeINode(); // TODO: Implement
				
				// Configure iNode
				inode_type *newINode = malloc(sizeof(inode_type));
				// TODO: Make sure all these configurations are correct.
				newINode->flags = dir_flag | inode_alloc_flag | dir_large_file | dir_access_rights;
				newINode->nlinks = 0;
				newINode->uid = 0;
				newINode->gid = 0;
				newINode->size = INODE_SIZE;
				newINode->addr[0] = getFreeDataBlockAddress(); // TODO: Implement

				lseek(fileDescriptor, (2 * BLOCK_SIZE) + ( (newINodeNumber - 1) * INODE_SIZE ), 0);
				write(fileDescriptor, newINode, sizeof(inode_type));

				// Configure data block
				dir_type *self = malloc(sizeof(dir_type));
				self->inode = newINodeNumber;
				self->filename[0] = '.';
				self->filename[1] = '\0';

				dir_type *parent = malloc(sizeof(dir_type));
				parent->inode = parentDirectory;
				parent->filename[0] = '.';
				parent->filename[1] = '.';
				parent->filename[2] = '\0';

				lseek(fileDescriptor, newINode->addr[0], 0);
				//Writing the self and parent directory entries to the data block received above
				write(fileDescriptor, self, sizeof(dir_type));
				write(fileDescriptor, parent, sizeof(dir_type));

				// Adding directory entru
				temp_dir->inode = newINodeNumber;
				strcpy(temp_dir->filename,dirName);
				lseek(fileDescriptor, tempINode.addr[i] + ( j * sizeof(dir_type) ), 0);
				write(fileDescriptor, temp_dir, sizeof(dir_type));

				// // Debug
				// unsigned short tempVal;
				// lseek(fileDescriptor, tempINode.addr[i], 0);
				// for (int a = 0; a < 32; a++) {
				// 	for (int b = 0; b < 8; b++) {
				// 		read(fileDescriptor, &tempVal, sizeof(unsigned int));
				// 		printf("%d",tempVal);
				// 	}
				// 	printf("\n");
				// }

				return 0;
			}
			if (strcmp(temp_dir->filename,dirName) == 0) {
				printf(RED"Error: "RESET"This directory already exists.\n");
				return -1;
			}
		}
	}
	return 0;
}

int fs_mkdir() {
	// If v6-dir does not exist, create the directory and set its first two entries to . and ..
	//
	// Arguments
	// v6-dir
	//
	// Behavior...
	// / -> error (Case 1)
	// a or a/ -> target inode is current directory (Case 2)
	// /a or /a/ -> target inode is 1 (Case 3)
	// b/a or b/a/ -> target inode is b's (Case 4)
	
	char *v6_dir;
	if ((v6_dir = strtok(NULL," ")) == NULL) {
		printf(RED "Error: " RESET "Please enter a directory.\n");
		return 0;
	}

	if (strlen(v6_dir) == 1 && v6_dir[0] == '/') {
		// Case 1
		printf(RED "Error: " RESET "Invalid directory.\n");
		return 0;
	}

	char newDir[1024];
	char dirPath[1024];
	int targetPath;

	char *case2RegexString = "^([[:alnum:]_.]+)/?$";
	size_t case2Groups = 2;
	regex_t case2Compiled;
	regmatch_t case2Array[case2Groups];

	char *case3RegexString = "^/([[:alnum:]_.]+)/?$";
	size_t case3Groups = 2;
	regex_t case3Compiled;
	regmatch_t case3Array[case3Groups];

	char *case4RegexString = "^([[:alnum:]_/.]+)/([[:alnum:]_]+)/?$";
	size_t case4Groups = 3;
	regex_t case4Compiled;
	regmatch_t case4Array[case4Groups];

	int error;
	if (error = regcomp(&case2Compiled, case2RegexString, REG_EXTENDED)) {
		char errorBuffer[100];
		regerror(error, &case2Compiled, errorBuffer, 100);
		printf(RED "Error: " RESET "regcomp() failed with %s\n",errorBuffer);
		return 1;
	}
	if (error = regcomp(&case3Compiled, case3RegexString, REG_EXTENDED)) {
		char errorBuffer[100];
		regerror(error, &case3Compiled, errorBuffer, 100);
		printf(RED "Error: " RESET "regcomp() failed with %s\n",errorBuffer);
		return 1;
	}
	if (error = regcomp(&case4Compiled, case4RegexString, REG_EXTENDED)) {
		char errorBuffer[100];
		regerror(error, &case4Compiled, errorBuffer, 100);
		printf(RED "Error: " RESET "regcomp() failed with %s\n",errorBuffer);
		return 1;
	}

	if ((error = regexec(&case3Compiled, v6_dir, case3Groups, case3Array, 0)) == 0) {
		// Case 3
		strcpy(newDir,v6_dir);
		newDir[case3Array[1].rm_eo] = 0;
		char *temp1 = "/";
		char *temp2 = newDir+case3Array[1].rm_so;
		if (fs_mkdir_helper(1,temp2) == -1) {
			printf(RED"Error: "RESET"Directory already exists.\n");
		}
	} 
	
	else if ((error = regexec(&case2Compiled, v6_dir, case2Groups, case2Array, 0)) == 0) {
		// Case 2
		strcpy(newDir,v6_dir);
		newDir[case2Array[1].rm_eo] = 0;
		char *temp1 = "";
		char *temp2 = newDir+case2Array[1].rm_so;
		if (fs_mkdir_helper(current_dir.inode,temp2) == -1) {
			printf(RED"Error: "RESET"Directory already exists.\n");
		}
	} 
	
	else if ((error = regexec(&case4Compiled, v6_dir, case4Groups, case4Array, 0)) == 0) {
		// Case 4
		strcpy(dirPath, v6_dir);
		dirPath[case4Array[1].rm_eo] = 0;
		strcpy(newDir, v6_dir);
		newDir[case4Array[2].rm_eo] = 0;
		char *temp1 = dirPath + case4Array[1].rm_so;
		char *temp2 = newDir + case4Array[2].rm_so;
		dir_type targetDirectory;
		if (path_to_dir(temp1,current_dir,&targetDirectory)) {
			printf(RED"Error: "RESET"Invalid path.\n");
			return 1;
		}
		if (fs_mkdir_helper(targetDirectory.inode,temp2) == -1) {
			printf(RED"Error: "RESET"Directory already exists.\n");
			return 1;
		}
	} 
	
	else {
		// char errorBuffer[100];
		// regerror(error, &case3Compiled, errorBuffer, 100);
		// printf(RED "Error: " RESET "regexec() failed with %s\n",errorBuffer);
		printf(RED "Error: " RESET "Please enter a valid path.\n");
		return 1;
	}

	return 0;
}

int fs_rmdir() {
	//DEBUG
	//printf("in rmdir\n");
	// Remove the directory specified (dir, in this case).
	//
	// Arguments
	// dir
	//Getting the directory name the user input
	char *dirName;
	if((dirName = strtok(NULL, " ")) == NULL) {
		printf(RED "Please also input a directory name\n");
		return -1;
	}
	//Find the directory file
	//temp inode to store the inode of the current directory
	inode_type currentInode;
	//temp dir_type to store the currently being iterated over dir entry
	dir_type currentEntry;

	//Reading in the current inode
	lseek(fileDescriptor, (2*BLOCK_SIZE) + (current_dir.inode-1)*INODE_SIZE, 0);
	read(fileDescriptor, &currentInode, INODE_SIZE);

	//Iterate over directory entries to find the selected one
	//var to iterate over 1024 bytes of a block
	int bytesRead;
	int i;
	for(i = 0; i < 11; i++) {
		//Only operating if addr[i] is used
		if(currentInode.addr[i] != 0) {
			//DEBUF
			//printf("in addr for\n");
			bytesRead = 0;
			//Iterating through blocks of entries for directory
			lseek(fileDescriptor, currentInode.addr[i], 0);
			//Reading all entries in the block
			while(bytesRead < 1024) {
				//Storing offset of this entry in the block
				int offset = bytesRead;
				read(fileDescriptor, &currentEntry, sizeof(dir_type));
				//incrementing bytesRead
				bytesRead += sizeof(dir_type);
				//DEBUG
				//printf("filename: %s\tdirName: %s\n", currentEntry.filename, dirName);
				//Checking if entry is the right name
				if(strcmp(currentEntry.filename, dirName) == 0) {
					//DEBUG
					//printf("found file\n");
					//Checking that the file is a directory
					inode_type dirInode;
					lseek(fileDescriptor, (2*BLOCK_SIZE) + (currentEntry.inode-1)*INODE_SIZE, 0);
					read(fileDescriptor, &dirInode, INODE_SIZE);
					//checking for directory flag
					if(!(dirInode.flags & dir_flag)) {
						//print error for not being a directory
						printf(RED "Error: " RESET "%s is not a directory\n", dirName);
						return -1;
					}
					//Checking that the directory is empty and deleting if it is
					if((rmdir_helper(currentEntry.inode)) == 0) {
						//DEBUG
						//printf("Removing dir: %s at %i\n", currentEntry.filename, currentEntry.inode);
						//Succesful execution
						//delete the directory
						char inodeBuffer[sizeof(inode_type)];
						memset(&inodeBuffer, 0, sizeof(inode_type));
						lseek(fileDescriptor, (2*BLOCK_SIZE) + (currentEntry.inode-1)*INODE_SIZE, 0);
						write(fileDescriptor, &inodeBuffer, sizeof(inode_type));

						//Now delete the file entry
						char dirBuffer[sizeof(dir_type)];
						memset(&dirBuffer, 0, sizeof(dir_type));
						lseek(fileDescriptor, currentInode.addr[i] + offset, 0);
						write(fileDescriptor, &dirBuffer, sizeof(dir_type));
						return 0;
					}
					else {
						//directory is not empty
						//print error and return
						printf(RED "Error: " RESET "%s is not empty\n", dirName);
						return -1;
					}
				}
			}
		}
	}
	return 0;
}

//Helper function to check if a directory is empty.
//Takes the inode of the directory file as its argument
int rmdir_helper(int dirInodeVal) {
	//DEBUG
	//printf("in helper\n");
	//Read in the inode of the directory
	inode_type dirInode;
	lseek(fileDescriptor, (2*BLOCK_SIZE)+(dirInodeVal-1)*INODE_SIZE, 0);
	read(fileDescriptor, &dirInode, sizeof(inode_type));

	//Iterate over all of the entries to see if there are any besides . and ..
	//var to iterate over 1024 bytes of a block
	int bytesRead;
	//temp dir_type to read entry
	dir_type currentEntry;
	int i;
	for(i = 0; i < 11; i++) {
		bytesRead = 0;
		//Iterating through blocks of entries for directory
		lseek(fileDescriptor, dirInode.addr[i], 0);
		//Reading all entries in the block
		while(bytesRead < 1024) {
			//Storing offset of this entry in the block
			int offset = bytesRead;
			read(fileDescriptor, &currentEntry, sizeof(dir_type));
			//incrementing bytesRead
			bytesRead += sizeof(dir_type);
			//Checking if entry is empty
			//DEBUG
			//printf("%i, %s\n", currentEntry.inode, currentEntry.filename);
			if(currentEntry.inode != 0 && !(strcmp(currentEntry.filename, ".") == 0 || strcmp(currentEntry.filename, "..") == 0)) {
				//directory is not empty
				//return error
				return -1;
			}
		}
	}
	//Read through all without finding a non empty entry, so delete the directory
	return 0;
}


//Debug function to read the file names of all the entries in the current directory
void ls_help() {
	//read the current inode
	inode_type currentInode;
	lseek(fileDescriptor, (2*BLOCK_SIZE) + (current_dir.inode-1)*INODE_SIZE, 0);
	read(fileDescriptor, &currentInode, sizeof(inode_type));

	//temp dir_type to read the current dir entry
	dir_type currentEntry;
	//temp inode to get infor about the item in the block
	inode_type tempInode;
	//iterating over all of the entries in the directory
	int i;
	int bytesRead;
	for(i = 0; i < 11; i++) {
		//printing the addr number
		printf("Entered block %i at %i\n", i, currentInode.addr[i]);
		bytesRead = 0;
		int fileNumber = 1;
		int dirOrNot;
		//lseek(fileDescriptor, currentInode.addr[i], 0);
		//Reading trough each block
		while(bytesRead < 1024) {
			lseek(fileDescriptor, currentInode.addr[i] + bytesRead, 0);
			read(fileDescriptor, &currentEntry, sizeof(dir_type));
			bytesRead += sizeof(dir_type);
			//Checking if its a dir
			lseek(fileDescriptor, (2*BLOCK_SIZE)+(currentEntry.inode-1)*INODE_SIZE, 0);
			read(fileDescriptor, &tempInode, sizeof(inode_type));
			if(tempInode.flags & dir_flag) { dirOrNot = 1; }
			else { dirOrNot = 0; }
			//Printing inode stats
			printf("%i filename: %s\tinode: %i\tDir(yes=1): %i\n", fileNumber, currentEntry.filename, currentEntry.inode, dirOrNot);
		       	fileNumber++;	
		}
	}
}
