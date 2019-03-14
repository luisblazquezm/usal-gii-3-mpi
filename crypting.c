/*************************************************
 *  Source: crypting.c                           *
 *                                               *
 *  @author: Luis Blázquez Miñambres             *
 *           Miguel Cabezas Puerto               *
 *           Samuel Gómez Sánchez                *
 *           Alberto Hernández Pintor            *
 *                                               *
 *  @version: 1.0                                *
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
 * =================== SPECIAL COMMENTS ===================
 *
 *  > DEGUG means it´s only a fancy message to test something
 *  > VOID MESSAGE means we are sending a message that is not filled yet.
 *  > NOT DONE YET, task not implemented yet
 *
 *  ** TO FIND ANY OF THESE IN THE CODE USE CTRL+F **
 * 
 *
 *
 *
 *  Also to compile : mpicc crypting.c crypting.h -lcrypt -o crypt
 *  To execute it : mpirun -np [numProcesos] crypt [arguments]
 */

int main(int argc, char *argv[]) 
{
	MPI_Init(&argc, &argv);

		int id = -1;

		if (MPI_SUCCESS != MPI_Comm_rank(MPI_COMM_WORLD, &id)) {
			fprintf(stderr, "%s\n", "main: ERROR in MPI_Comm_rank");
		}

		printf("Hello, I am proccess nº %d\n", id); /* DEBUG */
		printf("Pin pan trucu trucu %s", argv[1]); /* DEBUG */

		if (id == 0) {
			if (-1 == IO_proccess(argv)) {
				fprintf(stderr, "%s\n", "main: ERROR in IO_proccess");
				return -1;
			}
		}

		if (-1 == calculator_proccess(argv, id)) {
			fprintf(stderr, "%s\n", "main: ERROR in calculator_proccess");
			return -1;
		}

	MPI_Finalize();

	return 0;
}

/**************************************************
 *  ============= calculator_proccess ============ * 
/**************************************************
/**************************************************
 *  1) Wait for the key to arrive from IO proccess *
 *  2) Try to decrypt it                          *
 *     2.1) Found it --> notifies the rest of     *
 *			proccesses                             *
 *     2.2) Not Found it --> keeps trying :)      *
 *  3) Receives a new key                         *
 **************************************************/
int calculator_proccess(char *argv[], int proccess_id) 
{
	int num_keys = 0;

	/* Flags */
    int key_found = 0;
    int flag_probe = 0;
    int key_available = 0;
    int finish_execution = 0;

    /* Messages */
	msg_decrypt_t decrypt_msg;
	msg_data_t data_msg;

	key_data_t dec_key; 				// Decrypted key
	unsigned long long num_tries = 0; 	// Number of tries
	clock_t begin, end;					// Time to decrypt a key

	/* MPI additional parameters for Send an Recv */
	MPI_Status status;
	MPI_Request request;

	/* MPI Datatypes to create ---> Structs */
	MPI_Datatype MPI_DECRYPT_MSG_T;
	MPI_Datatype MPI_DATA_MSG_T;

	/* ======================  CHECKING PARAMETERS ====================== */

	if(argv[1] == NULL){
		printf("Introduce the number of keys that you want generate\n"); /* DEBUG */
	}else{
		num_keys = atoi(argv[1]);
	}

	if (-1 == proccess_id) {
		fprintf(stderr, "%s\n", "calculator_proccess: procces_id is -1");
		return -1;
	}

	/* ======================  CREATING TYPES OF MESSAGES ====================== */

	if (-1 == construct_decrypt_msg(num_keys, &decrypt_msg, &MPI_DECRYPT_MSG_T)) {
		fprintf(stderr, "%s\n", "calculator_proccess: construct_decrypt_msg");
		return -1;
	}

	if (-1 == construct_data_msg(num_keys, &data_msg, &MPI_DATA_MSG_T)) {
		fprintf(stderr, "%s\n", "calculator_proccess: construct_data_msg");
		return -1;
	}

	/* ======================  DECRYPTING / SENDING AND RECEIVING ====================== */

	do{

		if (MPI_SUCCESS != MPI_Recv(&decrypt_msg , 1, MPI_DECRYPT_MSG_T, IO_PROCESS_ID, DECRYPT_MESSAGE_TAG, MPI_COMM_WORLD, &status) ) {
			fprintf(stderr, "%s\n", "calculator_proccess: ERROR in MPI_Recv (1)");
			return -1;
		}

		/* Beggining search of key */
		begin = clock();

		do{
			num_tries++;
			key_available = key_decrypter(decrypt_msg, &end, &key_found);

			if(key_found == 1){

				/* Store this data into the data message to send *///<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< NOT DONE YET
				

				if (MPI_SUCCESS != MPI_Send(&data_msg , 1, MPI_DATA_MSG_T, IO_PROCESS_ID, DATA_MESSAGE_TAG, MPI_COMM_WORLD) ) {
					fprintf(stderr, "%s\n", "calculator_proccess: ERROR in MPI_Send (1)");
					return -1;
				}

				key_available = 1;

				break;
			}


			if (MPI_SUCCESS != MPI_Iprobe(IO_PROCESS_ID, MPI_ANY_TAG, MPI_COMM_WORLD, &flag_probe, &status)) {
				fprintf(stderr, "%s\n", "calculator_proccess: ERROR in MPI_Iprobe");
				return -1;
			}


			if(flag_probe != 0){

				switch(status.MPI_TAG) {

					case DECRYPT_MESSAGE_TAG:

						/* Store data into the decrypt message to send *///<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< NOT DONE YET

						if (MPI_SUCCESS != MPI_Irecv(&decrypt_msg , 1, MPI_DECRYPT_MSG_T, IO_PROCESS_ID, DECRYPT_MESSAGE_TAG, MPI_COMM_WORLD, &request) ) {
							fprintf(stderr, "%s\n", "calculator_proccess: ERROR in MPI_IRecv");
							return -1;
						}

					break;

					case REQUEST_DATA_MESSAGE_TAG:

						if(key_found == 1){

							/* Store data into the data message to send *///<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< NOT DONE YET

							if (MPI_SUCCESS != MPI_Send(&data_msg , 1, MPI_DATA_MSG_T, IO_PROCESS_ID, DATA_MESSAGE_TAG, MPI_COMM_WORLD) ) { /* VOID MESSAGE */
								fprintf(stderr, "%s\n", "calculator_proccess: ERROR in MPI_Send (2)");
								return -1;
							}

						}

						key_available = 1;
						
					break;

					case FINISH_EXECUTION_MESSAGE_TAG:
						finish_execution = 1;
					break;
				}
			}

		} while(key_available == 0);

	} while(finish_execution == 0);
}

/**************************************************
 *  ================ IO_proccess ================  * 
/**************************************************
 *  1) Generate random keys                       *
 *  2) Encrypt the keys                           *
 *  3) Constructs the type of MPI for structs     *
 *  3) Send the keys to calculator proccesses     *
 **************************************************/
int IO_proccess(char *argv[]) 
{
	int num_keys = 0;
	int i = 0;
	int num_procs = 0;
	int proc_id = -1;

	key_data_t key_to_assign;
	key_to_assign.key_id = -1;

	/* Tables */
	key_table_t k_table[num_keys]; 		 // Table of keys 
	proc_table_t p_table[num_procs];     // Table of proccesses 

	/* Messages */
	msg_decrypt_t decrypt_msg;
	msg_data_t data_msg;
	int finish_exec_msg = FINISH_EXECUTION_MESSAGE_TAG;
	int request_data_msg = REQUEST_DATA_MESSAGE_TAG;

	/* Flags*/
	int msg_received_flag = 0;
	int free_procs_flag = 0;

	/* MPI stuff to create the group */
	MPI_Group MPI_GROUP_WORLD;           
	MPI_Group id_group;                  /* Group identifier */
	MPI_Comm comm_group;                 /* Group Communication identifier */
	MPI_Request request;
	MPI_Status status;

	/* MPI Datatypes to create ---> Structs */
	MPI_Datatype MPI_DECRYPT_MSG_T;
	MPI_Datatype MPI_DATA_MSG_T;

	/* Data message reception */
	int num_procs_key = 0, key_id_rcv = -1, proc_id_rcv = -1;

	if(argv[1] == NULL) {
		printf("Introduce the number of keys that you want generate\n");
	} else {
		num_keys = atoi(argv[1]);
	}


	if (MPI_SUCCESS != MPI_Comm_size(MPI_COMM_WORLD, &num_procs)) {
		fprintf(stderr, "%s\n", "IO_proccess: ERROR in MPI_Comm_size");
		return -1;
	}

	/* ======================  CREATE TABLE OF KEYS AND PROCCESSES ====================== */

	if (-1 == initialice_table_of_keys(k_table, p_table, num_procs, num_keys)) { //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< k_table and p_table by reference!!!!!!
		fprintf(stderr, "%s\n", "IO_proccess: initialice_table_of_keys");
		return -1;
	}

	/* ======================  CREATING TYPES OF MESSAGES ====================== */

	if (-1 == construct_decrypt_msg(num_keys, &decrypt_msg, &MPI_DECRYPT_MSG_T)) {
		fprintf(stderr, "%s\n", "IO_proccess: construct_decrypt_msg");
		return -1;
	}

	if (-1 == construct_data_msg(num_keys, &data_msg, &MPI_DATA_MSG_T)) {
		fprintf(stderr, "%s\n", "IO_proccess: construct_data_msg");
		return -1;
	}

	/* ======================  SENDING KEYS ====================== */

	/* Find free proccesses and assign it to them */
	while ( search_free_procs(p_table, num_procs, &proc_id) ) {

		/* Assign a key to a proccess */
		if (-1 == assign_key_to_proccess(proc_id, k_table, num_keys, num_procs)) {
			fprintf(stderr, "%s\n", "IO_proccess: ERROR in assign_key_to_proccess (1)");
			return -1;
		}

		/* Assign a key to a proccess */
		if (0 == search_free_procs(p_table, num_procs, &proc_id) ) break; // No keys without a proccess asociated

		/* Checks if there is a data message or not */
		if (MPI_SUCCESS != MPI_Iprobe(MPI_ANY_SOURCE, DATA_MESSAGE_TAG, MPI_COMM_WORLD, &msg_received_flag, &status)) {
			fprintf(stderr, "%s\n", "IO_proccess: ERROR in MPI_Iprobe");
			return -1;
		}

		if (msg_received_flag) {

			/* Data reception from the calculator proccesses */
			if (MPI_SUCCESS != MPI_Irecv(&data_msg , 1, MPI_DATA_MSG_T, MPI_ANY_SOURCE, DATA_MESSAGE_TAG, MPI_COMM_WORLD, &request) ) {
				fprintf(stderr, "%s\n", "IO_proccess: ERROR in MPI_IRecv");
				return -1;
			}

			/* Store data from the data message received *///<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< NOT DONE YET
			key_id_rcv = data_msg.key.key_id;
			proc_id_rcv = data_msg.proccess_id;

			/* IMprimir clave encontrada en tiempo real *///<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< NOT DONE YET

			/* Assign a key to a proccess */
			if (-1 == assign_key_to_proccess(proc_id, k_table, num_keys, num_procs)) {
				fprintf(stderr, "%s\n", "IO_proccess: ERROR in assign_key_to_proccess (2)");
				return -1;
			}
		}

	}

	while ( are_there_keys_not_decrypted(k_table, num_keys) ) {

		/* Find free proccesses and assign it to them */
		free_procs_flag = search_free_procs(p_table, num_procs, &proc_id); 

		if (0 == free_procs_flag) { // No free procs

			/* Receives the data_msg from the proccess that found the key */
			if (MPI_SUCCESS != MPI_Recv(&data_msg , 1, MPI_DATA_MSG_T, MPI_ANY_SOURCE, DATA_MESSAGE_TAG, MPI_COMM_WORLD, &status) ) {
				fprintf(stderr, "%s\n", "IO_proccess: ERROR in MPI_Recv (1)");
				return -1;
			}

		} 

		/* Store data from the data message received *///<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< NOT DONE YET
		key_id_rcv = data_msg.key.key_id;
		proc_id_rcv = data_msg.proccess_id;
		num_procs_key = k_table[key_id_rcv]->num_procs_list;

		for (i = 0; i < num_procs_key; i++) {

			if (MPI_SUCCESS != MPI_Send(&request_data_msg , 1, MPI_INT, k_table[key_id_rcv]->procs[i], REQUEST_DATA_MESSAGE_TAG, MPI_COMM_WORLD) ) {
				fprintf(stderr, "%s\n", "IO_proccess: ERROR in MPI_Send (2)");
				return -1;
			}
		}
		
		for (i = 0; i < num_procs_key; i++) {

			if (MPI_SUCCESS != MPI_Recv(&data_msg , 1, MPI_DATA_MSG_T, k_table[key_id_rcv]->procs[i], DATA_MESSAGE_TAG, MPI_COMM_WORLD, &status) ) {
				fprintf(stderr, "%s\n", "IO_proccess: ERROR in MPI_Recv (2)");
				return -1;
			}

		}

		/* Store data from the data message received *///<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< NOT DONE YET

		if (are_there_keys_not_decrypted(k_table, num_keys)) {


			for (i = 0; i < num_procs_key; i++) {//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< NOT DONE YET. I dont understand this part ¯\_(-.-)_/¯

				/* Assign a key to a proccess */
				if (-1 == assign_key_to_proccess(proc_id, k_table, num_keys, num_procs)) {
					fprintf(stderr, "%s\n", "IO_proccess: ERROR in assign_key_to_proccess (3)");
					return -1;
				}
			}

		} else {
			break; // There are no keys left to be decrypted
		}


	}

	/* Finalize execution. Sending kill message to the proccesses */
	if (MPI_SUCCESS != MPI_Bcast(&finish_exec_msg , 1, MPI_INT, IO_PROCESS_ID, MPI_COMM_WORLD) ) {
		fprintf(stderr, "%s\n", "IO_proccess: ERROR in MPI_Bcast");
		return -1;
	}
	

	/* SHOW STATISTICS and all that stuff *///<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< NOT DONE YET
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
int assign_key_to_proccess(int proc_id, key_table_t k_table[], int num_keys, int num_procs) 
{
	key_data_t key_to_assign;
	key_to_assign.key_id = -1;
	int num_procs_calc = 0;
	int *procs_calc = NULL; /* Array containing the id of the proccesses that are calculating a key*/
	int i = 0;

	/* Flags */
	int keys_without_procs_flag = 0;

	/* MPI Datatypes to create ---> Structs */
	MPI_Datatype MPI_DECRYPT_MSG_T;
	MPI_Datatype MPI_DATA_MSG_T;

	/* Messages */
	msg_decrypt_t decrypt_msg;
	msg_data_t data_msg;
	int request_data_msg = REQUEST_DATA_MESSAGE_TAG;

	/* MPI additional parameters for Send an Recv */
	MPI_Request request;
	MPI_Status status;

	/* ======================  CHECKING PARAMETERS ====================== */

	if (-1 == proc_id) {
		fprintf(stderr, "%s\n", "assign_key_to_proccess: procces_id is -1");
		return -1;
	}

	if (0 == num_keys) {
		fprintf(stderr, "%s\n", "assign_key_to_proccess: num_keys is 0");
		return -1;
	}

	if (0 == num_procs) {
		fprintf(stderr, "%s\n", "assign_key_to_proccess: num_procs is 0");
		return -1;
	}

	if (NULL == (procs_calc = malloc(num_procs * sizeof(int)))) { 
		fprintf(stderr, "%s\n", "assign_key_to_proccess: ERROR in malloc(procs_calc)");
		return -1;
	}

	for(i = 0; i < num_procs ; i++) procs_calc[i] = -1;//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< This could be better done

	/* Although its main purpose is search keys with the less number of procceses, it also works for what we want here */
	/* Which is finding keys with 0 proccesses asigned */
	keys_without_procs_flag = search_keys_with_min_num_of_procs(k_table, num_keys, &num_procs_calc, &procs_calc, &key_to_assign);
		
    if (-1 == procs_calc[0]) { /* DEBUG */
		fprintf(stderr, "%s\n", "assign_key_to_proccess: ERROR in proc_calc[0]");
		return -1;
	}

	if (1 == keys_without_procs_flag) {

		if (-1 == construct_decrypt_msg(num_keys, &decrypt_msg, &MPI_DECRYPT_MSG_T)) {
			fprintf(stderr, "%s\n", "assign_key_to_proccess: construct_decrypt_msg");
			return -1;
		}

		if (MPI_SUCCESS != MPI_Send(&decrypt_msg, 1, MPI_DECRYPT_MSG_T, proc_id, DECRYPT_MESSAGE_TAG, MPI_COMM_WORLD) ) { /* VOID MESSAGE */
			fprintf(stderr, "%s\n", "assign_key_to_proccess: ERROR in MPI_Send (1)");
			return -1;
		}

		/* Register the proccess calculating the key */
		if (-1 == register_proccess_key_table(proc_id, key_to_assign.key_id, k_table) ){
			fprintf(stderr, "%s\n", "assign_key_to_proccess: ERROR in register_proccess_key_table");
			return -1;
		}

	} else { // All the keys have been asigned

		for (i = 0; i < num_procs_calc; i++) {

			if (MPI_SUCCESS != MPI_Send(&request_data_msg, 1, MPI_INT, procs_calc[i], REQUEST_DATA_MESSAGE_TAG, MPI_COMM_WORLD) ) {
				fprintf(stderr, "%s\n", "assign_key_to_proccess: ERROR in MPI_Send (2)");
				return -1;
			}

		}

		for (i = 0; i < num_procs_calc; i++) {

			if (MPI_SUCCESS != MPI_Recv(&data_msg , 1, MPI_DATA_MSG_T, procs_calc[i], DATA_MESSAGE_TAG, MPI_COMM_WORLD, &status) ) {
				fprintf(stderr, "%s\n", "assign_key_to_proccess: ERROR in MPI_Recv ");
				return -1;
			}

		}

		/* Store data from message *///<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< NOT DONE YET
		
		/* Notify the procceses that their key is going to be calculated by a new proccess */ //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< NOT DONE YET
		for (i = 0; i < num_procs_calc; i++) {

			if (MPI_SUCCESS != MPI_Send(&decrypt_msg, 1, MPI_DECRYPT_MSG_T, procs_calc[i], DECRYPT_MESSAGE_TAG, MPI_COMM_WORLD) ) { /* VOID MESSAGE */
				fprintf(stderr, "%s\n", "assign_key_to_proccess: ERROR in MPI_Send (3)");
				return -1;
			}

		}

		/* Send the key to the new proccess */
		if (MPI_SUCCESS != MPI_Send(&decrypt_msg, 1, MPI_DECRYPT_MSG_T,proc_id, DECRYPT_MESSAGE_TAG, MPI_COMM_WORLD) ) { /* VOID MESSAGE */
			fprintf(stderr, "%s\n", "assign_key_to_proccess: ERROR in MPI_Send (3)");
			return -1;
		}

	}
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

	srand(time(NULL));

	new_key.key_id = id;
	new_key.key =  MIN + rand() % (MAX - MIN);
	strcpy(new_key.cypher, key_encrypter(new_key.key));

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
	unsigned char* p = (unsigned char*)&key; //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< I don´t know why this is warning
	return crypt(p, "aa");
}

/***************************************
 *  key_decrypter                      * 
 ***************************************
 *                                     *
 *  Decrypts the crypted combination   *
 *  to get the original key            *
 *                                     *
 *  Return: if the key is found 0      *
 			Otherwise, returns 1       *
 ***************************************/
int key_decrypter(msg_decrypt_t msg, clock_t* end, int* key_found)
{
	char decrypt_string[CRYPT_LENGTH];

	srand(1);

	sprintf(decrypt_string, "%08ld", rand() % (msg.max_value) );
	if (0 == strcmp(crypt(decrypt_string, "aa"), msg.key.cypher) ) {
		*end = clock();
		*key_found = 1;
		printf("Encontrada: %s->%s \n", msg.key.cypher, decrypt_string);/* DEBUG */
		return 0;
	}

	return 1; //Not found
}

/***************************************
 *  initialice_table_of_keys           * 
 ***************************************
 *                                     *
 *  Initialice both the keys´          *
 *  and procces´ table                 *
 *                                     *
 *  Return: in case of error -1        *
 *			Otherwise, returns 1       *
 ***************************************/
int initialice_table_of_keys(key_table_t k_table[], proc_table_t p_table[], int n_proc, int num_keys) 
{
	int i, j;

	/* ======================  CHECKING PARAMETERS ====================== */

	if (0 == n_proc) {
		fprintf(stderr, "%s\n", "initialice_table_of_keys: n_proc is 0");
		return -1;
	}

	if (0 == num_keys) {
		fprintf(stderr, "%s\n", "initialice_table_of_keys: num_keys is 0");
		return -1;
	}

	/* KEY TABLE */
	for (i = 0; i < num_keys; i++) {

		k_table[i]->key_id = i;
		k_table[i]->key = key_generator(i);
		k_table[i]->decrypted_flag = 0;
		k_table[i]->assigned_flag = 0;
		k_table[i]->num_procs_list = 0;

		if (NULL == (k_table[i]->procs = malloc(n_proc * sizeof(int)))){ /*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< I dont´know if this is ok*/
			fprintf(stderr, "%s\n", "initialice_table_of_keys: ERROR in malloc(procs)");
			return -1;
		}

		for(j = 0; j < n_proc; j++) k_table[i]->procs[j] = -1; /* Ids of procceses working on that key */
	}

	/* PROCCESSES TABLE */
	for (i = 0; i < n_proc; i++) {
			
		p_table[i]->proc_id = j;
		p_table[i]->occupied_flag = 0;

		/* STATISTICS */
		p_table[i]->stats.n_keys = num_keys;
		p_table[i]->stats.n_rand_crypt_calls = 0;

		if (NULL == (p_table[i]->stats.key_proccesing_times = malloc(num_keys * sizeof(double)))){ /*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< I dont´know if this is ok*/
			fprintf(stderr, "%s\n", "initialice_table_of_keys: ERROR in malloc(key_proccesing_times)");
			return -1;
		}

		for(j = 0; j < num_keys; j++) p_table[i]->stats.key_proccesing_times[j] = -1;

	}

	return 1;
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
int search_free_procs(proc_table_t p_table[], int num_proc, int* proc_id) 
{	
	/* ======================  CHECKING PARAMETERS ====================== */

	if (0 == num_proc) {
		fprintf(stderr, "%s\n", "search_free_procs: num_proc is 0");
		return -1;
	}

	for (int i = 0 ; i < num_proc; i++) {
		if (p_table[i]->occupied_flag == 0){ 
			*proc_id = i;
			return 1;
		}
	}

	return 0; // All the proccesses are occupied
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
 *          is found returns 1.        *
 *			Otherwise, returns 0       *
 ***************************************/
int are_there_keys_not_decrypted(key_table_t k_table[], int num_keys) 
{

	/* ======================  CHECKING PARAMETERS ====================== */

	if (0 == num_keys) {
		fprintf(stderr, "%s\n", "are_there_keys_not_decrypted: num_keys is 0");
		return -1;
	}

	for (int i = 0; i < num_keys; i++) {
		if (k_table[i]->decrypted_flag == 0)  
			return 1;
	}

	return 0; // All the keys have been decrypted
}

/****************************************
 *  search_keys_with_min_num_of_procs   * 
 ****************************************
 *                                      *
 *  Searchs for the key that is been    *
 *	decrypted by the minor number of    *
 *	proccesses                          *
 *                                      *
 *  Return: if a key with no procceses  *
 *			is found returns 1          *
 *			Otherwise, returns 0        *
 ****************************************/
int search_keys_with_min_num_of_procs(key_table_t k_table[], int num_keys, int* num_procs, int** procs_calc, key_data_t *key)
{
	int i = 0;
	int key_id = -1;
	*num_procs = k_table[0]->num_procs_list;

	/* ======================  CHECKING PARAMETERS ====================== */

	if (0 == num_keys) {
		fprintf(stderr, "%s\n", "search_keys_with_min_num_of_procs: num_keys is 0");
		return -1;
	}

	for (i = 0 ; i < num_keys; i++) {

		/* If there are 2 or more keys with the same number of proccesses, p.e. 0 procceses, we get the first one we picked. */
		/* That´s why we put > and not >= */
		if (*num_procs > k_table[i]->num_procs_list) { 
			*num_procs = k_table[i]->num_procs_list;
			*key = k_table[i]->key;	
			key_id = i;
		}

	}

	/* Get the list of proccesses calculating the key */
	for (i = 0 ; i < k_table[i]->num_procs_list; i++)
		*procs_calc[i] = k_table[key_id]->procs[i];
		//*procs_calc = k_table[key_id].procs[i];
		//(procs_calc)++;//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

	if (0 == (*num_procs) ) 
		return 1;
	else 
		return 0;
}

/***************************************
 *  register_proccess_key_table  * 
 ***************************************
 *                                     *
 *  Adds a new proccess to the list    *
 *	of proccess doing a specific key   *
 *                                     *
 *  Return: in case of error -1        *
 *			Otherwise, returns 1       *
 ***************************************/
int register_proccess_key_table(int proc_id, int key_id, key_table_t k_table[])
{
	int num_procs_list = 0;

	/* ======================  CHECKING PARAMETERS ====================== */

	if (-1 == proc_id) {
		fprintf(stderr, "%s\n", "register_proccess_key_table: key_id is -1");
		return -1;
	}

	if (-1 == key_id) {
		fprintf(stderr, "%s\n", "register_proccess_key_table: key_id is -1");
		return -1;
	}

	num_procs_list = k_table[key_id]->num_procs_list;
	k_table[key_id]->procs[num_procs_list] = proc_id;

	(k_table[key_id]->num_procs_list)++;

	return 1;
}

/***************************************
 *  construct_type_msg                 * 
 ***************************************
 *                                     *
 *  Generates the struct containing    *
 *  the key and the size of the key    *
 *  in MPI datatypes                   *
 *                                     *
 *  Return: in case of error -1        *
 *			Otherwise, returns 1       *
 ***************************************/
int construct_key_type(int num_keys, key_data_t* data, MPI_Datatype* MPI_Type) 
{
	MPI_Datatype types[N_KEY_ELEMENTS];
	int lengths[N_KEY_ELEMENTS];
	MPI_Aint memory_address[N_KEY_ELEMENTS + 1];
	MPI_Aint memory_address_distances[N_KEY_ELEMENTS];

	/* ======================  CHECKING PARAMETERS ====================== */

	if (0 == num_keys) {
		fprintf(stderr, "%s\n", "construct_key_type: num_keys is 0");
		return -1;
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
	MPI_Address(&(data->key), &memory_address[3]);
	MPI_Address(&(data->cypher), &memory_address[4]);

	memory_address_distances[0] = memory_address[1] - memory_address[0];
	memory_address_distances[1] = memory_address[2] - memory_address[0];
	memory_address_distances[2] = memory_address[3] - memory_address[0];
	memory_address_distances[3] = memory_address[4] - memory_address[0];

	/* Create struct in MPI */
	if (MPI_SUCCESS != MPI_Type_struct(N_KEY_ELEMENTS, lengths, memory_address_distances, types, MPI_Type) ) {
		fprintf(stderr, "%s\n", "construct_key_type: ERROR in MPI_Type_struct");
		return -1;
	}

	/* Certificate the struct called pMPI_new_data_type before being used */
	if (MPI_SUCCESS != MPI_Type_commit(MPI_Type) ) {
		fprintf(stderr, "%s\n", "construct_key_type: ERROR in MPI_Type_Commit (2)");
		return -1;
	}

	return 1;
}

/***************************************
 *  construct_decrypt_msg              * 
 ***************************************
 *                                     *
 *  Generates the struct containing    *
 *  the key and the size of the key    *
 *  in MPI datatypes                   *
 *                                     *
 *  Return: in case of error -1        *
 			Otherwise, returns 1       *
 ***************************************/
int construct_decrypt_msg(int num_keys, msg_decrypt_t* data, MPI_Datatype* MPI_Type)  
{
	MPI_Datatype types[N_DECRYPT_MESSAGE_ELEMENTS];
	int lengths[N_DECRYPT_MESSAGE_ELEMENTS];
	MPI_Aint memory_address[N_DECRYPT_MESSAGE_ELEMENTS + 1];
	MPI_Aint memory_address_distances[N_DECRYPT_MESSAGE_ELEMENTS];

	MPI_Datatype MPI_KEY_T;

	/* ======================  CHECKING PARAMETERS ====================== */

	if (0 == num_keys) {
		fprintf(stderr, "%s\n", "construct_decrypt_msg: num_keys is 0");
		return -1;
	}

	/* CONSTRUCTS pMPI_KEY_T */
	if (-1 == construct_key_type(num_keys, &(data->key), &MPI_KEY_T)) {
		fprintf(stderr, "%s\n", "construct_decrypt_msg: ERROR in construct_key_type");
		return -1;
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
		fprintf(stderr, "%s\n", "construct_decrypt_msg: ERROR in MPI_Type_struct");
		return -1;
	}

	/* Certificate the struct called pMPI_new_data_type before being used */
	if (MPI_SUCCESS != MPI_Type_commit(MPI_Type) ) {
		fprintf(stderr, "%s\n", "construct_decrypt_msg: ERROR in MPI_Type_Commit");
		return -1;
	}

	return 1;
}

/***************************************
 *  construct_data_msg                 * 
 ***************************************
 *                                     *
 *  Generates the struct containing    *
 *  the key and the size of the key    *
 *  in MPI datatypes                   *
 *                                     *
 *  Return: in case of error -1        *
 			Otherwise, returns 1       *
 ***************************************/
int construct_data_msg(int num_keys, msg_data_t* data, MPI_Datatype* MPI_Type) 
{
	MPI_Datatype types[N_DATA_MESSAGE_ELEMENTS];
	int lengths[N_DATA_MESSAGE_ELEMENTS];
	MPI_Aint memory_address[N_DATA_MESSAGE_ELEMENTS + 1];
	MPI_Aint memory_address_distances[N_DATA_MESSAGE_ELEMENTS];

	MPI_Datatype MPI_KEY_T;

	/* ======================  CHECKING PARAMETERS ====================== */

	if (0 == num_keys) {
		fprintf(stderr, "%s\n", "construct_data_msg: num_keys is 0");
		return -1;
	}

	/* CONSTRUCTS pMPI_KEY_T */
	if (-1 == construct_key_type(num_keys, &(data->key), &MPI_KEY_T)) {
		fprintf(stderr, "%s\n", "construct_data_msg: ERROR in construct_key_type");
		return -1;
	}

	/* 
	 * Indicates the types of the struct
	 *
	 * typedef struct {
			int message_id;							
			key_data_t key;							
			int proccess_id;					
			unsigned long num_tries;				
			double time;							
		} msg_data_t;
	 *
	 */
	types[0] = MPI_INT;
	types[1] = MPI_KEY_T;
	types[2] = MPI_INT;
	types[3] = MPI_UNSIGNED_LONG;
	types[4] = MPI_DOUBLE;

	/* Indicate the numbers of elements of each type */
	lengths[0] = 1;
	lengths[1] = 1;
	lengths[2] = 1;
	lengths[3] = 1;
	lengths[4] = 1;

	/* Calculate the position of the elements in the memory address regarding the beginning of the struct */
	MPI_Address(data, &memory_address[0]);
	MPI_Address(&(data->message_id), &memory_address[1]);
	MPI_Address(&(data->key), &memory_address[2]);
	MPI_Address(&(data->proccess_id), &memory_address[3]);
	MPI_Address(&(data->num_tries), &memory_address[4]);
	MPI_Address(&(data->time), &memory_address[5]);

	memory_address_distances[0] = memory_address[1] - memory_address[0];
	memory_address_distances[1] = memory_address[2] - memory_address[0];
	memory_address_distances[2] = memory_address[3] - memory_address[0];
	memory_address_distances[3] = memory_address[4] - memory_address[0];
	memory_address_distances[4] = memory_address[5] - memory_address[0];

	/* Create struct in MPI */
	if (MPI_SUCCESS != MPI_Type_struct(N_DATA_MESSAGE_ELEMENTS, lengths, memory_address_distances, types, MPI_Type) ) {
		fprintf(stderr, "%s\n", "construct_data_msg: ERROR in MPI_Type_struct");
		return -1;
	}

	/* Certificate the struct called pMPI_new_data_type before being used */
	if (MPI_SUCCESS != MPI_Type_commit(MPI_Type) ) {
		fprintf(stderr, "%s\n", "construct_data_msg: ERROR in MPI_Type_Commit");
		return -1;
	}

	return 1;
}
