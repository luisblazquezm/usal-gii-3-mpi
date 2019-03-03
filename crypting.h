#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"

#define MIN 00000000
#define MAX 100000000
#define N_STRUCT_ELEMENTS 4
#define IO_PROCESS_ID 0
#define KEY_LENGTH 9

#define EXCLUSIVE_DECRYPT_MESSAGE     1
#define NON_EXCLUSIVE_DECRYPT_MESSAGE 2
#define HELP_MESSAGE 				  3
#define KEY_FOUND_MESSAGE 			  4
#define KEY_FOUND_BY_MESSAGE		  5
#define KILL_MESSAGE 				  6


typedef unsigned long Key_type;

typedef struct {
	char* key;
	int length;
	int process_id;
	unsigned long num_tries;
	double time;
} Message_type;

/* Process tasks functions */
int calculator_process(int process_id);
int IO_process(char *argv[]); 

/* Other functions. They could be included in a utils.h */
int construct_message_type(int numberOfKeys, Message_type* pdata, MPI_Datatype* pMPI_Message_type);
void key_generator(int numberOfKeys, unsigned long *key);
char *key_encrypter(unsigned long key); 
unsigned long long key_decrypter(char *encrypted_key, Key_type *decrypted_key, int key_length);
