//Programmer: Tyler Heald
//Date: 10/17/19
//CS 5348.001 Project 2
//This is the  single threaded program that
//will multiply two 1 mil digits together
//
//Check that input is valid, then allocate memory space for a 2 mil array to hold A&B
//Read A and B in from their respective files
//Allocate memory space for a 1 tril array to hold C
//Multiply A and B, store in C.
//Write C to an output file.

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

int main(int argc, char* argv[]) {
	//Validate input first
	if(argc != 4) {
		fprintf(stdout, "Program usage: ./a.out a.txt b.txt c.txt\n");
		//exit(1);
	}
	//Input is valid. Initialize variables to hold the length of A and B
	long aLength;
	long bLength;

	//Attempt to open the file for variable A
	FILE* afp;
	if((afp = fopen(argv[1], "r")) == NULL) {
		fprintf(stdout, "Failed to open file %s\n", argv[1]);
		exit(1);
	}
	//Opened the file, read the length of the variable
	fscanf(afp, "%ld", &aLength);

	//Attempting to open file for variable B
	FILE* bfp;
	if((bfp = fopen(argv[2], "r")) == NULL) {
		fprintf(stdout, "Failed to open file %s\n", argv[2]);
		exit(1);
	}
	//Opened file, reading length of variable
	fscanf(bfp, "%ld", &bLength);

	//Allocating memory for an array to hold all the numbers in variables A and B
	//Using chars for space
	char* array = malloc((aLength+bLength)*sizeof(char));
	
	//Reading in A to the start of the array
	//Creating a variable to count down number of digits to insert
	long arrayCount = aLength;
	//Getting the space between the number of digits and the variable itself
	fgetc(afp);
	//char variable to hold the next char for debug stuff
	char nextChar;
	for(;arrayCount > 0; arrayCount--) {
		//Getting the next character, making the int value equal what the char is and storing
		//in the position of bLength + amount left to add from A - 1 for index
		nextChar = fgetc(afp);
		array[arrayCount+bLength - 1] = nextChar-48;
	}
	//Closing the file for A
	fclose(afp);
	//Doing all the same for variable B
	arrayCount = bLength;
	fgetc(bfp);
	for(;arrayCount > 0;arrayCount--) {
		nextChar = fgetc(bfp);
		array[arrayCount-1] = nextChar-48;
	}
	fclose(bfp);
	
	//Creating the array for result C
	//The length of a number is at most the sum of the lengths
	//of the multiplicand and multiplier
	char* outArray = malloc((aLength+bLength)*sizeof(char));

	//Setting the array to be all zero values so it doesn't have any data already in it
	long outArrayCounter = aLength+bLength;
	for(;outArrayCounter > 0; outArrayCounter--){
		outArray[outArrayCounter-1] = 0;
	}
	//Setting the left most digit to 50 value so the program can determine
	//if the arary got fully filled or not
	outArray[aLength+bLength-1] = 50;

	//Performing the multiplication
	//Mulitplying A to B, so multiplying each digit of A by all of B
	long multCount;
	//Counter for inner loop over B
	long multCountInner;
	//Var to store the product to check for carry
	char product;
	//Iterating once for each digit in A
	for(multCount = 0; multCount < aLength; multCount++){
		//DEBUG
		//if(multCount % 1000 == 0) {
		//	fprintf(stdout, "Still kicking! %ld out of %ld!\n", multCount, aLength);
		//}
		//Now iterate for each digit in B
		for(multCountInner = 0; multCountInner < bLength; multCountInner++){
			//Multiply the A[multCount] digit by B[multCountInner] digit
			product = array[multCount+bLength] * array[multCountInner];
			//DEBUG
			//fprintf(stdout, "product of %d & %d is: %d\n", array[multCount+bLength], array[multCountInner], product);
			//Adding the least significant digit of the product to
			//C[multCount+multCountInner]
			outArray[multCount+multCountInner] += product % 10;
			//Checking if a carry needs to be performed
			if(outArray[multCount+multCountInner] > 9) {
				//Performing carry to multCount+multCountInner+1
				outArray[multCount+multCountInner+1] += outArray[multCount+multCountInner] / 10;
				//Removing the tens digit from multCount+multCountInner
				outArray[multCount+multCountInner] = outArray[multCount+multCountInner] % 10;
			}
			//Adding the potential tens digit from product to C[multCount+multCountInner+1]
			outArray[multCount+multCountInner+1] += product/10;
			//The next iteration will take care of any carry needed in C[multCount+multCountInner+1]
		}
	}
	//If outArray[aLength+bLength-1] != 50, then need to remainder by 50
	if(outArray[aLength+bLength-1] > 50) {
		outArray[aLength+bLength-1] = outArray[aLength+bLength-1] % 50;
	}
	//Creating/overwriting the file to output C
	FILE* cfp;
	if((cfp = fopen(argv[3], "w")) == NULL){
		fprintf(stdout, "Unable to open file %s for writing\n", argv[3]);
		exit(1);
	}
	//Writing the result to the file for C
	//Checking if the left most array index was used
	//If not, setting outArrayCounter to not include that
	if(outArray[aLength+bLength-1] > 9){
		outArrayCounter = aLength+bLength-1;
	}
	else{
		outArrayCounter = aLength+bLength;
	}
	//Looping over the used spaces of the array
	for(;outArrayCounter > 0; outArrayCounter--){
		fputc(outArray[outArrayCounter-1]+48, cfp);
	}
	//All operations are done, closing the output file and freeing memory
	close(cfp);
	free(array);
	free(outArray);
}
