#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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


/* ==================== MESSAGE ==================== */

typedef struct {
	int key_id;
    int length;
	unsigned long key;
	char* cypher;
} key_data_t;

typedef struct {
	int message_id;
	key_data_t key;
	int proccess_id;
	unsigned long num_tries;
	double time;
} msg_data_t;

typedef struct {
	int message_id;
	key_data_t key;
	unsigned long min_value;
	unsigned long max_value;
} msg_decrypt_t;

/* ==================== TABLES ==================== */

typedef struct {
	int* list_of_keys;
	int* keys_real_time;
	unsigned long num_tries;
	double* key_time;
	int num_procs;
	double exec_time;
} statistics_t;

struct proc_table_row {
	int proc_id;
	statistics_t stats;
};

struct key_table_row {
	int key_id;
	key_data_t key;
	int *procs;
	int num_procs_list;
	int decrypted_flag;
};


typedef struct proc_table_row* proc_table_t;
typedef struct key_table_row* key_table_t;

/* Process tasks functions */
int calculator_proccess(char *argv[], int proccess_id); 
int IO_proccess(char *argv[], int num_procs);

/* Other functions. They could be included in a utils.h */
int construct_key_type(int num_keys, key_data_t* data, MPI_Datatype* MPI_Type);
int construct_decrypt_msg(int num_keys, msg_decrypt_t* data, MPI_Datatype* MPI_Type); 
int construct_data_msg(int num_keys, msg_data_t* data, MPI_Datatype* MPI_Type);
int assign_key_to_proccess(int proc_id, key_table_t k_table, int num_keys, int num_procs);

/* KEY HANDLE FUNCTIONS */
key_data_t key_generator(int id);
char *key_encrypter(unsigned long key); 
int key_decrypter(msg_decrypt_t msg, clock_t* end, int* key_found);

/* TABLE HANDLE FUNCTIONS */
int initialice_table_of_keys(key_table_t *k_table, proc_table_t *p_table, int n_proc, int num_keys); 
int search_keys_not_assigned(key_table_t k_table, int num_procs, key_data_t *key);
int are_there_keys_not_decrypted(key_table_t k_table, int num_keys);
int search_keys_with_min_num_of_procs(key_table_t k_table, int num_keys, int* num_proc, int** procs_calc, key_data_t *key);


