#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <time.h>

#define BIGINT_MAX 2000000
#define THREAD_STK_SIZE 64 * 4096 * 4096 

struct BigInt num1; // Read-only
struct BigInt num2; // Read-only

struct BigInt multOut; // Writable (mutex)

int current_index;
int max_index;

sem_t output_mutex;
sem_t next_instr_mutex;

struct BigInt {
	int		size;
	int		digits[BIGINT_MAX]; 
};

struct singleAddOutput {
	int result;
	int carry;
};

struct singleAddInput {
	int num1;
	int num2;
	int carry;
};

void loadBigInt(struct BigInt *number, char* filePath);
void addBigInt(struct BigInt *target, struct BigInt *source, int start);
void singleAdd(struct singleAddInput *input, struct singleAddOutput *output);
void printBigInt(struct BigInt *number);

void loadBigInt(struct BigInt *number, char* filePath) {

	FILE *infile = fopen(filePath,"r");

	fscanf(infile,"%d", &number->size);

	getc(infile); // Get rid of the \n character
	for (int i = number->size-1; i >= 0; i--) {
		number->digits[i] = getc(infile) - 48;
	}

	fclose(infile);
}

void addBigInt(struct BigInt *target, struct BigInt *source, int start) {

	int maxSize;
	if (target->size > source->size) { maxSize = target->size; }
	else { maxSize = source->size; }

	struct singleAddInput addInput;
	struct singleAddOutput addOutput;
	addOutput.carry = 0;
	addOutput.result = 0;
	for (int i = start; i <= maxSize; i++) {

		addInput.num1 = source->digits[i];
		addInput.num2 = target->digits[i];
		addInput.carry = addOutput.carry;
		singleAdd(&addInput,&addOutput);
		target->digits[i] = addOutput.result;

	}

	// After all the adding has occurred
	if (addInput.carry == 1) {
		target->size = maxSize+1;
	} else {
		target->size = maxSize;
	}

}

void singleAdd(struct singleAddInput *input, struct singleAddOutput *output) {
	int tempOutput = input->num1 + input->num2 + input->carry;
	if (tempOutput > 9) {
		output->carry = 1;
		output->result = tempOutput - 10;
	} else {
		output->carry = 0;
		output->result = tempOutput;
	}
}

void printBigInt(struct BigInt *number) {
	FILE * fout;
	fout = fopen("c2.txt","w+");
	for (int i = number->size-1; i >= 0; i--) {
		fprintf(fout,"%d",number->digits[i]);
	}
	fclose(fout);
}

void singleMult(struct singleAddInput *input, struct singleAddOutput *output) {
	int tempOutput = ( input->num1 * input->num2 ) + input->carry;
	if (tempOutput > 9) {
		output->result = tempOutput % 10;
		output->carry = (tempOutput - output->result) / 10;
	} else {
		output->carry = 0;
		output->result = tempOutput;
	}
}

void multSegment(int offset) {
	// Multiplies using a single digit from one number onto another number
	
	struct BigInt * tempNumber = (struct BigInt *)malloc(sizeof(struct BigInt));
	
	// Performs multiplication using a single digit from one of the numbers
	struct singleAddInput multInput;
	struct singleAddOutput multOutput;
	multOutput.carry = 0;
	multOutput.result = 0;
	for (int i = 0; i <= num2.size; i++) {
		multInput.num1 = num2.digits[i];
		multInput.num2 = num1.digits[offset];
		multInput.carry = multOutput.carry;
		singleMult(&multInput,&multOutput);
		tempNumber->digits[i+offset] = multOutput.result;
	}

	if (multInput.carry > 0) {
		tempNumber->size = offset+num2.size+1;
	} else {
		tempNumber->size = offset+num2.size;
	}

	sem_wait(&output_mutex);
	addBigInt(&multOut, tempNumber, offset);
	sem_post(&output_mutex);

	if (tempNumber==NULL) {
		printf("%p\n",tempNumber);
	}
	free(tempNumber);
	
}

void *multThread(void *args) {

	while (1) {
		// Getting the next index to be multiplied
		sem_wait(&next_instr_mutex);
		int next_segment_index = current_index;
		current_index++;
		sem_post(&next_instr_mutex);

		if (next_segment_index >= max_index) {
			return NULL;
		}

		// Perform multiplication using one digit
		// and add result to output
		multSegment(next_segment_index);
	}

	return NULL;
}

int main() {

	loadBigInt(&num1,"a.txt");
	loadBigInt(&num2,"b.txt");

	sem_init(&output_mutex,0,1);
	sem_init(&next_instr_mutex,0,1);

	printf("Enter the number of threads: ");
	char *keyboardInput;
	fscanf(stdin,"%s",keyboardInput);
	int numThreads = atoi(keyboardInput);

	pthread_t *threads = (pthread_t*)malloc(sizeof(pthread_t)*numThreads);

	current_index = 0;
	max_index = num1.size;

	// tempNum = (struct BigInt**)malloc(sizeof(struct BigInt*) * max_index);

	for (int thread_no = 0; thread_no < numThreads; thread_no++) {
		if (pthread_create(&threads[thread_no], NULL, multThread, NULL)) {
			printf("Error creating thread\n");
		}
	}

	int * status;

	for (int thread_no = 0; thread_no < numThreads; thread_no++) {
		if (pthread_join(threads[thread_no],(void**)(&status))) {
			printf("Error joining thread\n");
		}
	}

	printBigInt(&multOut);

	return 0;
}
