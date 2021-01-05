//Filename: fs_navigation.c

//Team Members:Sean Kennedy and Tyler Heald

//UTD_ID: 2021388327 and 2021360768

//NetID: smk170630 and tch170130

//Class: CS 5348.001

//Project: Project 3

#include "fsaccess.h"
#include "utils.h"

int fs_ls() {
	//read the current inode
	inode_type currentInode;
	lseek(fileDescriptor, (2*BLOCK_SIZE) + (current_dir.inode-1)*INODE_SIZE, 0);
	read(fileDescriptor, &currentInode, sizeof(inode_type));

	//temp dir_type to read the current dir entry
	dir_type currentEntry;
	//temp inode to get info about the item in the block
	inode_type tempInode;
	//iterating over all of the entries in the directory
	int i;
	int bytesRead;
	for(i = 0; i < 11; i++) {
		//printing the addr number
		bytesRead = 0;
		//lseek(fileDescriptor, currentInode.addr[i], 0);
		//Reading trough each block
		while(bytesRead < 1024) {
			lseek(fileDescriptor, currentInode.addr[i] + bytesRead, 0);
			read(fileDescriptor, &currentEntry, sizeof(dir_type));
			bytesRead += sizeof(dir_type);
			//Checking if it exists
			if(currentEntry.inode != 0) {
				//Checking if its a dir
				lseek(fileDescriptor, (2*BLOCK_SIZE)+(currentEntry.inode-1)*INODE_SIZE, 0);
				read(fileDescriptor, &tempInode, sizeof(inode_type));
				if(tempInode.flags & dir_flag) { 
					printf(BLUE"%s\t"RESET, currentEntry.filename);
				}
				else { 
					printf("%s\t", currentEntry.filename);
				}
			}
		}
	}
	//Done reading all blocks
	printf("\n");
}


int fs_pwd_recursive(dir_type directory) {
	// Helper function to fs_pwd()
	if (directory.inode == 1) {
		// Base case of root directory
		printf("/");
		return 0;
	} else {
		// sleep(1);
		// Get the inode for inode_number
		int inode_index = directory.inode - 1;
		inode_type currentInode;
		lseek(fileDescriptor, (2 * BLOCK_SIZE) + (inode_index * INODE_SIZE), 0);
		read(fileDescriptor, &currentInode, INODE_SIZE);
		
		// Get the parent directory from the inode
		// TODO: We need to be able to deal with large directories.
		dir_type parent_dir;
		lseek(fileDescriptor, currentInode.addr[0] + sizeof(dir_type), 0);
		read(fileDescriptor, &parent_dir, sizeof(dir_type));

		// Recurse through remaining parent directories
		fs_pwd_recursive(parent_dir);

		// Print the current directory name
		char current_filename[14];
		inode_type parentINode;
		lseek(fileDescriptor, (2*BLOCK_SIZE) + ( (parent_dir.inode - 1) * sizeof(inode_type) ), 0);
		read(fileDescriptor, &parentINode, INODE_SIZE);
		dir_type tempDir;

		for (int i = 0; i < 8; i++) {
			if (parentINode.addr[i] == 0) {
				printf(RED"Error: "RESET"No address.\n");
				return -1;
			}
			for (int j = 0; j < 16; j++) {
				lseek(fileDescriptor, parentINode.addr[i] + ( j * sizeof(dir_type) ), 0);
				if (read(fileDescriptor,&tempDir,sizeof(dir_type)) != sizeof(dir_type)) {
					// Directory does not exist
					printf(RED"Error: "RESET"Directory does not exist.\n");
					return -1;
				}
				if (tempDir.inode == directory.inode) {
					printf("%s/",tempDir.filename);
					return 0;
				}
			}
		}
	}
}

int fs_pwd() {
	// List the full pathname of the current directory.
	fs_pwd_recursive(current_dir);
	printf("\n");
	return 0;
}

int fs_cd() {
	// Change the current (working) directory to the dirname.
	
	char *path;
	if ((path = strtok(NULL," ")) == NULL) {
		printf(RED "Error: " RESET "Please enter a directory.\n");
		return 1;
	}
	dir_type temp_dir;
	if (path_to_dir(path,current_dir,&temp_dir)) {
		printf(RED "Error: " RESET "Invalid directory.\n");
		return 1;
	} else {

		// Check if is a directory
		inode_type tempInode;
		lseek(fileDescriptor, (2*BLOCK_SIZE)+((temp_dir.inode-1)*sizeof(inode_type)),0);
		read(fileDescriptor,&tempInode,sizeof(inode_type));
		if ((tempInode.flags & dir_flag) == 0) {
			printf(RED"Error: "RESET"%s is not a directory.\n",path);
			return -1;
		}

		current_dir.inode = temp_dir.inode;
		strcpy(current_dir.filename,temp_dir.filename);
		return 0;
	}
	
}
