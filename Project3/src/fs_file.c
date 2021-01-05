//Filename: fs_file.c

//Team Members:Sean Kennedy and Tyler Heald

//UTD_ID: 2021388327 and 2021360768

//NetID: smk170630 and tch170130

//Class: CS 5348.001

//Project: Project 3

#include "fsaccess.h"
#include "fs_file.h"
#include "sys/stat.h"

int fs_cpin() {
	// Taked a directory from the host filesystem and copies it into ours
	//
	// Arguments
	// path 
	// v6-path

	// Create a new iNode *
	// Open a file to read from *
	// Read 512 bytes at a time
	// For each block
	// 	Add to end of existing data
	// 	If no room for existing data and small
	// 		Convert to large file-type
	// 		Add to end of existing data

	char *path, *v6_path;
	int pathFd;
	inode_type *newFileINode = malloc(sizeof(inode_type));

	// Parsing arguments
	if ((path = strtok(NULL," ")) == NULL) {
		printf(RED"Error: "RESET"Missing path & v6_path.\n");
		return 0;
	}
	if ((pathFd = open(path,O_RDONLY)) == -1) {
		printf(RED"Error: "RESET"Unable to open file at %s\n",path);
		return 0;
	}
	if ((v6_path = strtok(NULL," ")) == NULL) {
		printf(RED"Error: "RESET"Missing v6_path.\n");
		return 0;
	}
	// // TODO: I will need to do something like this at some point
	// // 			to take into account different paths.
	// if (path_to_dir(v6_path,current_dir,v6_pathFile)==-1) {
	// 	// Error Case
	// 	return 0;
	// }

	// Create a new iNode
	inode_type dirINode;
	int newINodeNumber;
	lseek(fileDescriptor, (2*BLOCK_SIZE)+((current_dir.inode-1)*INODE_SIZE),0);
	read(fileDescriptor,&dirINode,INODE_SIZE);
	dir_type *temp_file = malloc(sizeof(dir_type));
	// Search linearly through current directory assuming it is not large
	for (int i = 0; i < 8; i++) {
		lseek(fileDescriptor, dirINode.addr[i],0);
		for (int j = 0; j < 16; j++) {
			if (read(fileDescriptor,temp_file,sizeof(dir_type)) == sizeof(dir_type) &&
				temp_file->inode == 0) {
				// Space for a new file exists
				
				// Take free iNode number
				newINodeNumber = getFreeINode();

				// Configure iNode
				// TODO: Make sure all thes configurations are correct
				newFileINode->flags = 0100000; // Plain file
				newFileINode->nlinks = 0;
				newFileINode->uid = 0;
				newFileINode->gid = 0;
				newFileINode->size = 0; // Initially no data allocated
				newFileINode->addr[0] = getFreeDataBlockAddress();

				lseek(fileDescriptor, (2*BLOCK_SIZE) + ((newINodeNumber-1)*INODE_SIZE),0);
				write(fileDescriptor, newFileINode, sizeof(inode_type));

				// Adding directory entry
				temp_file->inode = newINodeNumber;
				strcpy(temp_file->filename,v6_path);
				lseek(fileDescriptor, dirINode.addr[i] + (j*sizeof(dir_type)),0);
				write(fileDescriptor, temp_file, sizeof(dir_type));
				
				// Done with the loop
				i = 8;
				j = 16;
			}
		}
	}

	// Read 512 bytes at a time
	int output_size;
	char buffer[1024];
	while ((output_size=read(pathFd,buffer,1024))>0) {
		if (newFileINode->size < 8192) {
			// Keep small file
			unsigned short index = newFileINode->size / 1024;
			newFileINode->addr[index] = getFreeDataBlockAddress();
			newFileINode->size += output_size;
			// Update inode
			lseek(fileDescriptor,(2*BLOCK_SIZE) +((newINodeNumber-1)*INODE_SIZE),0);
			write(fileDescriptor,newFileINode,sizeof(inode_type));
			// Update file / data
			lseek(fileDescriptor,newFileINode->addr[index],0);
			write(fileDescriptor,buffer,output_size);
		} else if (newFileINode->size == 8192) {
			// Upgrade to large file
		} else {
			// Large file
		}
	}
	if (output_size == -1) {
		printf(RED"Error: "RESET"Unable to open file.\n");
	}

	return 0;
}

int fs_open() {

	// Search for a file in the current directory
	char *path;
	if ((path = strtok(NULL," ")) == NULL) {
		printf(RED"Error: "RESET"Missing path & v6_path.\n");
		return 0;
	}
	
	dir_type file;
	if (path_to_dir(path,current_dir,&file)) {
		printf(RED"Error: "RESET"Unable to open file.\n");
		return 1;
	}

	// Getting the inode
	inode_type fileInode;
	lseek(fileDescriptor,(2*BLOCK_SIZE)+((file.inode-1)*INODE_SIZE),0);
	read(fileDescriptor,&fileInode,sizeof(inode_type));
	if (fileInode.flags & dir_flag) {
		printf(RED"Error: "RESET"%s is a directory.\n",file.filename);
		return 1;
	}

	char buffer[1024];
	for (unsigned int block = 0; block <= fileInode.size/1024; block++) {
		lseek(fileDescriptor,fileInode.addr[block],0);
		read(fileDescriptor,buffer,1024);
		printf("%s",buffer);
	}

	printf("\n");

	return 0;
}

//Takes a file from this file system and copies it out
int fs_cpout() {
	//Arguments
	//
	//v6-path
	//path

	//find the specified file in this directory
	//get the inode of that entry
	//open a file with the specified path
	//
	//determine if file is large or not
	//read through all blocks of the file
	//write each block to the outside file
	
	//DEBUG
	//printf("in cpout\n");

	char *v6path;
	char *path;
	//getting the v6path
	if((v6path = strtok(NULL, " ")) == NULL) {
		printf(RED"Error: "RESET"Missing v6path and possibly path\n");
		return -1;
	}
	//Getting the path
	if((path = strtok(NULL, " ")) == NULL) {
		printf(RED"Error: "RESET"Missing path\n");
		return -1;
	}

	//Getting the inode of the current directory
	inode_type currentInode;
	lseek(fileDescriptor, (2*BLOCK_SIZE) + (current_dir.inode-1)*INODE_SIZE, 0);
	read(fileDescriptor, &currentInode, sizeof(inode_type));

	//dir_type to store currentEntry
	dir_type currentEntry;
	//inode_type to hold the inode of the file to be copied out
	inode_type cpFile;
	//Iterating over the directory entries
	int i = 0;
	int bytesRead;
	for(i = 0; i < 11; i++) {
		//Only reading through block if it isn't unused
		if(currentInode.addr[i] != 0) {
			//DEBUG
			//printf("in addr loop\n");
			bytesRead = 0;
			//Reading trough each block
			while(bytesRead < 1024) {
				lseek(fileDescriptor, currentInode.addr[i] + bytesRead, 0);
				read(fileDescriptor, &currentEntry, sizeof(dir_type));
				bytesRead += sizeof(dir_type);
				//Checking if it's the right file
				if(strcmp(currentEntry.filename, v6path) == 0) {
					//DEBUG
					//printf("Found the file\n");
					//Checking if it's a directory
					lseek(fileDescriptor, (2*BLOCK_SIZE)+(currentEntry.inode-1)*INODE_SIZE, 0);
					read(fileDescriptor, &cpFile, sizeof(inode_type));
					if(cpFile.flags & dir_flag) { 
						printf(RED"ERROR: "RESET"cannot copy a directory\n");
					}
					else { 
						//Open the file that this is going to be copied out to
						int fp = open(path, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
						//DEBUG
						//printf("Opened out file\n");
						//var for iterating over addr
						int i;
						//buffer to read data from this filesystem
						unsigned char buffer[BLOCK_SIZE/sizeof(unsigned char)];
						memset(buffer, 0, BLOCK_SIZE/sizeof(unsigned char));
						//Determine if it's a large file or not
						//Already have the inode
						//Checking if it's a large file
						if(cpFile.flags & dir_large_file) {
							//DEBUG
							//printf("large file\n");
							//file is large, has indirect blocks, and (potentially) a 2-level indirect at end
							//each block pointed to by addr[k] is a block of address
							//there will be 256 entries in addr[k] blocks
							//If addr[10] is used, then that is a double indirect block
							int k;
							for(k = 0; k < 10; k++) {
								//Only iterating over addr blocks that are used
								if(cpFile.addr[k] !=0) {
									//Loop through the addresses, read in a block, loop through the block
									//int to count blocks read
									int blocksCounted;
									//unsigned int to keep ttrack of block addresses
									unsigned int blockAddr;
									lseek(fileDescriptor, cpFile.addr[k], 0);
									for(blocksCounted = 0; blocksCounted < 256; blocksCounted++) {
										read(fileDescriptor, &blockAddr, sizeof(unsigned int));
										//Have the block address, need to read it in, if it's not 0
										if(blockAddr != 0) {
											int bytesReadFromFile;
											int bytesToRead;
											if((cpFile.size - (BLOCK_SIZE*(k*256 + blocksCounted))) > BLOCK_SIZE) {
												bytesToRead = BLOCK_SIZE;
											}
											else {
												bytesToRead = cpFile.size % BLOCK_SIZE;
											}
											lseek(fileDescriptor, &blockAddr, 0);
											//DEBUG
											//printf("writing %s to file\n", buffer);
											bytesReadFromFile = read(fileDescriptor, &buffer, bytesToRead);
											//write the block out
											write(fp, buffer, bytesReadFromFile);
											close(fp);
										}
									}
								}
							}
							//Now have to do the same for the 2 level indirect block, if it's used
							if(cpFile.addr[10] != 0) {
								lseek(fileDescriptor, cpFile.addr[10], 0);
								int blocksRead;
								unsigned int indirBlockAddr;
								for(blocksRead = 0; blocksRead < 256; blocksRead++) {
									read(fileDescriptor, &indirBlockAddr, sizeof(unsigned int));
									lseek(fileDescriptor, indirBlockAddr, 0);
									//int to count blocks read
									int blocksCounted;
									//unsigned int to keep ttrack of block addresses
									unsigned int blockAddr;
									lseek(fileDescriptor, cpFile.addr[k], 0);
									for(blocksCounted = 0; blocksCounted < 256; blocksCounted++) {
										read(fileDescriptor, &blockAddr, sizeof(unsigned int));
										//Have the block address, need to read it in, if it's not 0
										if(blockAddr != 0) {
											int bytesReadFromFile;
											int bytesToRead;
											if((cpFile.size - (BLOCK_SIZE*(blocksRead*256 + blocksCounted))) > BLOCK_SIZE) {
												bytesToRead = BLOCK_SIZE;
											}
											else {
												bytesToRead = cpFile.size % BLOCK_SIZE;
											}
											lseek(fileDescriptor, &blockAddr, 0);
											bytesReadFromFile = read(fileDescriptor, &buffer, bytesToRead);
											//write the block out
											write(fp, buffer, bytesReadFromFile);
											close(fp);
										}
									}
								}
							}
						}
						else {
							//DEBUG
							//printf("Not large file\n");
							//file is not large, has direct addresses to blocks
							//Iterating over blocks
							int j;
							for(j = 0; j < 11; j++) {
								//Only writing the block if the addr block is used
								if(cpFile.addr[j] != 0) {
									int bytesReadFromFile;
									int bytesToRead;
									if((cpFile.size - (BLOCK_SIZE*j)) > BLOCK_SIZE) {
										bytesToRead = BLOCK_SIZE;
									}
									else {
										bytesToRead = cpFile.size % BLOCK_SIZE;
									}
									lseek(fileDescriptor, cpFile.addr[j], 0);
									//Read the whole block
									bytesReadFromFile = read(fileDescriptor, &buffer, bytesToRead);
									//DEBUG
									//printf("writing %s\n", buffer);
									write(fp, buffer, bytesReadFromFile);
									//Closing the file
									close(fp);
								}
							}
						}
					}
				}
			}
		}
	}
}

//Command to remove a file
int rm() {
	//DEBUG
	//printf("in rm\n");
	//arguments
	//filename
	
	//reads in file name
	//finds file inode, if it exists
	//determines if it's a directory
	//determines if it's a large file
	//
	//Reading in the name of the file to delete
	char *fileName;
	if((fileName = strtok(NULL, " ")) == NULL) {
		printf(RED "Error: " RESET "Please enter a file name.\n");
		return 0;
	}
	//lseek to the start of the current directory
	lseek(fileDescriptor, (2*BLOCK_SIZE) + (current_dir.inode-1)*INODE_SIZE, 0);
	//temp inode to iterate over the files in the current directory
	inode_type currentInode;
	read(fileDescriptor, &currentInode, INODE_SIZE);
	//dir_type to iterate over the directory entries
	dir_type currentEntry;
	//Var to iterate over the 1024 bytes of a block
	int bytesRead;
	//Iterating through the addresses in the inode
	int i = 0;
	for(i = 0; i < 11; i++) {
		//DEBUG
		//printf("in addr loop\n");
		bytesRead = 0;
		//Iterating through the blocks of entries for the directory
		lseek(fileDescriptor, currentInode.addr[i], 0);
		//Reading all the entries in the block
		while(bytesRead < 1024) {
			//Storing the offset of this entry in the block
			int offset = bytesRead;
			read(fileDescriptor, &currentEntry, sizeof(dir_type));
			//incrementing bytesRead
			bytesRead += sizeof(dir_type);
			//Checking if the entry is the right name
			if(strcmp(currentEntry.filename, fileName) == 0) {
				//DEBUG
				//printf("Found file\n");
				//Checking that the file isn't a directory
				//Finding the inode of the file
				inode_type fileInode;
				lseek(fileDescriptor, (2*BLOCK_SIZE) + (currentEntry.inode-1)*INODE_SIZE, 0);
				read(fileDescriptor, &fileInode, INODE_SIZE);
				//Checking for directory flag
				if(fileInode.flags & dir_flag) {
					//Print is dir error
					printf(RED "Error: " RESET "%s is a directory\n", fileName);
					return 0;
				}
				//Delete the file
				//Iterate over the blocks of the file to free them
				//Checking if the file is large or nor
				if(fileInode.flags & dir_large_file) {
					//DEBUG
					//printf("large file\n");
					//Is large, need to do indirect blocks
					//each block pointed to by addr[k] is a block of address
					//there will be 256 entries in addr[k] blocks
					//If addr[10] is used, then that is a double indirect block
					int k;
					for(k = 0; k < 10; k++) {
						//only if the address block is used
						if(fileInode.addr[k] != 0) {
							//Loop through the addresses, read in a block, loop through the block
							//int to count blocks freed
							int blocksFreed;
							//unsigned int to keep ttrack of block addresses
							unsigned int blockAddr;
							lseek(fileDescriptor, fileInode.addr[k], 0);
							for(blocksFreed = 0; blocksFreed < 256; blocksFreed++) {
								read(fileDescriptor, &blockAddr, sizeof(unsigned int));
								add_block_to_free_list(blockAddr, 0);
							}
						}
					}
					//Only if 2 level indirect is used
					if(fileInode.addr[10] != 0) {
						//Now have to do the same for the 2 level indirect block
						lseek(fileDescriptor, fileInode.addr[10], 0);
						int blocksRead;
						unsigned int indirBlockAddr;
						for(blocksRead = 0; blocksRead < 256; blocksRead++) {
							read(fileDescriptor, &indirBlockAddr, sizeof(unsigned int));
							lseek(fileDescriptor, indirBlockAddr, 0);
							//Freeing the blocks pointed to by the indirect block
							int blocksFreed;
							unsigned int blockAddr;
							for(blocksFreed = 0; blocksFreed < 256; blocksFreed++) {
								read(fileDescriptor, &blockAddr, sizeof(unsigned int));
								add_block_to_free_list(blockAddr, 0);
							}
						}
					}
				}
				else {
					//DEBUG
					//printf("not large file\n");
					//Freeing each of the blocks, only if they are used
					int k;
					for(k = 0; k < 11; k++) {
						if(fileInode.addr[k] != 0) {
							add_block_to_free_list(fileInode.addr[k], 0);
						}
					}
				}
				//All the blocks are freed, now free the inode
				char inodeBuffer[sizeof(inode_type)];
				memset(&inodeBuffer, 0, sizeof(inode_type));
				lseek(fileDescriptor, (2*BLOCK_SIZE) + (currentEntry.inode-1)*INODE_SIZE, 0);
				write(fileDescriptor, &inodeBuffer, sizeof(inode_type));
				//Now have to remove the directory entry
				char dirBuffer[sizeof(dir_type)];
				memset(&dirBuffer, 0, sizeof(dir_type));
				lseek(fileDescriptor, currentInode.addr[i] + offset, 0);
				write(fileDescriptor, &dirBuffer, sizeof(dir_type));
				//Done, return out
				return 0;
			}
		}
	}
}
