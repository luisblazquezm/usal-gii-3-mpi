#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"

#define MIN 00000000
#define MAX 100000000
#define N_STRUCT_ELEMENTS 4
#define IO_PROCESS_ID 0
#define KEY_LENGTH 9

#define DECRYPT_MESSAGE     		  1
#define DATA_MESSAGE                  2
#define REQUEST_DATA_MESSAGE 		  3
#define FINISH_EXECUTION_MESSAGE 	  4

typedef unsigned long Key_number_type;

typedef struct {
	int message_id;
	Key_type key;
	int process_id;
	int length;
	unsigned long num_tries;
	double time;
} Message_data_type;

typedef struct {
	int key_id;
	char* key;
} Key_type;

typedef struct {
	int message_id;
	Key_type key;
	Calculation_range_type Calculation_range;
} Message_decrypt_type;

typedef struct {
	unsigned long min_value;
	unsigned long max_value;
} Calculation_range_type;

typedef struct {
	int message_id;
} Message_request_data_type;

typedef struct {
	int message_id;
} Message_finish_execution_type;

typedef struct {
	int key_id;
	unsigned long key;
	int process[];
	Statistics_type Stadistics;
} Key_table_type;

typedef struct {
	/* This is for Samuel to fill */
} Statistics_type;

/* Process tasks functions */
int calculator_process(int process_id);
int IO_process(char *argv[]); 

/* Other functions. They could be included in a utils.h */
int construct_message_type(int numberOfKeys, Message_type* pdata, MPI_Datatype* pMPI_Message_type);
void key_generator(int numberOfKeys, unsigned long *key);
char *key_encrypter(unsigned long key); 
unsigned long long key_decrypter(char *encrypted_key, Key_number_type *decrypted_key, int key_length, int *key_found);
