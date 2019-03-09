#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"

#define MIN                           00000000
#define MAX                           100000000

#define N_DECRYPT_MESSAGE_ELEMENTS          5
#define N_KEY_ELEMENTS				        3
#define N_DATA_MESSAGE_ELEMENTS       	    6
#define N_REQUEST_DATA_MESSAGE_ELEMENTS     1
#define N_FINISH_EXECUTION_MESSAGE_ELEMENTS 1

#define IO_PROCESS_ID                 0
#define KEY_LENGTH                    9

#define DECRYPT_MESSAGE     		  1
#define DATA_MESSAGE                  2
#define REQUEST_DATA_MESSAGE 		  3
#define FINISH_EXECUTION_MESSAGE 	  4

/* ======= MESSAGES ======= */

typedef struct {
	int key_id;
	unsigned long key;
	char* cypher;
} key_t;

typedef struct {
	int message_id;
	Key_type key;
	int process_id;
	int length;
	unsigned long num_tries;
	double time;
} msg_data_t;

typedef struct {
	int message_id;
	key_t key;
	unsigned long min_value;
	unsigned long max_value;
} msg_decrypt_t;

/* ======= TABLES ======= */

struct proc_table_row {
	int proc_id;
	statistics_t stats;
}

struct key_table_row {
	int key_id;
	key_t key;
	int *procs;
	int decrypted_flag;
}

typedef struct {
	/* This is for Samuel to fill */
} statistics_t;

typedef struct proc_table_row* proc_table_t;
typedef struct key_table_row* key_table_t;

/* Process tasks functions */
int calculator_process(int process_id);
int IO_process(char *argv[]); 

/* Other functions. They could be included in a utils.h */
int construct_key_type(int num_keys, key_t* data);
int construct_decrypt_msg(int num_keys, msg_decrypt_t* data); 
int construct_data_msg(int num_keys, msg_data_t* data);
void assign_key_to_proccess(int id, key_table_t k_table, int num_keys);

/* KEY HANDLE FUNCTIONS */
key_t key_generator(int id);
char *key_encrypter(unsigned long key); 
int key_decrypter(key_t key);

/* TABLE HANDLE FUNCTIONS */
void initialice_table_of_keys(key_table_t *k_table, proc_table_t *p_table, int num_proc, int num_keys); 
int search_keys_not_assigned(key_table_t k_table, int num_keys);


