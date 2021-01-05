//Filename: utils.c

//Team Members:Sean Kennedy and Tyler Heald

//UTD_ID: 2021388327 and 2021360768

//NetID: smk170630 and tch170130

//Class: CS 5348.001

//Project: Project 3

#include "utils.h"

int path_to_dir(char *path, dir_type base, dir_type *output) {
	// Returns the inode given some directory path
	//
	// Recursive function that follows these general rules...
	// 	pti( "/a", x ) -> pti( "a", 1 );
	// 	pti( "", x ) -> x;
	// 	pti( "a/b", x ) -> pti( "b", newX );
	// 	pti( "a", x ) -> pti( "", newX );
	// 	pti( "invalid", x ) -> -1;

	if (path == NULL || strlen(path) == 0) {
		output->inode = base.inode;
		strcpy(output->filename, base.filename);
		return 0;
	}
	
	if (path[0] == '/') {
		// Path "//..." is invalid
		if (path[1] == '/') { return -1; }

		char* newPath = path + 1;
		return path_to_dir(newPath, root, output);
	}

	inode_type tempINode;
	lseek(fileDescriptor, (2 * BLOCK_SIZE) + ( (base.inode - 1) * sizeof(inode_type) ), 0);
	read(fileDescriptor, &tempINode, INODE_SIZE);

	char *targetDirName = strtok(path, "/");
	dir_type tempDir;

	// Search linearly through current directory assuming we do NOT have a large file
	for (int i = 0; i < 8; i++) {
		if (tempINode.addr[i] == 0) {
			printf(RED"Error: "RESET"No address.\n");
			return -1;
		}
		for (int j = 0; j < 16; j++) {
			lseek(fileDescriptor, tempINode.addr[i] + ( j * sizeof(dir_type) ), 0);
			if (read(fileDescriptor,&tempDir,sizeof(dir_type)) != sizeof(dir_type)) {
				// Directory does not exist
				printf(RED"Error: "RESET"Directory does not exist.\n");
				return -1;
			}
			if (strcmp(tempDir.filename,targetDirName) == 0) {
				char *newString = strtok(NULL,"");
				output->inode = tempDir.inode;
				strcpy(output->filename,tempDir.filename);
				return 0;
			}
		}
	}

	return -1;
}

int getFreeINode() {
	if (superBlock.ninode>0) {
		return superBlock.inode[--superBlock.ninode];
	}
	return -1;
}

int getFreeDataBlockAddress() {
	if (superBlock.nfree>0) {
		int blockNumber = superBlock.free[--superBlock.nfree];

		lseek(fileDescriptor,(2+blockNumber) * BLOCK_SIZE,0);
		dir_type null_dir;
		null_dir.inode = 0;
		null_dir.filename[0] = '\0';
		for (int i = 0; i < 32; i++) {
			write(fileDescriptor,&null_dir,16);
		}
		return (2 + blockNumber) * BLOCK_SIZE;
	}

	return -1;
}
