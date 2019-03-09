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

int main(int argc, char **argv) 
{
	MPI_Init(&argc, &argv);

		int id = -1;
		int num_proc = 0;
		Message_type message;
		MPI_Datatype pMPI_Message_type;

		if (MPI_SUCCESS != MPI_Comm_rank(MPI_COMM_WORLD, &id)) {
			fprintf(stderr, "%s\n", "main: ERROR in MPI_Comm_rank");
		}

		if (MPI_SUCCESS != MPI_Comm_size(MPI_COMM_WORLD, &num_proc)) {
			fprintf(stderr, "%s\n", "initialice_table_of_keys: ERROR in MPI_Comm_size");
			return ;
		}

		printf("Hello, I am process nº %d\n", id);



		if (id == 0) {
			IO_process(&argv, num_proc);
		}

		calculator_process(id);

	MPI_Finalize();

	return 0;
}

/**************************************************
 *  ============= calculator_process ============ * 
/**************************************************
/**************************************************
 *  1) Wait for the key to arrive from IO process *
 *  2) Try to decrypt it                          *
 *     2.1) Found it --> notifies the rest of     *
 *			processes                             *
 *     2.2) Not Found it --> keeps trying :)      *
 *  3) Receives a new key                         *
 **************************************************/
int calculator_process(int process_id) 
{
	int sender_proccess = IO_PROCESS_ID;
	int message_id = process_id;
    int key_found = 0;
    int flag_probe = 0;
    int key_available = 0;
    int finish_execution = 0;
	msg_decrypt_t decrypt_msg;
	msg_data_t data_msg;
	unsigned long long dec_key;
	unsigned long long num_tries = 0;
	MPI_Status status;
	MPI_Request request;

	if (-1 == construct_decrypt_msg(int num_keys, &decrypt_message)) {
			fprintf(stderr, "%s\n", "calculator_process: construct_decrypt_msg");
			return -1;
	}

	if (-1 == construct_data_msg(int num_keys, &data_msg)) {
			fprintf(stderr, "%s\n", "calculator_process: construct_data_msg");
			return -1;
	}

	do{

		if (MPI_SUCCESS != MPI_Recv(decrypt_message , 1, pMPI_DECRYPT_MSG_T, sender_proccess, DECRYPT_MESSAGE, MPI_COMM_WORLD, &status) ) {
			fprintf(stderr, "%s\n", "calculator_process: ERROR in MPI_Recv (1)");
			return -1;
		}

		/* Get parameters from decrypt_message *///<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

		do{
			num_tries++;
			key_available = key_decrypter(decrypt_message.key, &dec_key, decrypt_message.length, &key_found);

			/* Fill data_msg with all the info to send */ //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

			if(key_found == 1){

				if (MPI_SUCCESS != MPI_Send(/* Message */ , 1, /* Type of messge*/, sender_proccess, /* ID of message */, MPI_COMM_WORLD) ) {
					fprintf(stderr, "%s\n", "calculator_process: ERROR in MPI_Send (1)");
					return -1;
				}

				key_available = 1;

				break;
			}

			if (MPI_SUCCESS != MPI_IRecv(/* MEssage */ , 1, /* Type of message */, sender_proccess, MPI_ANY_TAG, MPI_COMM_WORLD, &request) ) {
				fprintf(stderr, "%s\n", "calculator_process: ERROR in MPI_IRecv");
				return -1;
			}

			if (MPI_SUCCESS != MPI_Iprobe(sender_proccess, MPI_ANY_TAG, MPI_COMM_WORLD, &flag_probe, &status)) {
				fprintf(stderr, "%s\n", "calculator_process: ERROR in MPI_Iprobe");
				return -1;
			}

			if(flag_probe != 0){
				switch(MPI_ANY_TAG){
					case DECRYPT_MESSAGE:
						if (MPI_SUCCESS != MPI_Recv(decrypt_message , 1, pMPI_DECRYPT_MSG_T, sender_proccess, DECRYPT_MESSAGE, MPI_COMM_WORLD, &status) ) {
							fprintf(stderr, "%s\n", "calculator_process: ERROR in MPI_Recv (2)");
							return -1;
						}
					;

					case REQUEST_DATA_MESSAGE:

						if(key_found == 1){
							if (MPI_SUCCESS != MPI_Send(/* MEssage */ , 1, /* Type of message */, sender_proccess, REQUEST_DATA_MESSAGE, MPI_COMM_WORLD) ) {
								fprintf(stderr, "%s\n", "calculator_process: ERROR in MPI_Send (2)");
								return -1;
							}
						}

						key_available = 1;
						
					break;

					case FINISH_EXECUTION_MESSAGE:
						finish_execution = 1;
					break;
				}
			}

		}while(key_available == 0);

	}while(finish_execution == 0);
}

/**************************************************
 *  ================ IO_process ================  * 
/**************************************************
 *  1) Generate random keys                       *
 *  2) Encrypt the keys                           *
 *  3) Constructs the type of MPI for structs     *
 *  3) Send the keys to calculator proccesses     *
 **************************************************/
int IO_process(char *argv[], int num_proc) 
{
	int number_of_Keys = atoi(argv[1]); //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	int msg_received_flag = 0;
	key_table_t k_table[number_of_Keys]; /* Table of keys */
	proc_table_t p_table[num_proc];      /* Table of proccesses */
	MPI_Group MPI_GROUP_WORLD;           
	MPI_Group id_group;                  /* Group identifier */
	MPI_Comm comm_group;                 /* Group Communication identifier */
	MPI_Request request;

	/* ======================  CREATE TABLE OF KEYS AND PROCCESSES ====================== */

	initialice_table_of_keys(&k_table, &p_table, num_proc, number_of_Keys);

	/* ======================  CREATING TYPES OF MESSAGES ====================== */

	if(argv[1] == NULL){
		// printf("Introduce the number of keys that you want generate\n");
	}else{
		number_of_Keys = atoi(argv[1]);
	}

	/* Creates the new type of struct so it can be availabe for all the proccesses */ 
	if (-1 == construct_decrypt_msg(int num_keys, &decrypt_message)) {
		fprintf(stderr, "%s\n", "IO_process: construct_decrypt_msg");
		return -1;
	}

	/* ======================  CREATING GROUP OF COMMUNICATION ====================== */

	/* OBTIENE GRUPOS DEL COMUNICADOR MPI_COMM_WORLD */
	if (MPI_SUCCESS != MPI_Comm_group(MPI_COMM_WORLD, &MPI_GROUP_WORLD)) {
		fprintf(stderr, "%s\n", "IO_process: ERROR in MPI_Comm_group");
		return -1;
	}
	/* CREA EL NUEVO GRUPO */
	if (MPI_SUCCESS != MPI_Group_incl(MPI_GROUP_WORLD, 1, IO_PROCESS_ID, &id_group)) { // We include the IO_proccess into the group 
		fprintf(stderr, "%s\n", "IO_process: ERROR in MPI_Group_incl");
		return -1;
	} 

	if (MPI_SUCCESS != MPI_Comm_create(MPI_COMM_WORLD, id_group, &comm_group)) {  /* CREA EL NUEVO COMUNICADOR */
		fprintf(stderr, "%s\n", "IO_process: ERROR in MPI_Comm_create");
		return -1;
	}

	/* ======================  SENDING KEYS ====================== */

	while (0 != (key_to_assign = search_keys_not_assigned(k_table, number_of_Keys)) ) {

		/* Find free proccesses and assign it to them. Maybe including them into a group. */

		assign_key_to_proccess( , key_to_assign);

		/* Assign key to a proccess */
		if (MPI_SUCCESS != MPI_Send(/* MEssage */ , 1, /* Type of message */, sender_proccess, DECRYPT_MESSAGE, MPI_COMM_WORLD) ) {
			fprintf(stderr, "%s\n", "IO_process: ERROR in MPI_Send");
			return -1;
		}

		if (1 == key_to_assign) break; // No keys available
		
		/* Data reception from the calculator proccesses */
		if (MPI_SUCCESS != MPI_IRecv(/* MEssage */ , 1, /* Type of message */, MPI_ANY_SOURCE, DATA_MESSAGE, MPI_COMM_WORLD, &request) ) {
			fprintf(stderr, "%s\n", "IO_process: ERROR in MPI_IRecv");
			return -1;
		}

		if (MPI_SUCCESS != MPI_Test(request, &msg_received_flag, &status) ) {
			fprintf(stderr, "%s\n", "IO_process: ERROR in MPI_Test");
			return -1;
		}

		if (msg_received_flag) {
			/* Store dawta and assign new key to the proccess */
		}

	}

	while (search_keys_not_decrypted(k_table, number_of_Keys)) {

		/* Find free proccesses and assign it to them. Maybe including them into a group. */
		if (MPI_SUCCESS != MPI_Recv(/* MEssage */ , 1, /* Type of message */, sender_proccess, DECRYPT_MESSAGE, MPI_COMM_WORLD, &status) ) {
			fprintf(stderr, "%s\n", "IO_process: ERROR in MPI_Recv (2)");
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
key_t key_generator(int id)
{
	key_t new_key;

	srand(time(NULL));

	new_key.key_id = id;
	new_key.key =  MIN + rand() % (MAX - MIN);
	new_key.cypher =  key_encrypter(new_key.key);

	return new_key;
}

/***************************************
 *  construct_message_type             * 
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
	return crypt(key, "aa");
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
int key_decrypter(key_t key)
{
	char decrypt_string[/* Key length*/];//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

	srand(1);

	sprintf(decrypt_string,"%08d",rand()%100000000);
	if (0 == strcmp(crypt(decrypt_string, "aa"), key.cypher)) {
		//printf("Encontrada: %s->%s (%lld)\n", encrypted_key, decrypt_string, intentos);
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
 *  Return:                            *
 ***************************************/
void initialice_table_of_keys(key_table_t *k_table, proc_table_t *p_table, int num_proc, int num_keys) 
{
	int i;

	for (i = 0 ; i < num_keys ; i++) {
		k_table[i]->key_id = i;
		k_table[i]->key = key_generator(i);
		k_table[i]->decrypted_flag = 0;

		if (NULL == (k_table[i]->procs = malloc(num_proc * sizeof(int)))){ /*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< I dont´know if this is ok*/
			fprintf(stderr, "%s\n", "initialice_table_of_keys: ERROR in malloc(int)");
			return ;
		}

		for (int j = 0; j < num_proc ; j++) {
			k_table[i]->procs[j] = -1; /* Ids of procceses working on that key */
		}
	}

	for (i = 0 ; i < num_proc ; i++) {
		p_table[i]->proc_id = i;
		p_table[i]->stats = NULL;
	}
}

/***************************************
 *  search_keys_not_assigned           * 
 ***************************************
 *                                     *
 *  Searchs for any key that is not    *
 *  assigned to a proccess             *
 *  in the keys´ table                 *
 *                                     *
 *  Return: if a key with no proccess  *
 *          assigned is found          *
 *          returns the id of the key. *
 *			Otherwise, returns 0       *
 ***************************************/
int search_keys_not_assigned(key_table_t k_table, int num_keys) 
{
	for (int i = 0 ; i < num_keys; i++) {
		if (k_table[i].proc[0] == -1) // The first position of the ids of procs is empty 
			return i;
	}

	return 0; // All the keys have been asigned to any proccess
}

/***************************************
 *  search_keys_not_decrypted          * 
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
int search_keys_not_decrypted(key_table_t k_table, int num_keys) 
{
	for (int i = 0; i < num_keys; i++) {
		if (k_table[i].decrypted_flag == 0)  
			return 1;
	}

	return 0; // All the keys have been decrypted
}

/***************************************
 *  assign_key_to_proccess             * 
 ***************************************
 *                                     *
 *  Assigns a key to a specific        *
 *  proccess at the beginning of       *
 *  the execution.                     *
 *                                     *
 *  Return:                            *
 ***************************************/
void assign_key_to_proccess(int id, key_table_t k_table, int num_keys) 
{

	int key_to_assign;

	if (0 != (key_to_assign = search_keys_not_assigned(k_table, number_of_Keys))) {

		/* Construct message */

		if (MPI_SUCCESS != MPI_Send(/* MEssage */ , 1, /* Type of message */, proccess_id, DECRYPT_MESSAGE, MPI_COMM_WORLD) ) {
			fprintf(stderr, "%s\n", "IO_process: ERROR in MPI_Send");
			return -1;
		}

	} else { // All the keys have been asigned

	}
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
 			Otherwise, returns 1       *
 ***************************************/
int construct_key_type(int num_keys, key_t* data) 
{
	MPI_Datatype types[N_KEY_ELEMENTS];
	int lengths[N_KEY_ELEMENTS];
	MPI_Aint memory_address[N_KEY_ELEMENTS + 1];
	MPI_Aint memory_address_distances[N_KEY_ELEMENTS];

	MPI_Datatype pMPI_CHAR_ARRAY; /* char * type in MPI */
	MPI_Datatype* pMPI_KEY_T; // THE NEW TYPE IN MPI

	/* 
	 * Indicates the types of the struct
	 *
	 * typedef struct {
	 *		int key_id;
	 *		unsigned long key;
	 *		char* cypher;
	 *	} key_t;
	 *
	 */

    /* Create array of char or char* in MPI to store the encrypted keys */
	if (MPI_SUCCESS != MPI_Type_vector(num_keys, 1, num_keys, MPI_CHAR, &pMPI_CHAR_ARRAY) ) {
		fprintf(stderr, "%s\n", "construct_key_type: ERROR in MPI_Type_vector");
		return -1;
	}

	/* Certificate it before being used */
	if (MPI_SUCCESS != MPI_Type_Commit(pMPI_CHAR_ARRAY) ) {
		fprintf(stderr, "%s\n", "construct_key_type: ERROR in MPI_Type_Commit (1)");
		return -1;
	}

	types[0] = MPI_INT;
	types[1] = MPI_UNSIGNED_LONG;
	types[2] = pMPI_CHAR_ARRAY;

	/* Indicate the numbers of elements of each type */
	lengths[0] = 1;
	lengths[1] = 1;
	lengths[2] = 1;

	/* Calculate the position of the elements in the memory address regarding the beginning of the struct */
	MPI_Address(data, &memory_address[0]);
	MPI_Address(&(data->key_id), &memory_address[1]);
	MPI_Address(&(data->key), &memory_address[2]);
	MPI_Address(&(data->cypher), &memory_address[3]);

	memory_address_distances[0] = memory_address[1] - memory_address[0];
	memory_address_distances[1] = memory_address[2] - memory_address[0];
	memory_address_distances[2] = memory_address[3] - memory_address[0];

	/* Create struct in MPI */
	if (MPI_SUCCESS != MPI_Type_struct(N_KEY_ELEMENTS, lengths, memory_address_distances, types, pMPI_KEY_T) ) {
		fprintf(stderr, "%s\n", "construct_key_type: ERROR in MPI_Type_struct");
		return -1;
	}

	/* Certificate the struct called pMPI_new_data_type before being used */
	if (MPI_SUCCESS != MPI_Type_Commit(pMPI_KEY_T) ) {
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
int construct_decrypt_msg(int num_keys, msg_decrypt_t* data) 
{
	MPI_Datatype types[N_DECRYPT_MESSAGE_ELEMENTS];
	int lengths[N_DECRYPT_MESSAGE_ELEMENTS];
	MPI_Aint memory_address[N_DECRYPT_MESSAGE_ELEMENTS + 1];
	MPI_Aint memory_address_distances[N_DECRYPT_MESSAGE_ELEMENTS];

	/* CONSTRUCTS pMPI_KEY_T */
	if (-1 == construct_key_type(num_keys, data->key)) {
		fprintf(stderr, "%s\n", "construct_decrypt_msg: ERROR in construct_key_type");
		return -1;
	}

	MPI_Datatype* pMPI_DECRYPT_MSG_T; // THE NEW TYPE IN MPI

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
	types[0] = pMPI_KEY_T;
	types[1] = MPI_INT;
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
	if (MPI_SUCCESS != MPI_Type_struct(N_DECRYPT_MESSAGE_ELEMENTS, lengths, memory_address_distances, types, pMPI_DECRYPT_MSG_T) ) {
		fprintf(stderr, "%s\n", "construct_decrypt_msg: ERROR in MPI_Type_struct");
		return -1;
	}

	/* Certificate the struct called pMPI_new_data_type before being used */
	if (MPI_SUCCESS != MPI_Type_Commit(pMPI_DECRYPT_MSG_T) ) {
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
int construct_data_msg(int num_keys, msg_data_t* data) 
{
	MPI_Datatype types[N_DATA_MESSAGE_ELEMENTS];
	int lengths[N_DATA_MESSAGE_ELEMENTS];
	MPI_Aint memory_address[N_DATA_MESSAGE_ELEMENTS + 1];
	MPI_Aint memory_address_distances[N_DATA_MESSAGE_ELEMENTS];

	/* CONSTRUCTS pMPI_KEY_T */
	if (-1 == construct_key_type(num_keys, data->key)) {
		fprintf(stderr, "%s\n", "construct_data_msg: ERROR in construct_key_type");
		return -1;
	}

	MPI_Datatype* pMPI_DATA_MSG_T; // THE NEW TYPE IN MPI

	/* 
	 * Indicates the types of the struct
	 *
	 * typedef struct {
			int message_id;
			Key_type key;
			int process_id;
			int length;
			unsigned long num_tries;
			double time;
		} msg_data_t;
	 *
	 */
	types[0] = MPI_INT;
	types[1] = pMPI_KEY_T;
	types[2] = MPI_INT;
	types[3] = MPI_INT;
	types[4] = MPI_UNSIGNED_LONG;
	types[5] = MPI_DOUBLE;

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
	MPI_Address(&(data->process_id), &memory_address[3]);
	MPI_Address(&(data->length), &memory_address[4]);
	MPI_Address(&(data->num_tries), &memory_address[5]);
	MPI_Address(&(data->time), &memory_address[6]);

	memory_address_distances[0] = memory_address[1] - memory_address[0];
	memory_address_distances[1] = memory_address[2] - memory_address[0];
	memory_address_distances[2] = memory_address[3] - memory_address[0];
	memory_address_distances[3] = memory_address[4] - memory_address[0];
	memory_address_distances[4] = memory_address[5] - memory_address[0];
	memory_address_distances[5] = memory_address[6] - memory_address[0];

	/* Create struct in MPI */
	if (MPI_SUCCESS != MPI_Type_struct(N_DATA_MESSAGE_ELEMENTS, lengths, memory_address_distances, types, pMPI_DATA_MSG_T) ) {
		fprintf(stderr, "%s\n", "construct_data_msg: ERROR in MPI_Type_struct");
		return -1;
	}

	/* Certificate the struct called pMPI_new_data_type before being used */
	if (MPI_SUCCESS != MPI_Type_Commit(pMPI_DATA_MSG_T) ) {
		fprintf(stderr, "%s\n", "construct_data_msg: ERROR in MPI_Type_Commit");
		return -1;
	}

	return 1;
}