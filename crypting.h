#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <crypt.h>
#include "mpi.h"

#define MIN                           00000000
#define MAX                           100000000

#define N_DECRYPT_MESSAGE_ELEMENTS          5
#define N_KEY_ELEMENTS				        3
#define N_DATA_MESSAGE_ELEMENTS       	    6
#define N_REQUEST_DATA_MESSAGE_ELEMENTS     1
#define N_FINISH_EXECUTION_MESSAGE_ELEMENTS 1

#define IO_PROCESS_ID                     0
#define KEY_LENGTH                    	  9
#define CRYPT_LENGTH					 13

#define DECRYPT_MESSAGE_TAG     		  1
#define DATA_MESSAGE_TAG                  2
#define REQUEST_DATA_MESSAGE_TAG 		  3
#define FINISH_EXECUTION_MESSAGE_TAG 	  4 


/* ==================== MESSAGE ==================== */

typedef struct {
	int key_id;								/* Id of the key */
    int length;								/* Length of the key */
	unsigned long key;						/* Uncrypted key. Only numbers */
	char cypher[CRYPT_LENGTH];				/* Cyphered key . Characters + Numbers */
} key_data_t;

typedef struct {
	int message_id;							/* Id of the message */
	key_data_t key;							/* Key object (status= DECRYPTED) */
	int proccess_id;						/* Id of the proccess that has found it */
	unsigned long num_tries;				/* Number of tries the proccess has needeed to found the key */
	double time;							/* Number of seconds the proccess has needeed to found the key */
} msg_data_t;

typedef struct {
	int message_id;							/* Id of the message */
	key_data_t key;							/* Key object (status= ENCRYPTED/CYPHERED)  */
	unsigned long min_value;				/* Min. limit to find a key */
	unsigned long max_value;				/* Max. limit to find a key */
} msg_decrypt_t;

/* ==================== TABLES ==================== */

typedef struct {
	int n_keys; 							/* Number of keys the proccess has found */
	unsigned long n_rand_crypt_calls;		/* Number of calls a proccess has done to rand and crypt */ /* NOTE: the number of accumulated tries comes as a result of adding the calls of all the procceses all together*/
	double* key_proccesing_times;			/* List of the time (in seconds) a key has taken to be found by the proccess. One for each key */ /* NOTE: the number of accumulated tries comes as a result of adding the times of all the procceses all together*/
} proc_statistics_t;

struct proc_table_row {
	int proc_id;							/* Id of the proccess */
	int occupied_flag;						/* Indicates if the proccess has a key or not */
	proc_statistics_t stats;				/* Statistics data of every proccess */
};

struct key_table_row {
	int key_id;								/* Id of the key*/
	key_data_t key;							/* Key object */
	int *procs;								/* List of proccesses working on the key */
	int num_procs_list;						/* Number of proccesses working on the key */
	int decrypted_flag;						/* Indicates if the key has already been found or not */
	int assigned_flag;
};


typedef struct proc_table_row* proc_table_t;
typedef struct key_table_row* key_table_t;

/* Process tasks functions */
int calculator_proccess(char *argv[], int proccess_id); 
int IO_proccess(char *argv[]);

/* Other functions. They could be included in a utils.h */
int construct_key_type(int num_keys, key_data_t* data, MPI_Datatype* MPI_Type);
int construct_decrypt_msg(int num_keys, msg_decrypt_t* data, MPI_Datatype* MPI_Type); 
int construct_data_msg(int num_keys, msg_data_t* data, MPI_Datatype* MPI_Type);
int assign_key_to_proccess(int proc_id, key_table_t k_table[], int num_keys, int num_procs);
int are_there_keys_not_decrypted(key_table_t k_table[], int num_keys);
int search_free_procs(proc_table_t p_table[], int num_proc, int* proc_id);
int search_keys_with_min_num_of_procs(key_table_t k_table[], int num_keys, int* num_proc, int** procs_calc, key_data_t *key);

/* KEY HANDLE FUNCTIONS */
key_data_t key_generator(int id);
char *key_encrypter(unsigned long key); 
int key_decrypter(msg_decrypt_t msg, clock_t* end, int* key_found);

/* TABLE HANDLE FUNCTIONS */
int initialice_table_of_keys(key_table_t k_table[], proc_table_t *p_table, int n_proc, int num_keys); 
int register_proccess_key_table(int proc_id, int key_id, key_table_t k_table[]);


