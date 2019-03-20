#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <crypt.h>
#include "mpi.h"

#define MIN                           0		        /*<<<<<<<<<<<<<<<<<<<<<< Claves de 8 */
#define MAX                           10000		    /*<<<<<<<<<<<<<<<<<<<<<< Claves de 8 */

#define SEED_MULT                     1890

#define N_DECRYPT_MESSAGE_ELEMENTS          4
#define N_KEY_ELEMENTS				        4
#define N_DATA_MESSAGE_ELEMENTS       	    5
#define N_REQUEST_DATA_MESSAGE_ELEMENTS     1
#define N_FINISH_EXECUTION_MESSAGE_ELEMENTS 1

#define IO_PROCESS_ID                     0
#define NULL_PROC_ID                     -1

#define KEY_LENGTH                    	  4			/*<<<<<<<<<<<<<<<<<<<<<< Claves de 8 */
#define CRYPT_LENGTH					  14        /* OMG you fucked here: it's 13 + 1 */

#define DECRYPT_MESSAGE_TAG     		  1
#define DATA_MESSAGE_TAG                  2
#define REQUEST_DATA_MESSAGE_TAG 		  3
#define FINISH_EXECUTION_MESSAGE_TAG 	  4

#define return_value_print(x){\
            switch(x) {\
                case ERROR_1: printf("%s\n", "Error 1"); break;\
                case ERROR_2: printf("%s\n", "Error 2"); break;\
                case ERROR_3: printf("%s\n", "Error 3"); break;\
                case ERROR_4: printf("%s\n", "Error 4"); break;\
                case ERROR_5: printf("%s\n", "Error 5"); break;\
                case KEYS_LEFT: printf("%s\n", "Keys left"); break;\
                case NO_KEYS_LEFT: printf("%s\n", "No keys left"); break;\
            }\
        }


// Functions' return values' macros
#define KEYS_LEFT               -10
#define NO_KEYS_LEFT            -11
#define KEY_FOUND               -10
#define KEY_NOT_FOUND           -11

#define ERROR                   -1
#define SUCCESS                 -128
#define ERROR_1                 -129
#define ERROR_2                 -130
#define ERROR_3                 -131
#define ERROR_4                 -132
#define ERROR_5                 -133

/* Debug */
#define DEFAULT_NUM_KEYS 100


/* ==================== MESSAGE ==================== */

typedef struct {
	int key_id;								/* Id of the key */
    int length;								/* Length of the key */
	unsigned long key_number;				/* Uncrypted key. Only numbers */
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

typedef int msg_request_data_t;
typedef int msg_finish_execution_t;

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
};


typedef struct proc_table_row* proc_table_t;
typedef struct key_table_row* key_table_t;

/* Process tasks functions */
int calculator_proccess(char *argv[], int proccess_id); 
int IO_proccess(char *argv[]);

/* Other functions. They could be included in a utils.h */
int register_key_type(key_data_t* data, MPI_Datatype* MPI_Type);
int register_decrypt_msg(msg_decrypt_t* data, MPI_Datatype* MPI_Type); 
int register_data_msg(msg_data_t* data, MPI_Datatype* MPI_Type);
int assign_key_to_proccess(int proc_id, key_table_t k_table, proc_table_t p_table, int num_keys, int num_procs);
int are_there_keys_not_decrypted(key_table_t k_table, int num_keys);
int search_free_procs(proc_table_t p_table, int num_proc, int* proccess_id);
int search_keys_with_min_num_of_procs(key_table_t k_table, int num_keys, int* num_procs, int* procs_calc, key_data_t *key);
int distribute_work(int num_procs, unsigned long *starting_values);

/* KEY HANDLE FUNCTIONS */
key_data_t key_generator(int id);
char *key_encrypter(unsigned long key); 
int key_decrypter(msg_decrypt_t *msg);

/* TABLE HANDLE FUNCTIONS */
int initialize_tables(key_table_t k_table, proc_table_t p_table, int n_proc, int num_keys);
int associate_proc_to_key(int proc_id, int key_id, key_table_t k_table, proc_table_t p_table);

int fill_data_msg(msg_data_t* data_msg, key_data_t key, int proc_id, int num_tries, clock_t begin, clock_t end); /* NOTE : data_msg.time = (double)(end - begin) / CLOCKS_PER_SEC;*/
int fill_decrypt_msg(msg_decrypt_t *decrypt_msg, key_data_t key , unsigned long min_value, unsigned long max_value);
int store_stats_data(proc_table_t p_table, msg_data_t data_msg, int num_procs);
int update_tables_after_key_found(msg_data_t data_msg, key_table_t k_table, proc_table_t p_table);

/* Debug */
int debug_msg_printf(const char *format, ...);
int print_key_table(key_table_t key_table, int nkeys);
int print_proc_table(proc_table_t proc_table, int nprocs);


