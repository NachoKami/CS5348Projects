//Filename: fs_initialization.c

//Team Members:Sean Kennedy and Tyler Heald

//UTD_ID: 2021388327 and 2021360768

//NetID: smk170630 and tch170130

//Class: CS 5348.001

//Project: Project 3

// Functions related to the initialization of the filesystem

#include "fsaccess.h"

int preInitialization(){

  char *n1, *n2;
  unsigned int numBlocks = 0, numInodes = 0;
  char *filepath;
  
	// takes 3 arguments
  filepath = strtok(NULL, " ");
  n2 = strtok(NULL, " ");
  n1 = strtok(NULL, " ");
         
      
  if(access(filepath, F_OK) != -1) {
      // Basic error handling
      if( (fileDescriptor = open(filepath, O_RDWR|O_NONBLOCK, 0600)) == -1){
      
         printf("\n filesystem already exists but open() failed with error [%s]\n", strerror(errno));
         return 1;
      }
      printf("filesystem already exists and the same will be used.\n");
			return 0;
  
  } else {
  
        	if (!n1 || !n2) {
							// Basic error handling
              printf(" All arguments(path, number of inodes and total number of blocks) have not been entered\n");
							return 1;
					} else {
          		numBlocks = atoi(n1); // Doesn't handle potential errors (random letters)
          		numInodes = atoi(n2); // Doesn't handle potential errors (random letters)
          		
							// Finally initialize the filesystem
          		if( initfs(filepath,numBlocks, numInodes )){
          		  printf("The file system is initialized\n");	
								return 0;
          		} else {
            		printf("Error initializing file system. Exiting... \n");
            		return 1;
          		}
       		}
  }
}

int initfs(char* path, unsigned short blocks,unsigned short inodes) {

   unsigned int buffer[BLOCK_SIZE/4];
   int bytes_written; // This is unused
   
   unsigned short i = 0;
   superBlock.fsize = blocks;
   unsigned short inodes_per_block= BLOCK_SIZE/INODE_SIZE;
   
   if((inodes%inodes_per_block) == 0)
   superBlock.isize = inodes/inodes_per_block;
   else
   superBlock.isize = (inodes/inodes_per_block) + 1;
   
   if((fileDescriptor = open(path,O_RDWR|O_CREAT|O_NONBLOCK,0700))== -1)
       {
           printf("\n open() failed with the following error [%s]\n",strerror(errno));
           return 0;
       }
       
   for (i = 0; i < blocks; i++)
      superBlock.free[i] =  i;			//initializing free array to 0 to remove junk data. free array will be stored with data block numbers shortly.
    
   superBlock.nfree = blocks;
   superBlock.ninode = inodes;
   
   for (i = 0; i < inodes; i++)
	    superBlock.inode[i] = i + 1;		//Initializing the inode array to inode numbers
   
   superBlock.flock = 'a'; 					//flock,ilock and fmode are not used.
   superBlock.ilock = 'b';					
   superBlock.fmod = 0;
   superBlock.time[0] = 0;
   superBlock.time[1] = 1970; // Seems like UNIX time
   
   lseek(fileDescriptor, BLOCK_SIZE, SEEK_SET);
   write(fileDescriptor, &superBlock, BLOCK_SIZE); // writing superblock to file system
   
   // writing zeroes to all inodes in ilist
	 // Good thing we check if the filesystem
	 // already exists. All the data would be
	 // gone otherwise.
   for (i = 0; i < BLOCK_SIZE/4; i++) 
   	  buffer[i] = 0;
        
   for (i = 0; i < superBlock.isize; i++)
   	  write(fileDescriptor, buffer, BLOCK_SIZE);
   
   int data_blocks = blocks - 2 - superBlock.isize;
   int data_blocks_for_free_list = data_blocks - 1;
   
   // Create root directory
   create_root();
 
	 // // Skip past the superblock and iNodes to free blocks
   // for ( i = 2 + superBlock.isize + 1; i < data_blocks_for_free_list; i++ ) {
   //    add_block_to_free_list(i , buffer);
   // }
   
   return 1;
}

// Add Data blocks to free list
// Specifically, free block #block_number
void add_block_to_free_list(int block_number,  unsigned int *empty_buffer){

  if ( superBlock.nfree == FREE_SIZE ) {

    int free_list_data[BLOCK_SIZE / 4], i;
    free_list_data[0] = FREE_SIZE;
    
    for ( i = 0; i < BLOCK_SIZE / 4; i++ ) {
       if ( i < FREE_SIZE ) {
         free_list_data[i + 1] = superBlock.free[i];
       } else {
         free_list_data[i + 1] = 0; // getting rid of junk data in the remaining unused bytes of header block
       }
    }
    
		// Adds the new free block here
    lseek( fileDescriptor, (block_number) * BLOCK_SIZE, 0 );
    write( fileDescriptor, free_list_data, BLOCK_SIZE ); // Writing free list to header block
    
    superBlock.nfree = 0;
    
  } else {

	  lseek( fileDescriptor, (block_number) * BLOCK_SIZE, 0 );
    write( fileDescriptor, empty_buffer, BLOCK_SIZE );  // writing 0 to remaining data blocks to get rid of junk data
  }

  superBlock.free[superBlock.nfree] = block_number;  // Assigning blocks to free array
  ++superBlock.nfree;
}

// Create root directory
void create_root() {

  int root_data_block = (2 + superBlock.isize) * BLOCK_SIZE; // Allocating first data block to root directory
  int i;
  
  root.inode = 1;   // root directory's inode number is 1.
  root.filename[0] = '.';
  root.filename[1] = '\0';
  
  inode.flags = inode_alloc_flag | dir_flag | dir_large_file | dir_access_rights;   		// flag for root directory 
  inode.nlinks = 0; // Initially the directory is empty
  inode.uid = 0;
  inode.gid = 0;
  inode.size = INODE_SIZE;
  inode.addr[0] = root_data_block;
  
  for( i = 1; i < ADDR_SIZE; i++ ) {
    inode.addr[i] = 0;
  }
  
  inode.actime[0] = 0;
  inode.modtime[0] = 0;
  inode.modtime[1] = 0;
  
	// Puts the first iNode right after the superblock
  lseek(fileDescriptor, 2 * BLOCK_SIZE, 0);
  write(fileDescriptor, &inode, INODE_SIZE);   // 
  
	// Puts the first data block right after the end
	// of the inodes (superBlock.isize + 2)
  lseek(fileDescriptor, root_data_block, 0);
  write(fileDescriptor, &root, 16);
  
  root.filename[0] = '.';
  root.filename[1] = '.';
  root.filename[2] = '\0';
  
  write(fileDescriptor, &root, 16);
	char nullBuffer[16] = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
	write(fileDescriptor, nullBuffer, 16); // 'null terminated' directory
 
}
