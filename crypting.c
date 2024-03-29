/*************************************************
 *  Source: crypting.c                           *
 *                                               *
 *  @author: Luis Blázquez Miñambres             *
 *           Miguel Cabezas Puerto               *
 *           Samuel Gómez Sánchez                *
 *           Alberto Hernández Pintor            *
 *                                               *
 *  @version: 12.3                               *
 *                                               *
 *  @date: 23/02/2019                            *
 *                                               *
 *  @brief: Programa de simulación de gestión de *
 *          procesos que desencriptan claves     *
 *          para probar el rendimiento de la CPU *
 *          en distintas maquinas                *
 *************************************************/

#include "crypting.h"

/*
 * =================== HOW TO RUN THE PROGRAM ===================
 *
 *  To Compile it : mpicc crypting.c crypting.h stats.c stats.h -lcrypt -o crypt
 *  To execute it : mpirun -np [numProcesos] crypt [arguments]
 */
 
 /* MPI_Datatypes */
MPI_Datatype MPI_KEY_T;
MPI_Datatype MPI_DECRYPT_MSG_T;
MPI_Datatype MPI_DATA_MSG_T;
 

int main(int argc, char *argv[]) 
{
	MPI_Init(&argc, &argv);

		int id = -1;
		key_data_t key_data;
		msg_decrypt_t decrypt_msg;
		msg_data_t data_msg;
		
	    /* ======================  CREATING TYPES OF MESSAGES ====================== */
        
        if (ERROR == register_key_type(&key_data, &MPI_KEY_T)) {
		    fprintf(stderr, "%s\n", "main: register_key_type: ERROR");
		    return -1;
	    }
	    
	    if (ERROR == register_decrypt_msg(&decrypt_msg, &MPI_DECRYPT_MSG_T)) {
		    fprintf(stderr, "%s\n", "main: register_decrypt_msg: ERROR");
		    return -1;
	    }
	    
	    if (ERROR == register_data_msg(&data_msg, &MPI_DATA_MSG_T)) {
		    fprintf(stderr, "%s\n", "main: register_data_msg: ERROR");
		    return -1;
	    }
	    
	    /* ======================  EXECUTION ====================== */

		if (MPI_SUCCESS != MPI_Comm_rank(MPI_COMM_WORLD, &id)) {
			fprintf(stderr, "%s\n", "main: ERROR in MPI_Comm_rank");
		}

		if (id == 0) {
			if (IO_proccess(argc, argv) < 0) {
				fprintf(stderr, "%s\n", "main: ERROR in IO_proccess");
				return -1;
			}
		} else {
		    if (-1 == calculator_proccess(argc, argv, id)) {
			    fprintf(stderr, "%s\n", "main: ERROR in calculator_proccess");
			    return -1;
		    }
		}
		
	MPI_Finalize();

	return 0;
}

/***************************************************
 *  ============= calculator_proccess ============ * 
/***************************************************
/***************************************************
 *  1) Wait for the key to arrive from IO proccess *
 *  2) Try to decrypt it                           *
 *     2.1) Found it --> notifies the rest of      *
 *			proccesses                             *
 *     2.2) Not Found it --> keeps trying :)       *
 *  3) Receives a new key                          *
 ***************************************************/
int calculator_proccess(int argc, char *argv[], int proccess_id) 
{

    int num_keys = 0;

	/* Flags */
    int decrypt_flag = 0;
    int key_available = 0;
    int flag_probe = 0;
    int finish_execution = 0;

    /* Messages */
	msg_decrypt_t decrypt_msg;
	msg_data_t data_msg;
	msg_request_data_t request_data_msg;
	msg_finish_execution_t finish_execution_msg;

	key_data_t dec_key; 				// Decrypted key
	unsigned long long num_tries = 0; 	// Number of tries
	clock_t begin, end;					// Time to decrypt a key

	/* MPI additional parameters for Send an Recv */
	MPI_Status status;
	MPI_Request request;

	/* ======================  CHECKING PARAMETERS ====================== */

	if(argc < 2){
		num_keys = DEFAULT_NUM_KEYS;
	} else if (2 == argc) {
		num_keys = atoi(argv[1]);
	} else {
	    printf("%s", "More arguments than expected\n"); // DEBUG
	}

	if (-1 == proccess_id) {
		fprintf(stderr, "%s\n", "calculator_proccess: procces_id is -1");
		return ERROR;
	}
	
	// Seed for rand numbers
	srand(time(NULL));
	
	/* ======================  EXECUTION ====================== */
	
	do { // While runnning
	
		if (MPI_SUCCESS != MPI_Probe(IO_PROCESS_ID, MPI_ANY_TAG, MPI_COMM_WORLD, &status)) {
			fprintf(stderr, "%s\n", "calculator_proccess: ERROR in MPI_Iprobe");
			return ERROR;
		}
		
	    switch(status.MPI_TAG) {

			case DECRYPT_MESSAGE_TAG:
			
			    if (MPI_SUCCESS != MPI_Irecv(&decrypt_msg, 1, MPI_DECRYPT_MSG_T, IO_PROCESS_ID, DECRYPT_MESSAGE_TAG, MPI_COMM_WORLD, &request) ) {
					fprintf(stderr, "%s\n", "calculator_proccess: ERROR in MPI_IRecv");
					return ERROR;
				}
				
				key_available = 1;
			
			break;
			
			case FINISH_EXECUTION_MESSAGE_TAG: // Message not received, but gonna end
				finish_execution = 1;
			break;
		}
		
		// End if we must end, of course
		if (finish_execution)
		    break;

		/* Beggining search of key */
		begin = clock();

		do { // While there is a key to decrypt
		
		    decrypt_flag = key_decrypter(&decrypt_msg);
		    ++num_tries;
		    
		    if (KEY_FOUND == decrypt_flag) {
		    
		        end = clock();
		    
		        if (ERROR == fill_data_msg(&data_msg, decrypt_msg.key, proccess_id, num_tries, begin, end, 1) ) {
					fprintf(stderr, "%s\n", "calculator_proccess: ERROR in fill_data_msg (1)");
					return ERROR;
				}

				if (MPI_SUCCESS != MPI_Send(&data_msg , 1, MPI_DATA_MSG_T, IO_PROCESS_ID, DATA_MESSAGE_TAG, MPI_COMM_WORLD) ) {
					fprintf(stderr, "%s\n", "calculator_proccess: ERROR in MPI_Send (1)");
					return ERROR;
				}
                
                key_available = 0;
				break; // Key not available anymore
		    }
		    
		    if (MPI_SUCCESS != MPI_Iprobe(IO_PROCESS_ID, MPI_ANY_TAG, MPI_COMM_WORLD, &flag_probe, &status)) {
				fprintf(stderr, "%s\n", "calculator_proccess: ERROR in MPI_Iprobe");
				return ERROR;
			}

			if(0 != flag_probe){
			    switch(status.MPI_TAG) {

					case DECRYPT_MESSAGE_TAG:
					
					    if (MPI_SUCCESS != MPI_Irecv(&decrypt_msg , 1, MPI_DECRYPT_MSG_T, IO_PROCESS_ID, DECRYPT_MESSAGE_TAG, MPI_COMM_WORLD, &request) ) {
							fprintf(stderr, "%s\n", "calculator_proccess: ERROR in MPI_IRecv (1)");
							return ERROR;
						}
					
					break;

					case REQUEST_DATA_MESSAGE_TAG:
					
					    end = clock();
					    
					    if (MPI_SUCCESS != MPI_Irecv(&request_data_msg, 1, MPI_INT, IO_PROCESS_ID, REQUEST_DATA_MESSAGE_TAG, MPI_COMM_WORLD, &request) ) {
							fprintf(stderr, "%s\n", "calculator_proccess: ERROR in MPI_IRecv (2)");
							return ERROR;
						}
					
					    if (ERROR == fill_data_msg(&data_msg, decrypt_msg.key, proccess_id, num_tries, begin, end, 0) ) {
							fprintf(stderr, "%s\n", "calculator_proccess: ERROR in fill_data_msg (2)");
							return ERROR;
						}

						if (MPI_SUCCESS != MPI_Send(&data_msg , 1, MPI_DATA_MSG_T, IO_PROCESS_ID, DATA_MESSAGE_TAG, MPI_COMM_WORLD) ) {
							fprintf(stderr, "%s\n", "calculator_proccess: ERROR in MPI_Send (2)");
							return ERROR;
						}
						
						// We must now wait for indications
						key_available = 0;
					
					break;

					case FINISH_EXECUTION_MESSAGE_TAG: // Message not received, but gonna end
						finish_execution = 1;
					break;
				}
				
				if (finish_execution)
				    break;
			}
		    
		} while(key_available); // While there is a key to decrypt

	} while(!finish_execution);

    return 0;
}

/**************************************************
 *  ================ IO_proccess ================  * 
/**************************************************
 *  1) Generate random keys                       *
 *  2) Encrypt the keys                           *
 *  3) registers the type of MPI for structs     *
 *  3) Send the keys to calculator proccesses     *
 **************************************************/
int IO_proccess(int argc, char *argv[]) 
{ 
	int num_keys = 0;
	int i = 0;
	int num_procs = 0;
	int proc_id = -1;
	int k_id = -1;
	int key_left_id = -1;
	int *tmp_procs = NULL;		 // List of stored proccesses in a key 
	int tmp_num_procs = -1;

	key_data_t key_to_assign;
	key_to_assign.key_id = -1;

	/* Tables */
	key_table_t k_table; 		 // Table of keys 
	proc_table_t p_table;        // Table of proccesses 

	/* Messages */
	msg_decrypt_t decrypt_msg;
	msg_data_t data_msg;
	msg_finish_execution_t finish_execution_msg = FINISH_EXECUTION_MESSAGE_TAG;
	msg_request_data_t request_data_msg = REQUEST_DATA_MESSAGE_TAG;

	/* Flags*/
	int msg_received_flag = -1;
	int free_procs_flag = -1;
	int flag_probe = -1;
	int recv_msg_flag = -1;
	int free_keys_flag = -1;

	/* Data message reception */
	int key_id_rcv = -1, proc_id_rcv = -1;

	/* MPI additional parameters for Send an Recv */
	MPI_Status status;
	MPI_Request request;

	/* ======================  CHECKING PARAMETERS ====================== */
	
    if(argc < 2){
		num_keys = DEFAULT_NUM_KEYS;
	} else if (2 == argc) {
		num_keys = atoi(argv[1]);
	} else {
	    printf("%s", "More arguments than expected\n"); // DEBUG
	}

	if (MPI_SUCCESS != MPI_Comm_size(MPI_COMM_WORLD, &num_procs)) {
		fprintf(stderr, "%s\n", "IO_proccess: ERROR in MPI_Comm_size");
		return ERROR;
	}

    // Initialize seed
    srand(time(NULL));

	/* ======================  CREATE TABLE OF KEYS AND PROCCESSES ====================== */

	if (NULL == (k_table = (struct key_table_row *) malloc(num_keys * sizeof(struct key_table_row)))){ 
		fprintf(stderr, "%s\n", "IO_proccess: ERROR in malloc(k_table)");
		return ERROR;
	}

	if (NULL == (p_table = (struct proc_table_row *) malloc(num_procs * sizeof(struct proc_table_row)))){ 
		fprintf(stderr, "%s\n", "IO_proccess: ERROR in malloc(p_table)");
		return ERROR;
	}

	if (-1 == initialize_tables(k_table, p_table, num_procs, num_keys)) { 
		fprintf(stderr, "%s\n", "IO_proccess: ERROR in initialize_tables");
		return ERROR;
	}
	
	/* ============================================  THERE ARE KEYS LEFT TO GIVE TO THE PROCCESSES ============================================ */

	free_keys_flag = search_free_keys(k_table, num_keys, &k_id);
	if (ERROR_1 == free_keys_flag) {
	    fprintf(stderr, "%s\n", "IO_proccess: ERROR in search_free_keys (1)");
		return ERROR_1;
	}
	while (KEYS_LEFT == free_keys_flag) {
	    
        recv_msg_flag = 0;
	
	    /* Find free proccesses and assign it to them */
	    
	    free_procs_flag = search_free_procs(p_table, num_procs, &proc_id);
	    
		if (-1 == free_procs_flag) { // Error
			fprintf(stderr, "%s\n", "IO_proccess: ERROR in search_free_procs (1)");
			return ERROR;
		} else if (0 == free_procs_flag) {  // All processes are busy
		    recv_msg_flag = 1;              // Then wait for messages, see below
		} else { // There are some free processes
		    
		    /* Assign a key to a proccess */
		    if (-1 == assign_key_to_proccess(proc_id, k_table, p_table, num_keys, num_procs)) {
			    fprintf(stderr, "%s\n", "IO_proccess: ERROR in assign_key_to_proccess (1)");
			    return ERROR;
		    }
		    
		    free_keys_flag = search_free_keys(k_table, num_keys, &k_id);
	        if (ERROR_1 == free_keys_flag) {
	            fprintf(stderr, "%s\n", "IO_proccess: ERROR in search_free_keys (2)");
		        return ERROR;
	        } else if (NO_KEYS_LEFT == free_keys_flag){ // No keys without a proccess asociated
		        break;
	        }
	        
	        /* Check if data has been sent and process it if that is the case */
		    if (MPI_SUCCESS != MPI_Iprobe(MPI_ANY_SOURCE, DATA_MESSAGE_TAG, MPI_COMM_WORLD, &flag_probe, &status)) {
			    fprintf(stderr, "%s\n", "IO_proccess: ERROR in MPI_Iprobe (1)");
			    return ERROR;
		    }
		    
		    if (flag_probe){ // If there are some messages, we should get them!
		        recv_msg_flag = 1;
		    }
		}
		
		if (recv_msg_flag) { // If we must receive some message, do, obviously
		    if (MPI_SUCCESS != MPI_Recv(&data_msg, 1, MPI_DATA_MSG_T, MPI_ANY_SOURCE, DATA_MESSAGE_TAG, MPI_COMM_WORLD, &status) ) {
		        fprintf(stderr, "%s\n", "IO_process: ERROR in MPI_Recv (1)");
		        return ERROR;
	        }
	        	        
	        // And now process the data
		    if (-1 == store_stats_data(p_table, data_msg, num_procs) ){
			    fprintf(stderr, "%s\n", "IO_proccess: ERROR in store_stats_data (1)");
			    return ERROR;
		    }

		    if (-1 == update_tables_after_key_found(data_msg, k_table, p_table)) {
			    fprintf(stderr, "%s\n", "IO_proccess: ERROR in update_tables_after_key_found (1)");
			    return ERROR;
		    }
		    
		    /* Assign a key to a proccess */
		    if (SUCCESS != assign_key_to_proccess(data_msg.proccess_id, k_table, p_table, num_keys, num_procs)) {
			    fprintf(stderr, "%s\n", "IO_proccess: ERROR in assign_key_to_proccess (2)");
			    return ERROR;
		    }
		    
		    free_keys_flag = search_free_keys(k_table, num_keys, &k_id);
	        if (ERROR_1 == free_keys_flag) {
	            fprintf(stderr, "%s\n", "IO_proccess: ERROR in search_free_keys (2)");
		        return ERROR;
	        } else if (NO_KEYS_LEFT == free_keys_flag){ // No keys without a proccess asociated
		        break;
	        }
	    }
	}

	/* ============================================  THERE ARE NOT KEYS LEFT. DISTRIBUTE THE WORK BETWEEN THEM ============================================ */

    k_id = are_there_keys_not_decrypted(k_table, num_keys); // It returns the id of keys not decrypted
    if (ERROR == k_id) {
       fprintf(stderr, "%s\n", "IO_process: are_there_keys_not_decrypted (1)");
	   return ERROR;
    }
    
    while (k_id >= 0) { // There are undecrypted keys
    
        // Check if there are any messages. Block otherwise
        if (MPI_SUCCESS != MPI_Iprobe(MPI_ANY_SOURCE, DATA_MESSAGE_TAG, MPI_COMM_WORLD, &flag_probe, &status)) {
            fprintf(stderr, "%s\n", "IO_process: ERROR in MPI_Iprobe (2)");
            return ERROR;
        }
        
        if (!flag_probe) { // There aren't any messages, thus all processes are busy
	        // Wait for a message
	        if (MPI_SUCCESS != MPI_Probe(MPI_ANY_SOURCE, DATA_MESSAGE_TAG, MPI_COMM_WORLD, &status) ) {
	            fprintf(stderr, "%s\n", "IO_process: ERROR in MPI_Probe");
	            return ERROR;
            }
	    } else { // There are some messages, that is, free processes
	        
	        if (MPI_SUCCESS != MPI_Irecv(&data_msg, 1, MPI_DATA_MSG_T, MPI_ANY_SOURCE, DATA_MESSAGE_TAG, MPI_COMM_WORLD, &request) ) {
			    fprintf(stderr, "%s\n", "calculator_proccess: ERROR in MPI_IRecv (2)");
			    return ERROR;
		    }
		    
		    // Store temporal data
		    key_id_rcv = data_msg.key.key_id;
            proc_id_rcv = data_msg.proccess_id;
            tmp_num_procs = k_table[key_id_rcv].num_procs_list;
		   
		    if (NULL == (tmp_procs = malloc(tmp_num_procs * sizeof(int)))) {
		        fprintf(stderr, "%s\n", "IO process: ERROR in malloc(tmp_procs)");
			    return ERROR;
		    }
		    
		    for (i = 0; i < tmp_num_procs; ++i) {
		        tmp_procs[i] = k_table[key_id_rcv].procs[i];
		    }
		    		    
		    // And update tables and etc
	        if (-1 == store_stats_data(p_table, data_msg, num_procs) ){
		        fprintf(stderr, "%s\n", "IO_proccess: ERROR in store_stats_data (2)");
		        return ERROR;
	        }

	        if (-1 == update_tables_after_key_found(data_msg, k_table, p_table)) {
		        fprintf(stderr, "%s\n", "IO_proccess: ERROR in update_tables_after_key_found (2)");
		        return ERROR;
	        }
	        
	        // Warn processes that they must stop
            for (i = 0; i < tmp_num_procs; i++) {
            
                if (tmp_procs[i] == proc_id_rcv)
                    continue;

	            if (MPI_SUCCESS != MPI_Isend(&request_data_msg , 1, MPI_INT, tmp_procs[i], REQUEST_DATA_MESSAGE_TAG, MPI_COMM_WORLD, &request) ) {
		            fprintf(stderr, "%s\n", "IO_proccess: ERROR in MPI_ISend (1)");
		            return ERROR;
	            }
            }
            
            for (i = 0; i < tmp_num_procs; i++) {
            
                if (tmp_procs[i] == proc_id_rcv)
                    continue;

	            if (MPI_SUCCESS != MPI_Recv(&data_msg , 1, MPI_DATA_MSG_T, tmp_procs[i], DATA_MESSAGE_TAG, MPI_COMM_WORLD, &status) ) {
				    fprintf(stderr, "%s\n", "assign_key_to_proccess: ERROR in MPI_Recv (3)");
				    return ERROR_4;
			    }
			    			    
	            if (-1 == store_stats_data(p_table, data_msg, num_procs) ){
		            fprintf(stderr, "%s\n", "IO_proccess: ERROR in store_stats_data (3)");
		            return ERROR;
	            }
                
                
	            if (-1 == update_tables_after_key_found(data_msg, k_table, p_table)) {
		            fprintf(stderr, "%s\n", "IO_proccess: ERROR in update_tables_after_key_found (3)");
		            return ERROR;
	            }
            }
            
            k_id = are_there_keys_not_decrypted(k_table, num_keys); // It returns the id of keys not decryptede
            if (ERROR == k_id) {
               fprintf(stderr, "%s\n", "IO_process: are_there_keys_not_decrypted (2)");
	           return ERROR;
            }
            
            if (k_id >= 0) { // There are undecrypted keys

			    for (i = 0; i < tmp_num_procs; i++) {

				    proc_id = tmp_procs[i];

				    /* Assign a key to a proccess */
				    if (-1 == assign_key_to_proccess(proc_id, k_table, p_table, num_keys, num_procs)) {
					    fprintf(stderr, "%s\n", "IO_proccess: ERROR in assign_key_to_proccess (3)");
					    return ERROR;
				    }
			    }
		        
            } else { // No keys left
                break;
            }
	    }
    }
    
    // Kill message 
    for (i = 1; i < num_procs; ++i){
        if (MPI_SUCCESS != MPI_Isend(&finish_execution_msg, 1, MPI_INT, i, FINISH_EXECUTION_MESSAGE_TAG, MPI_COMM_WORLD, &request) ) {
			fprintf(stderr, "%s\n", "assign_key_to_proccess: ERROR in MPI_ISend (2)");
			return ERROR;
		}
    }

    printf("\n\n%s\n\n", "PROCESOS FINALIZADOS. Pulse ENTER para continuar: (Vaya pulsando enter para ir pasando por cada tabla)");
    process_raw_data_and_print(k_table, num_keys, p_table, num_procs - 1); // -1 because we don't show stats for IO process

    return 0;
	    
}


/***************************************
 *  initialize_tables                  * 
 ***************************************
 *                                     *
 *  initialize both the keys´          *
 *  and procces´ table                 *
 *                                     *
 *  Return: in case of error -1        *
 *			Otherwise, returns 1       *
 ***************************************/
int initialize_tables(key_table_t k_table, proc_table_t p_table, int n_proc, int num_keys) 
{
	int i, j;

	/* ======================  CHECKING PARAMETERS ====================== */

	if (0 >= n_proc) {
		fprintf(stderr, "%s\n", "initialize_tables: n_proc is 0 or less");
		return ERROR;
	}

	if (0 >= num_keys) {
		fprintf(stderr, "%s\n", "initialize_tables: num_keys is 0 or less");
		return ERROR;
	}

	/* KEY TABLE */
	for (i = 0; i < num_keys; i++) {

		k_table[i].key_id = i;
		k_table[i].key = key_generator(i);
		k_table[i].founder = 0;
		k_table[i].num_procs_list = 0;

		if (NULL == (k_table[i].procs = malloc(n_proc * sizeof(int)))){ 
			fprintf(stderr, "%s\n", "initialize_tables: ERROR in malloc(procs)");
			return ERROR;
		}

		for(j = 0; j < n_proc; j++) k_table[i].procs[j] = NULL_PROC_ID; /* Ids of procceses working on that key */
	}


	/* PROCCESSES TABLE */
	for (i = 0; i < (n_proc - 1) ; i++) { /* ONLY CALCULATORS */
			
		p_table[i].proc_id = i + 1; /* Because proccess 0 is IO proccess. SO it starts in 1*/
		p_table[i].occupied_flag = 0;

		/* STATISTICS */
		p_table[i].stats.n_keys = 0;
		p_table[i].stats.n_rand_crypt_calls = 0;

		if (NULL == (p_table[i].stats.key_proccesing_times = malloc(num_keys * sizeof(double)))){ 
			fprintf(stderr, "%s\n", "initialize_tables: ERROR in malloc(key_proccesing_times)");
			return ERROR;
		}

		for(j = 0; j < num_keys; j++) p_table[i].stats.key_proccesing_times[j] = 0.0;

	}

	return 1;
}

/***************************************
 *  key_generator                      * 
 ***************************************
 *                                     *
 *  Generates a new key with its       *
 *  correspondent cyprhed combination  *
 *                                     *
 *  Return: the new key (id, key,      *
 *          crypted combination).      *
 ***************************************/
key_data_t key_generator(int id)
{
	key_data_t new_key;

	new_key.key_id = id;
	new_key.key_number =  MIN + rand() % (MAX - MIN);
	strcpy(new_key.cypher, key_encrypter(new_key.key_number));
	if (NULL == new_key.cypher) {
	    fprintf(stderr, "%s\n", "key_generator: ERROR in key_encrypter(new_key.key_number)");
	}

	new_key.length = KEY_LENGTH;

	return new_key;
}

/***************************************
 *  key_encrypter                      * 
 ***************************************
 *                                     *
 *  Uses the function 'crypt' to       *
 *  generate the crypted combination   *
 *  given the real key                 *
 *                                     *
 *  Return: the crypted key.           *
 ***************************************/
char *key_encrypter(unsigned long key) 
{
	char p[KEY_LENGTH];
	sprintf(p, "%08ld", key );
	return crypt(p, "aa");
}

/***************************************
 *  register_type_msg                 * 
 ***************************************
 *                                     *
 *  Generates the struct containing    *
 *  the key and the size of the key    *
 *  in MPI datatypes                   *
 *                                     *
 *  Return: in case of error -1        *
 *			Otherwise, returns 1       *
 ***************************************/
int register_key_type(key_data_t* data, MPI_Datatype* MPI_Type) 
{
	MPI_Datatype types[N_KEY_ELEMENTS];
	int lengths[N_KEY_ELEMENTS];
	MPI_Aint memory_address[N_KEY_ELEMENTS + 1];
	MPI_Aint memory_address_distances[N_KEY_ELEMENTS];

	/* ======================  CHECKING PARAMETERS ====================== */
	
	if (NULL == data) {
	    fprintf(stderr, "%s\n", "register_key_type: data cannot be NULL");
		return ERROR;
	} else if (NULL == MPI_Type) {
	    fprintf(stderr, "%s\n", "register_key_type: MPI_Type cannot be NULL");
		return ERROR;
	}

	/* 
	 * Indicates the types of the struct
	 *
		typedef struct {
			int key_id;								
		    int length;								
			unsigned long key;						
			char cypher[CRYPT_LENGTH];				
		} key_data_t;
	 *
	 */

	types[0] = MPI_INT;
	types[1] = MPI_INT;
	types[2] = MPI_UNSIGNED_LONG;
	types[3] = MPI_CHAR; 

	/* Indicate the numbers of elements of each type */
	lengths[0] = 1;
	lengths[1] = 1;
	lengths[2] = 1;
	lengths[3] = CRYPT_LENGTH;

	/* Calculate the position of the elements in the memory address regarding the beginning of the struct */
	MPI_Address(data, &memory_address[0]);
	MPI_Address(&(data->key_id), &memory_address[1]);
	MPI_Address(&(data->length), &memory_address[2]);
	MPI_Address(&(data->key_number), &memory_address[3]);
	MPI_Address(&(data->cypher), &memory_address[4]);

	memory_address_distances[0] = memory_address[1] - memory_address[0];
	memory_address_distances[1] = memory_address[2] - memory_address[0];
	memory_address_distances[2] = memory_address[3] - memory_address[0];
	memory_address_distances[3] = memory_address[4] - memory_address[0];

	/* Create struct in MPI */
	if (MPI_SUCCESS != MPI_Type_struct(N_KEY_ELEMENTS, lengths, memory_address_distances, types, MPI_Type) ) {
		fprintf(stderr, "%s\n", "register_key_type: ERROR in MPI_Type_struct");
		return ERROR;
	}

	/* Certificate the struct called pMPI_new_data_type before being used */
	if (MPI_SUCCESS != MPI_Type_commit(MPI_Type) ) {
		fprintf(stderr, "%s\n", "register_key_type: ERROR in MPI_Type_Commit (2)");
		return ERROR;
	}

	return 1;
}

/***************************************
 *  register_decrypt_msg               * 
 ***************************************
 *                                     *
 *  Generates the struct containing    *
 *  the key and the size of the key    *
 *  in MPI datatypes                   *
 *                                     *
 *  Return: in case of error -1        *
 			Otherwise, returns 1       *
 ***************************************/
int register_decrypt_msg(msg_decrypt_t* data, MPI_Datatype* MPI_Type)  
{
	MPI_Datatype types[N_DECRYPT_MESSAGE_ELEMENTS];
	int lengths[N_DECRYPT_MESSAGE_ELEMENTS];
	MPI_Aint memory_address[N_DECRYPT_MESSAGE_ELEMENTS + 1];
	MPI_Aint memory_address_distances[N_DECRYPT_MESSAGE_ELEMENTS];

	/* ======================  CHECKING PARAMETERS ====================== */

	if (NULL == data) {
	    fprintf(stderr, "%s\n", "register_decrypt_msg: data cannot be NULL");
		return ERROR;
	} else if (NULL == MPI_Type) {
	    fprintf(stderr, "%s\n", "register_decrypt_msg: MPI_Type cannot be NULL");
		return ERROR;
	}

	/* 
	 * Indicates the types of the struct
	 *
	 * typedef struct {
	 *		int message_id;
	 *		key_t key;
	 *		unsigned long min_value;
	 *      unsigned long max_value;
	 *	} msg_decrypt_t;
	 *
	 */
	types[0] = MPI_INT;
	types[1] = MPI_KEY_T;
	types[2] = MPI_UNSIGNED_LONG;
	types[3] = MPI_UNSIGNED_LONG;

	/* Indicate the numbers of elements of each type */
	lengths[0] = 1;
	lengths[1] = 1;
	lengths[2] = 1;
	lengths[3] = 1;

	/* Calculate the position of the elements in the memory address regarding the beginning of the struct */
	MPI_Address(data, &memory_address[0]);
	MPI_Address(&(data->message_id), &memory_address[1]);
	MPI_Address(&(data->key), &memory_address[2]);
	MPI_Address(&(data->min_value), &memory_address[3]);
	MPI_Address(&(data->max_value), &memory_address[4]);

	memory_address_distances[0] = memory_address[1] - memory_address[0];
	memory_address_distances[1] = memory_address[2] - memory_address[0];
	memory_address_distances[2] = memory_address[3] - memory_address[0];
	memory_address_distances[3] = memory_address[4] - memory_address[0];

	/* Create struct in MPI */
	if (MPI_SUCCESS != MPI_Type_struct(N_DECRYPT_MESSAGE_ELEMENTS, lengths, memory_address_distances, types, MPI_Type) ) {
		fprintf(stderr, "%s\n", "register_decrypt_msg: ERROR in MPI_Type_struct");
		return ERROR;
	}

	/* Certificate the struct called pMPI_new_data_type before being used */
	if (MPI_SUCCESS != MPI_Type_commit(MPI_Type) ) {
		fprintf(stderr, "%s\n", "register_decrypt_msg: ERROR in MPI_Type_Commit");
		return ERROR;
	}

	return 1;
}

/***************************************
 *  register_data_msg                 * 
 ***************************************
 *                                     *
 *  Generates the struct containing    *
 *  the key and the size of the key    *
 *  in MPI datatypes                   *
 *                                     *
 *  Return: in case of error -1        *
 			Otherwise, returns 1       *
 ***************************************/
int register_data_msg(msg_data_t* data, MPI_Datatype* MPI_Type) 
{
	MPI_Datatype types[N_DATA_MESSAGE_ELEMENTS];
	int lengths[N_DATA_MESSAGE_ELEMENTS];
	MPI_Aint memory_address[N_DATA_MESSAGE_ELEMENTS + 1];
	MPI_Aint memory_address_distances[N_DATA_MESSAGE_ELEMENTS];

	/* ======================  CHECKING PARAMETERS ====================== */

	if (NULL == data) {
	    fprintf(stderr, "%s\n", "register_decrypt_msg: data cannot be NULL");
		return ERROR;
	} else if (NULL == MPI_Type) {
	    fprintf(stderr, "%s\n", "register_decrypt_msg: MPI_Type cannot be NULL");
		return ERROR;
	}

	/* 
	   Indicates the types of the struct
	 
	   typedef struct {
			int message_id;							
			key_data_t key;							
			int proccess_id;					
			unsigned long num_tries;				
			double time;	
			int found_flag;						
		} msg_data_t;
	 
	 */
	types[0] = MPI_INT;
	types[1] = MPI_KEY_T;
	types[2] = MPI_INT;
	types[3] = MPI_UNSIGNED_LONG;
	types[4] = MPI_DOUBLE;
	types[5] = MPI_INT;

	/* Indicate the numbers of elements of each type */
	lengths[0] = 1;
	lengths[1] = 1;
	lengths[2] = 1;
	lengths[3] = 1;
	lengths[4] = 1;
	lengths[5] = 1;

	/* Calculate the position of the elements in the memory address regarding the beginning of the struct */
	MPI_Address(data, &memory_address[0]);
	MPI_Address(&(data->message_id), &memory_address[1]);
	MPI_Address(&(data->key), &memory_address[2]);
	MPI_Address(&(data->proccess_id), &memory_address[3]);
	MPI_Address(&(data->num_tries), &memory_address[4]);
	MPI_Address(&(data->time), &memory_address[5]);
	MPI_Address(&(data->found_flag), &memory_address[6]);

	memory_address_distances[0] = memory_address[1] - memory_address[0];
	memory_address_distances[1] = memory_address[2] - memory_address[0];
	memory_address_distances[2] = memory_address[3] - memory_address[0];
	memory_address_distances[3] = memory_address[4] - memory_address[0];
	memory_address_distances[4] = memory_address[5] - memory_address[0];
	memory_address_distances[5] = memory_address[6] - memory_address[0];

	/* Create struct in MPI */
	if (MPI_SUCCESS != MPI_Type_struct(N_DATA_MESSAGE_ELEMENTS, lengths, memory_address_distances, types, MPI_Type) ) {
		fprintf(stderr, "%s\n", "register_data_msg: ERROR in MPI_Type_struct");
		return ERROR;
	}

	/* Certificate the struct called pMPI_new_data_type before being used */
	if (MPI_SUCCESS != MPI_Type_commit(MPI_Type) ) {
		fprintf(stderr, "%s\n", "register_data_msg: ERROR in MPI_Type_Commit");
		return ERROR;
	}

	return 1;
}

/***************************************
 * fill_decrypt_msg                    * 
 ***************************************
 *                                     *
 *  Fills the data_msg_t we'll send    *
 *                                     *
 *  Return: in case of error -1        *
 			Otherwise, returns 1       *
 ***************************************/
int fill_decrypt_msg(msg_decrypt_t *decrypt_msg, key_data_t key , unsigned long min_value, unsigned long max_value) {

    if (NULL == decrypt_msg) {
        fprintf(stderr, "%s\n", "fill_decrypt_msg: decrypt_msg cannot be NULL");
		return ERROR;
    }

	decrypt_msg->message_id=DECRYPT_MESSAGE_TAG;
	decrypt_msg->key=key;
	decrypt_msg->min_value=min_value;
	decrypt_msg->max_value=max_value;
	
	return SUCCESS;
}

/***************************************
 *  search_free_procs                  * 
 ***************************************
 *                                     *
 *  Searchs for any key that is not    *
 *  assigned to a proccess             *
 *  in the keys´ table.                *
 *  All the procceses must have a key  *
 *  at the beggining of the program.   *
 *  When that happens,                 *
 *  this will return 0                 *
 *                                     *
 *  Return: if a key with no proccess  *
 *          assigned is found          *
 *          returns 1                  *
 *			Otherwise, returns 0       *
 ***************************************/
int search_free_procs(proc_table_t p_table, int num_proc, int* proccess_id) 
{	
	/* ======================  CHECKING PARAMETERS ====================== */
    int  i;
    
	if (0 >= num_proc) {
		fprintf(stderr, "%s\n", "search_free_procs: num_proc is 0 or less");
		return ERROR;
	} else if (NULL == p_table) {
	    fprintf(stderr, "%s\n", "search_free_procs: p_table cannot be NULL");
		return ERROR;
	} else if (NULL == proccess_id) {
	    fprintf(stderr, "%s\n", "search_free_procs: proccess_id cannot be NULL");
		return ERROR;
	}

	/* It´s n - 1 because we don´t count IO_Proccess on the table. Only calculators  */
	for (i = 0 ; i < (num_proc - 1); i++) { 
		if (0 == p_table[i].occupied_flag){ 
			*proccess_id = p_table[i].proc_id;
			return 1;
		}
	}

	return 0; // All the proccesses are occupied
}


/***************************************
 *  distribute_work                    * 
 ***************************************
 *                                     *
 *  Distributes the work between the   *
 *  processes so they can get a new    *
 *  key to work with.                  *
 *                                     *
 *  Return: if ERROR returns -1.       *
 *			Otherwise, returns 0.      *
 ***************************************/
int distribute_work(int num_procs, unsigned long *starting_values)
{
    int i = -1;
    unsigned long interval = 0;
    
    if (num_procs <= 0) {
        fprintf(stderr, "%s\n", "distribute_work: num_procs is 0 or less");
		return ERROR;
    } else if (NULL == starting_values) {
        fprintf(stderr, "%s\n", "distribute_work: starting_values cannot be NULL");
		return ERROR;
    }
    
    interval = (MAX - MIN) / num_procs;
    
    starting_values[0] = MIN;
    for (i = 1; i < num_procs; ++i) {
        starting_values[i] = starting_values[i - 1] + interval;
    }
    
    return 0;
    
}

/***************************************
 *  assign_key_to_proccess             * 
 ***************************************
 *                                     *
 *  Assigns a key to a specific        *
 *  proccess at the beginning of       *
 *  the execution.                     *
 *                                     *
 *  Return: in case of error -1        *
 * 			Otherwise, returns 1       *
 ***************************************/
int assign_key_to_proccess(int proc_id, key_table_t k_table, proc_table_t p_table, int num_keys, int num_procs) 
{
	key_data_t key_to_assign;
	key_to_assign.key_id = -1;
	int num_procs_calc = 0;
	int *procs_calc = NULL; /* Array containing the id of the proccesses that are calculating a key*/
	int i = 0;
	unsigned long *starting_values = NULL;

	/* Flags */
	int keys_without_procs_flag = 0;

	/* Messages */
	msg_decrypt_t decrypt_msg;
	msg_data_t data_msg;
	msg_request_data_t request_msg;

	/* MPI additional parameters for Send an Recv */
	MPI_Request request;
	MPI_Status status;

	/* Minimum and maximum values */
	unsigned long min_search_value = MIN;
	unsigned long max_search_value = MAX;

	/* ======================  CHECKING PARAMETERS ====================== */

	if (proc_id < 0) {
		fprintf(stderr, "%s\n", "assign_key_to_proccess: procces_id is less than 0");
		return ERROR_1;
	} else if (0 == num_keys) {
		fprintf(stderr, "%s\n", "assign_key_to_proccess: num_keys is 0");
		return ERROR_1;
	} else if (0 == num_procs) {
		fprintf(stderr, "%s\n", "assign_key_to_proccess: num_procs is 0");
		return ERROR_1;
	} else if (NULL == k_table) {
	    fprintf(stderr, "%s\n", "assign_key_to_proccess: k_table cannot be NULL");
		return ERROR_1;
	} else if (NULL == p_table) {
	    fprintf(stderr, "%s\n", "assign_key_to_proccess: p_table cannot be NULL");
		return ERROR_1;
	}
	
	if (NULL == (procs_calc = malloc(num_procs * sizeof(int)))) { 
		fprintf(stderr, "%s\n", "assign_key_to_proccess: ERROR in malloc(procs_calc)");
		return ERROR_2;
	}
	
	for(i = 0; i < num_procs ; i++)
	    procs_calc[i] = -1;
	
	if (NULL == (starting_values = malloc(num_procs_calc * sizeof(unsigned long)))) {
	    fprintf(stderr, "%s\n", "assign_key_to_proccess: ERROR in malloc (starting_values)");
		return ERROR_2;
	}
	
	/* Although its main purpose is search keys with the less number of procceses, it also works for what we want here */
	/* Which is finding keys with 0 proccesses asigned */	
	keys_without_procs_flag = search_keys_with_min_num_of_procs(k_table, num_keys, &num_procs_calc, procs_calc, &key_to_assign);
	
	if (KEYS_LEFT == keys_without_procs_flag) { // There is some free key
	
		if (-1 == fill_decrypt_msg(&decrypt_msg, key_to_assign , MIN, MAX) ) {
			fprintf(stderr, "%s\n", "assign_key_to_proccess: ERROR in fill_decrypt_msg (1)");
			return ERROR_3;
		}
		
		if (MPI_SUCCESS != MPI_Send(&decrypt_msg, 1, MPI_DECRYPT_MSG_T, proc_id, DECRYPT_MESSAGE_TAG, MPI_COMM_WORLD) ) { 
			fprintf(stderr, "%s\n", "assign_key_to_proccess: ERROR in MPI_Send (1)");
			return ERROR_4;
		}
		
		/* Register the proccess calculating the key */
		if (-1 == associate_proc_to_key(proc_id, key_to_assign.key_id, k_table, p_table) ){
			fprintf(stderr, "%s\n", "assign_key_to_proccess: ERROR in associate_proc_to_key");
			return ERROR_3;
		}
		
		return SUCCESS;

	} else { // All the keys have been asigned
	
		for (i = 0; i < num_procs_calc; i++) {
			if (MPI_SUCCESS != MPI_Isend(&request_msg, 1, MPI_INT, procs_calc[i], REQUEST_DATA_MESSAGE_TAG, MPI_COMM_WORLD, &request) ) {
				fprintf(stderr, "%s\n", "assign_key_to_proccess: ERROR in MPI_Send (2)");
				return ERROR_4;
			}
		}

		for (i = 0; i < num_procs_calc; i++) {

			if (MPI_SUCCESS != MPI_Recv(&data_msg , 1, MPI_DATA_MSG_T, procs_calc[i], DATA_MESSAGE_TAG, MPI_COMM_WORLD, &status) ) {
				fprintf(stderr, "%s\n", "assign_key_to_proccess: ERROR in MPI_Recv ");
				return ERROR_4;
			}

			if (-1 == store_stats_data(p_table, data_msg, num_procs) ){
				fprintf(stderr, "%s\n", "assign_key_to_proccess: ERROR in store_stats_data");
				return ERROR_3;
			}

		}

		/* Distribuir trabajo equitativamente */
		distribute_work(num_procs_calc + 1, starting_values); // +1 because there is a new process
		
		/* Notify the procceses that their key is going to be calculated by a new proccess */
		for (i = 0; i < num_procs_calc - 1; i++) {

			if (-1 == fill_decrypt_msg(&decrypt_msg, key_to_assign , starting_values[i], starting_values[i + 1] - 1) ) {
				fprintf(stderr, "%s\n", "assign_key_to_proccess: ERROR in fill_decrypt_msg (1)");
				return ERROR_3;
			}

			if (MPI_SUCCESS != MPI_Send(&decrypt_msg, 1, MPI_DECRYPT_MSG_T, procs_calc[i], DECRYPT_MESSAGE_TAG, MPI_COMM_WORLD) ) {
				fprintf(stderr, "%s\n", "assign_key_to_proccess: ERROR in MPI_Send (3)");
				return ERROR_4;
			}
		}
		
		/* Send the key to the new proccess */
		if (-1 == fill_decrypt_msg(&decrypt_msg, key_to_assign , starting_values[num_procs_calc - 1], MAX) ) {
			fprintf(stderr, "%s\n", "assign_key_to_proccess: ERROR in fill_decrypt_msg (2)");
			return ERROR_3;
		}

		if (MPI_SUCCESS != MPI_Send(&decrypt_msg, 1, MPI_DECRYPT_MSG_T, proc_id, DECRYPT_MESSAGE_TAG, MPI_COMM_WORLD) ) {
			fprintf(stderr, "%s\n", "assign_key_to_proccess: ERROR in MPI_Send (4)");
			return ERROR_4;
		}
		
		// Update process list in key table for the key we have just assigned
		num_procs_calc = ++k_table[key_to_assign.key_id].num_procs_list;
		k_table[key_to_assign.key_id].procs[num_procs_calc - 1] = proc_id;
		
		return SUCCESS;
	}
}

/****************************************
 *  search_keys_with_min_num_of_procs   * 
 ****************************************
 *                                      *
 *  Searchs for the key that is been    *
 *	decrypted by the minor number of    *
 *	proccesses                          *
 *                                      *
 *  Return: number of keys found        *
 *          if successes.               *
 *			Otherwise, returns          *
 *          NO KEYS_LEFT macro          *
 ****************************************/
int search_keys_with_min_num_of_procs(key_table_t k_table, int num_keys, int* num_procs, int* procs_calc, key_data_t *key)
{
	int i = 0;
	int first_free_key = -1;
	int key_id = -1;

	/* ======================  CHECKING PARAMETERS ====================== */

	if (0 == num_keys) {
		fprintf(stderr, "%s\n", "search_keys_with_min_num_of_procs: num_keys is 0");
		return ERROR_1;
	} else if (NULL == k_table) {
	    fprintf(stderr, "%s\n", "search_keys_with_min_num_of_procs: k_table cannot be NULL");
		return ERROR_1;
	} else if (NULL == num_procs) {
	    fprintf(stderr, "%s\n", "search_keys_with_min_num_of_procs: num_procs cannot be NULL");
		return ERROR_1;
	} else if (NULL == procs_calc) {
	    fprintf(stderr, "%s\n", "search_keys_with_min_num_of_procs: procs_calc cannot be NULL");
		return ERROR_1;
	} else if (NULL == key) {
	    fprintf(stderr, "%s\n", "search_keys_with_min_num_of_procs: key cannot be NULL");
		return ERROR_1;
	}
	
	// Get first free key as minimum
	for (i = 0; i < num_keys; ++i){
	    if (0 == k_table[i].founder){
	        first_free_key = i;
	        key_id = i;
	        *num_procs = k_table[first_free_key].num_procs_list;
	        *key = k_table[first_free_key].key;
	        break;
	    }
    }
	
	if (-1 == first_free_key) // No free keys
	    return NO_KEYS_LEFT;
    
    // Look for minimum
	for (i = first_free_key + 1; i < num_keys; i++) {
	    
	    if (k_table[i].founder)
	        continue;

		/* If there are 2 or more keys with the same number of proccesses, p.e. 0 procceses, we get the first one we picked. */
		if (*num_procs > k_table[i].num_procs_list) { /* That´s why we put > and not >= */
		
			*num_procs = k_table[i].num_procs_list;
			*key = k_table[i].key;	
			key_id = i;
			
			if (0 == *num_procs){ /* There is at least one key with no proccesses asigned */
		        return KEYS_LEFT;
	        }
		}
	}
	
	/* Get the list of proccesses calculating the key */
	for (i = 0 ; i < k_table[key_id].num_procs_list; i++){
		procs_calc[i] = k_table[key_id].procs[i];
	}
	
	return KEYS_LEFT;
}

/***************************************
 *  associate_proc_to_key              * 
 ***************************************
 *                                     *
 *  Adds a new proccess to the list    *
 *	of proccess doing a specific key   *
 *                                     *
 *  Return: in case of error -1        *
 *			Otherwise, returns 0       *
 ***************************************/
int associate_proc_to_key(int proc_id, int key_id, key_table_t k_table, proc_table_t p_table)
{
	int num_procs_list = 0;

	/* ======================  CHECKING PARAMETERS ====================== */

	if (-1 == proc_id) {
		fprintf(stderr, "%s\n", "associate_proc_to_key: key_id is -1");
		return ERROR;
	} else if (-1 == key_id) {
		fprintf(stderr, "%s\n", "associate_proc_to_key: key_id is -1");
		return ERROR;
	} else if (NULL == k_table) {
	    fprintf(stderr, "%s\n", "associate_proc_to_key: k_table cannot be NULL");
		return ERROR;
	} else if (NULL == p_table) {
	    fprintf(stderr, "%s\n", "associate_proc_to_key: p_table cannot be NULL");
		return ERROR;
	}

	/* It´s proc_id - 1 because the first proccess (id = 1) is in position number 0 in the table */
	p_table[proc_id - 1].occupied_flag = 1; /* A key has been asigned to him */

	num_procs_list = k_table[key_id].num_procs_list;
	k_table[key_id].procs[num_procs_list] = proc_id;

	(k_table[key_id].num_procs_list)++;

	return 0;
}

/***************************************
 * store_stats_data                    * 
 ***************************************
 *                                     *
 *  Fills the data_msg_t we'll send    *
 *                                     *
 *  Return: in case of error -1        *
 *			Otherwise, returns 1       *
 ***************************************/
int store_stats_data(proc_table_t p_table, msg_data_t data_msg, int num_procs){
	
	int n_proc;

	/* ======================  CHECKING PARAMETERS ====================== */

	if (0 == num_procs) {
		fprintf(stderr, "%s\n", "store_stats_data: num_procs is 0");
		return ERROR;
	} else if (NULL == p_table) {
	    fprintf(stderr, "%s\n", "store_stats_data: p_table cannot be NULL");
		return ERROR;
	} 
	
	n_proc = data_msg.proccess_id - 1;
	if (0 <= n_proc && n_proc < num_procs) {
	    if (data_msg.found_flag)
	        p_table[n_proc].stats.n_keys += 1;
		p_table[n_proc].stats.n_rand_crypt_calls += data_msg.num_tries;
		p_table[n_proc].stats.key_proccesing_times[p_table[n_proc].stats.n_keys - 1] = data_msg.time;
		return 1;
	} else {
	    return ERROR;
	}
}

/****************************************
 *  update_tables_after_key_found       * 
 ****************************************
 *                                      *
 *  Updates the key and proccess tables *
 *  after a key has been found          *
 *                                      *
 *  Return: in case of error -1         *
 *			Otherwise, returns 0        *
 ****************************************/
int update_tables_after_key_found(msg_data_t data_msg, key_table_t k_table, proc_table_t p_table)
{
    int i, j;
	int k_id = data_msg.key.key_id;
	int n_procs_key = k_table[k_id].num_procs_list;
	int proc_id = -1;

	/* ======================  CHECKING PARAMETERS ====================== */

	if (NULL == k_table) {
	    fprintf(stderr, "%s\n", "update_tables_after_key_found: k_table cannot be NULL");
		return ERROR;
	} else if (NULL == p_table) {
	    fprintf(stderr, "%s\n", "update_tables_after_key_found: p_table cannot be NULL");
		return ERROR;
	} 

	// Iterates over the number of processes asociated to a key
	for (i = 0; i < n_procs_key; i++) {
	
		if (data_msg.proccess_id == k_table[k_id].procs[i]) {
		
			proc_id = k_table[k_id].procs[i];
			
			p_table[proc_id - 1].occupied_flag = 0;
			for (j = i + 1; j < n_procs_key; ++j) { // Move every position backwards. Like a queue
			    k_table[k_id].procs[j- 1] = k_table[k_id].procs[j];
			}
			
			k_table[k_id].procs[n_procs_key - 1] = NULL_PROC_ID;
			(k_table[k_id].num_procs_list)--;
			
			if (data_msg.found_flag)
			    k_table[k_id].founder = data_msg.proccess_id;
			
			return 0;
		}
	}
	return 0;
}

/***************************************
 *  are_there_keys_not_decrypted       * 
 ***************************************
 *                                     *
 *  Searchs for any key that has not   *
 *  been decrypted yet                 *
 *  in the keys´ table                 *
 *                                     *
 *  Return: if a non-decrypted key     *
 *          is found returns the id    *
 *			Otherwise, returns -1      *
 ***************************************/
int are_there_keys_not_decrypted(key_table_t k_table, int num_keys) 
{
    int i;    
    
	/* ======================  CHECKING PARAMETERS ====================== */

	if (0 >= num_keys) {
		fprintf(stderr, "%s\n", "are_there_keys_not_decrypted: num_keys is 0 or less");
		return ERROR;
	} else if (NULL == k_table) {
	    fprintf(stderr, "%s\n", "are_there_keys_not_decrypted: k_table cannot be NULL");
		return ERROR;
	}

	for (i = 0; i < num_keys; i++) {
		if (0 == k_table[i].founder) 
			return i;
	}

	return NO_KEYS_LEFT; // All the keys have been decrypted
}

/****************************************
 *  key_decrypter                       * 
 ****************************************
 *                                      *
 *  Decrypts the crypted combination    *
 *  to get the original key             *
 *                                      *
 *  Return: if the key is found returns *
 *          KEY_FOUND macro.            *
 *			if not found returns        *
 *			KEY_NOT_FOUND macro.        *
 *			Otherwise, returns 1        *
 ****************************************/
int key_decrypter(msg_decrypt_t* msg)
{
	char decrypt_string[msg->key.length];
	char *ptr = NULL;
	
	if (NULL == msg) {
	    fprintf(stderr, "%s\n", "key_decrypter: msg is NULL");
		return ERROR;
	}

	sprintf(decrypt_string, "%08ld", (msg->min_value + rand() % (msg->max_value - msg->min_value) ));
	ptr = crypt(decrypt_string, "aa");

	if (0 == strcmp(ptr, msg->key.cypher) ) {
		msg->key.key_number = strtoul(decrypt_string, &ptr, 0);
		return KEY_FOUND;
	}

	return KEY_NOT_FOUND; //Not found
}

/***************************************
 * fill_data_msg                       * 
 ***************************************
 *                                     *
 *  Fills the data_msg_t we'll send    *
 *                                     *
 *  Return: in case of error -1        *
 *			Otherwise, returns 1       *
 ***************************************/

int fill_data_msg(msg_data_t* data_msg, key_data_t key, int proc_id, int num_tries, clock_t begin, clock_t end, int found_flag){

    if (NULL == data_msg) {
        fprintf(stderr, "%s\n", "fill_data_msg: data_msg is NULL");
		return ERROR;
    }

	data_msg->message_id = DATA_MESSAGE_TAG;
	data_msg->key = key;
	data_msg->proccess_id = proc_id;
	data_msg->num_tries = num_tries;
	data_msg->time= (double)(end-begin);
	data_msg->found_flag = found_flag;
	return SUCCESS;
}

/******************************************
 * search_free_keys                       * 
 ******************************************
 *                                        *
 *  Looks for a non decrypted key         *
 *                                        *
 *  Return: in case of error ERROR        *
 *          if free keys KEYS_LEFT        *
 *          if no free keys NO_KEYS_LEFT  *
 ******************************************/
int search_free_keys(key_table_t k_table, int num_keys, int *key)
{
    int i;
    int free_key;

    if (NULL == k_table) {
        fprintf(stderr, "%s\n", "search_free_keys: k_table is NULL");
		return ERROR;
    } else if (NULL == key) {
        fprintf(stderr, "%s\n", "search_free_keys: key is NULL");
		return ERROR;
    } else if (num_keys < 0) {
        fprintf(stderr, "%s\n", "search_free_keys: num_keys cannot be less than 0");
		return ERROR;
    }
    
    for (i = 0; i < num_keys; ++i) {
        if (0 == k_table[i].founder && 0 == k_table[i].num_procs_list) {
            *key = i;
            return KEYS_LEFT;
        }
    }
    
    return NO_KEYS_LEFT;
}

/*********************************************
 * update_metadata                           * 
 *********************************************
 *                                           *
 *  Join of both functions, store_stats_data *
 *  and update_tables_after_key_found        *
 *                                           *
 *  Return: in case of error -1.             *
 *          Otherwise returns                *
 *          SUCCESS macro                    *
 *********************************************/
int update_metadata(msg_data_t data_msg,
                    key_table_t k_table, int num_keys,
                    proc_table_t p_table, int num_procs)
{

    if (NULL == k_table) {
        fprintf(stderr, "%s\n", "update_metadata: k_table is NULL");
		return ERROR;
    } else if (NULL == p_table) {
        fprintf(stderr, "%s\n", "update_metadata: p_table is NULL");
		return ERROR;
    } else if (num_keys < 0) {
        fprintf(stderr, "%s\n", "update_metadata: num_keys is less than 0");
		return ERROR;
    } else if (num_procs < 0) {
        fprintf(stderr, "%s\n", "update_metadata: num_procs is less than 0");
		return ERROR;
    }
    
    // And now process the datas
    if (-1 == store_stats_data(p_table, data_msg, num_procs) ){
	    fprintf(stderr, "%s\n", "update_metadata: ERROR in store_stats_data");
	    return ERROR;
    }

    if (-1 == update_tables_after_key_found(data_msg, k_table, p_table)) {
	    fprintf(stderr, "%s\n", "update_metadata: ERROR in update_tables_after_key_found");
	    return ERROR;
    }
    
    return SUCCESS;
}



/* ========================================================  DEBUGGIN FUNCTIONS ======================================================== */


/***************************************
 *  print_key_table                    * 
 ***************************************
 *                                     *
 *  Prints the info of the key table   *
 *                                     *
 *  Return: if ERROR returns -1.       *
 *          Otherwise returns 0.       *
 ***************************************/
int print_key_table(key_table_t key_table, int nkeys) {
    
    int i, j;
    char format[] = "ID: %d | Cipher: %s | Key: %lu | Decrypted: %d | NumProcsAssigned %d\n";
    
    if (NULL == key_table || nkeys < 0)
        return ERROR;
    
    printf("%s\n\n", "Keys' table is:");
    for (i = 0; i < nkeys; ++i) {
        printf(format,
                key_table[i].key_id,
                key_table[i].key.cypher,
                key_table[i].key.key_number,
                key_table[i].founder,
                key_table[i].num_procs_list
        );
        printf("%s", "\t|-> Procs: ");
        for (j = 0; j < key_table[i].num_procs_list; ++j)
            printf("%d ", key_table[i].procs[j]);
        putchar('\n');
    }
    
    return 0;
}

/***************************************
 *  print_proc_table                   * 
 ***************************************
 *                                     *
 *  Prints the info of the processes   *
 *  table                              *
 *                                     *
 *  Return: if ERROR returns -1.       *
 *          Otherwise returns 0.       *
 ***************************************/
int print_proc_table(proc_table_t proc_table, int nprocs) {

    int i;
    char format[] = "Proc_id: %d | Occupied: %s | Keys found: %d | Calls to rand/crypt: %lu\n";
    
    if (NULL == proc_table || nprocs < 0)
        return ERROR;
    
    printf("%s\n\n", "Processes' table is:");
    for (i = 0; i < nprocs; ++i) {
        printf(format,
                proc_table[i].proc_id,
                proc_table[i].occupied_flag ? "TRUE" : "FALSE",
                proc_table[i].stats.n_keys,
                proc_table[i].stats.n_rand_crypt_calls
        );
    }
    
    return 0;
}