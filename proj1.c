//Files used were dogfox, ipsum, file1, proj1.c
//Programmer: Tyler Heald
//Class: CS 5348.001
//There is a makefile, but compilation is gcc proj1.c -lpthread -lrt

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdbool.h>

//DEFINES
#define SEM_NAME "mutexLock"

//GLOBALS
sem_t *mutex;

//FORWARD DECLARATIONS
void processFile(char* fileName, char* outFileName);

int main(int argc, char* argv[]) {
	//Checking args (number, usage)
	//Needs at least four args (./a.out 1 file outfile)
	if(argc < 4) {
		fprintf(stderr, "Proper usage is ./a.out n f1 f2 ... fn outfile\n");
		exit(1);
	}
	//Checking if argv[1] (n) is an integer
	if(atoi(argv[1]) < 1) {
		fprintf(stderr, "Please enter an integer greater than 0 after ./a.out\n");
		exit(1);
	}
	//Comparing argv[1] (n) to number of files provided
	if(atoi(argv[1]) != (argc - 3)) {
		fprintf(stderr, "The number of files to be opened does not match the number of files provided\n");
		exit(1);
	}
	//Checking all files exist
	int fileCount;
	bool foundAll = true;
	for(fileCount = 2; fileCount < argc - 1; fileCount ++) {
		if(access(argv[fileCount], F_OK) < 0) {
			fprintf(stderr, "Failed to find file %s\n", argv[fileCount]);
			foundAll = false;
		}
	}
	if(!foundAll) { 
		fprintf(stderr, "Couldn't find all files, exited program\n");
		exit(1);
	}

	//Creating a semaphore for all of the processes to use
	mutex = sem_open(SEM_NAME, O_CREAT, 0644, 1);

	//Opening the output file to write some basic info
	//ENTERING CRITICAL SECTION
	sem_wait(mutex);
	FILE *fp;
	if((fp = fopen(argv[argc-1], "wb")) == NULL) {
		fprintf(stderr, "Failed to open the output file in parent!\n");
		//Cleaning up semaphore before exiting
		sem_unlink(SEM_NAME);
		exit(1);
	}

	//Printing header to the output file
	fprintf(fp, "Tyler Heald - CS 5348.001 Project 1\n");
	fclose(fp);
	sem_post(mutex);
	//EXITED CRITICAL SECTION
	
	//Creating a process for each file
	int procCount;
	int pid;
	for(procCount = 0; procCount < (argc-3); procCount ++) {
		if((pid = fork()) == 0) {
			//Child, needs to process file and then kill itself
			processFile(argv[procCount+2], argv[argc-1]);
		}
	}
	//Parent, waits till all children are done
	int status;
	while(wait(&status) > 0);

	//All are done, clean up semaphore, notify user
	sem_unlink(SEM_NAME);
	fprintf(stdout, "All processes are finished\n");
	exit(0);
}

void processFile(char* fileName, char* outFileName) {
	//trying to open the file first, then setting everything else up if that's okay
	FILE *inFile;
	if((inFile = fopen(fileName, "r")) == NULL) {
		fprintf(stderr, "Couldn't open the file %s\n", fileName);
		exit(1);
	}
	int count[26] = {0};

	//Reading the file, counting each char, capitals are counted the same as non capitals
	char nextChar;
	while((nextChar = fgetc(inFile)) != EOF) {
		//Checking for capitals or non-capitals to easily gorup them
		if(nextChar <= 'Z' && nextChar >= 'A') {
			count[nextChar - 'A'] = count[nextChar - 'A'] + 1;
		}
		else if(nextChar <= 'z' && nextChar >= 'a') {
			count[nextChar -'a'] = count[nextChar - 'a'] + 1;
		}
	}

	//Writing the results to the output file
	//have to open semaphore first
	//ENTERING CRITICAL SECTION
	sem_wait(mutex);

	FILE *outFile;
	if((outFile = fopen(outFileName, "ab")) == NULL) {
		fprintf(stderr, "Failed to open file %s\n", outFileName);
		exit(1);
	}

	//Printing header
	fprintf(outFile, "\nCharacter statistics for file: %s\n", fileName);

	int charCount;
	for(charCount = 'a'; charCount <= 'z'; charCount++) {
		fprintf(outFile, "%c: %d\n", charCount, count[charCount - 'a']);
	}
	fclose(outFile);
	sem_post(mutex);
	//EXITED CRITICAL SECTION

	//Process is done, exit
	exit(0);
}
