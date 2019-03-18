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
#include <unistd.h>
#include <ctype.h>

/*
 * =================== SPECIAL COMMENTS ===================
 *
 *  > DEGUG means it´s only a fancy message to test something
 *  > VOID MESSAGE means we are sending a message that is not filled yet.
 *  > NOT DONE YET, task not implemented yet
 *
 *  ** TO FIND ANY OF THESE IN THE CODE USE CTRL+F **
 * 
 *  Look for CHECK to keep fixing
 *
 *
 *  Also to compile : mpicc crypting.c crypting.h -lcrypt -o crypt
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
        
        if (-1 == register_key_type(&key_data, &MPI_KEY_T)) {
		    fprintf(stderr, "%s\n", "IO_proccess: ERROR in register_key_type");
		    return -1;
	    }
	    
	    if (-1 == register_decrypt_msg(&decrypt_msg, &MPI_DECRYPT_MSG_T)) {
		    fprintf(stderr, "%s\n", "IO_proccess: ERROR in register_decrypt_msg");
		    return -1;
	    }
	    
	    if (-1 == register_data_msg(&data_msg, &MPI_DATA_MSG_T)) {
		    fprintf(stderr, "%s\n", "IO_proccess: ERROR in register_data_msg");
		    return -1;
	    }
	    
	    /* ======================  EXECUTION ====================== */

		if (MPI_SUCCESS != MPI_Comm_rank(MPI_COMM_WORLD, &id)) {
			fprintf(stderr, "%s\n", "main: ERROR in MPI_Comm_rank");
		}

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
	msg_decrypt_t decrypt_msg;
	MPI_Status status;
	MPI_Request request;
	
	if (MPI_SUCCESS != MPI_Recv(&decrypt_msg, 1, MPI_DECRYPT_MSG_T, 0, DECRYPT_MESSAGE_TAG, MPI_COMM_WORLD, &status) ) {
		fprintf(stderr, "%s\n", "calculator_proccess: ERROR in MPI_Recv");
		return -1;
	}
	
    printf("ID: %d | Key ID: %d | Cipher: %s | Key: %lu | Min: %lu | Max: %lu\n",
            decrypt_msg.message_id,
            decrypt_msg.key.key_id,
            decrypt_msg.key.cypher,
            decrypt_msg.key.key_number,
            decrypt_msg.min_value,
            decrypt_msg.max_value
    );

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
int IO_proccess(char *argv[]) 
{
    /*
    PROCEDURE Proceso_central {
    
        ...
        
        WHILE (Claves sin procesos asociados) {
            
            IF (Hay_procesos_libres) {
                p ← Seleccionar_proceso_libre()
                Asignar_clave(p)
                IF (No quedan claves sin procesos asociados)
                    BREAK; // NO quedan claves libres
                Mensaje ← Recibir_mensaje_no_bloqueante()
            } ELSE {
                Mensaje ← Recibir_mensaje_bloqueante()
            }
            
            IF (Hay mensaje) { // Solo puede ser de datos y debido a clave encontrada
                Guardar_datos(Mensaje)
                Asignar _clave(Mensaje.proceso)
            }
        }
        
        WHILE (Queden claves) { // Este bucle WHILE creo que es muy optimizable en el reparto de
                                // tareas
                                
            IF (No hay procesos libres)
                Mensaje ← Recibir_mensaje_bloqueante()  // Solo puede ser de datos y
                                                        // debido a clave encontrada
            Datos ← Guardar_datos(Mensaje)
            
            FOR p IN procesos_calculando_clave_encontrada {
                Enviar_mensaje_solicitud_datos(q)
            }
            
            FOR n IN numero_de_procesos_calculando_clave {
                Datos ← Recibir_respuesta_bloqueante()
            }
            
            IF (Quedan claves) {
                FOR p IN procesos_calculando_clave_encontrada
                    Asignar _clave(p)
            } ELSE
                BREAK; // NO quedan claves
            }
            
            FOR p IN procesos_calculadores_en_el_sistema
                Enviar_mensaje_finalizar_ejecucion()
                
            // Cálculos finalizados. Falta mostrar estadísticas, etc.
        }
    */


	int num_keys = 0;
	int i = 0;
	int num_procs = 0;
	int proc_id = -1;
	int k_id = -1;
	int key_left_id = -1;

	key_data_t key_to_assign;
	key_to_assign.key_id = -1;

	/* Tables */
	key_table_t k_table; 		 // Table of keys 
	proc_table_t p_table;        // Table of proccesses 

	/* Messages */
	msg_decrypt_t decrypt_msg;
	msg_data_t data_msg;
	int finish_exec_msg = FINISH_EXECUTION_MESSAGE_TAG;
	int request_data_msg = REQUEST_DATA_MESSAGE_TAG;

	/* Flags*/
	int msg_received_flag = -1;
	int free_procs_flag = -1;

	/* MPI stuff to create the group */
	MPI_Group MPI_GROUP_WORLD;           
	MPI_Group id_group;                  /* Group identifier */
	MPI_Comm comm_group;                 /* Group Communication identifier */
	MPI_Status status;

	/* MPI Datatypes to create ---> Structs */

	/* Data message reception */
	int num_procs_key = 0, key_id_rcv = -1, proc_id_rcv = -1;
	
	if(argv[1] == NULL) {
	    num_keys = DEFAULT_NUM_KEYS; /* DEBUG */
	    printf("(IO_proccess) No argv. So num_keys will be %d\n", num_keys); /* DEBUG */
    } else {
	    num_keys = atoi(argv[1]);
    }

	if (MPI_SUCCESS != MPI_Comm_size(MPI_COMM_WORLD, &num_procs)) {
		fprintf(stderr, "%s\n", "IO_proccess: ERROR in MPI_Comm_size");
		return -1;
	}

	/* ======================  CREATE TABLE OF KEYS AND PROCCESSES ====================== */

	if (NULL == (k_table = (struct key_table_row *) malloc(num_keys * sizeof(struct key_table_row)))){ 
		fprintf(stderr, "%s\n", "IO_proccess: ERROR in malloc(k_table)");
		return -1;
	}

	if (NULL == (p_table = (struct proc_table_row *) malloc(num_procs * sizeof(struct proc_table_row)))){ 
		fprintf(stderr, "%s\n", "IO_proccess: ERROR in malloc(p_table)");
		return -1;
	}

	if (-1 == initialize_tables(k_table, p_table, num_procs, num_keys)) { 
		fprintf(stderr, "%s\n", "IO_proccess: ERROR in initialize_tables");
		return -1;
	}
	
	/* DEBUG *///<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	/*
	int _nprocs;
	int _procs[100];
	key_data_t key;
	search_keys_with_min_num_of_procs(k_table, num_keys, &_nprocs, _procs, &key);
	
	print_key_table(k_table, num_keys);
	printf("\n\nKey ID: %d | Num procs: %d\n", key.key_id, _nprocs);

	data_msg.proccess_id = 1;
	data_msg.key.key_id = key.key_id;
	data_msg.key.key_number = key.key_number;
	strcpy(data_msg.key.cypher, key.cypher);
	data_msg.time = 45000.00;
	data_msg.num_tries = 30000000000000000;

	print_proc_table(p_table, num_procs - 1);
	store_data(p_table, data_msg, num_procs);
	print_proc_table(p_table, num_procs - 1);

	printf("\n\n\n================== BEFORE ==================\n");
	print_key_table(k_table, num_keys);
	print_proc_table(p_table, num_procs - 1);
	register_proccess_and_key_table(1, key.key_id, k_table, p_table);
	printf("\n\n\n================== AFTER ==================\n");
	print_key_table(k_table, num_keys);
	print_proc_table(p_table, num_procs - 1);

	exit(1);
	*/
	/* END OF DEBUG */
	
	/* ======================  SENDING KEYS ====================== */

	/* ============================================  THERE ARE KEYS LEFT TO GIVE TO THE PROCCESSES ============================================ */
	key_left_id = are_there_keys_not_decrypted(k_table, num_keys);
	if (-2 == key_left_id) {
	    fprintf(stderr, "%s\n", "IO_proccess: ERROR in are_there_keys_not_decrypted");
		return -1;
	}
	
	while (-1 != key_left_id) {
	
	    /*
	    WHILE (Claves sin procesos asociados) {
            
            IF (Hay_procesos_libres) {
                p ← Seleccionar_proceso_libre()
                Asignar_clave(p)
                IF (No quedan claves sin procesos asociados)
                    BREAK; // NO quedan claves libres
                Mensaje ← Recibir_mensaje_no_bloqueante()
            } ELSE {
                Mensaje ← Recibir_mensaje_bloqueante()
            }
            
            IF (Hay mensaje) { // Solo puede ser de datos y debido a clave encontrada
                Guardar_datos(Mensaje)
                Asignar _clave(Mensaje.proceso)
            }
        }
        */
	
	    /* Find free proccesses and assign it to them */
	    free_procs_flag = search_free_procs(p_table, num_procs, &proc_id);

	    /*
		if (-1 == free_procs_flag) { // Error
			fprintf(stderr, "%s\n", "IO_proccess: ERROR in search_free_procs");
			return -1;
		} else if (1 == free_procs_flag) { // All processes are busy
		    if (MPI_SUCCESS != MPI_Recv(&data_msg, 1, MPI_DATA_MSG_T, IO_PROCESS_ID, DATA_MESSAGE_TAG, MPI_COMM_WORLD, &status) ) {
			    fprintf(stderr, "%s\n", "IO_process: ERROR in MPI_Recv (1)");
			    return -1;
		    }
		} else { // There are some free processes
		    /* Assign a key to a proccess 
		    
		    if (-1 == assign_key_to_proccess(proc_id, k_table, p_table, num_keys, num_procs)) {
			    fprintf(stderr, "%s\n", "IO_proccess: ERROR in assign_key_to_proccess (1)");
			    return -1;
		    }
		    
		    
		    THIS MUST BE CHECKED PLUS ASSIGNING KEYS ** CHECK **
		    
		    int search_keys_with_min_num_of_procs(key_table_t k_table, int num_keys, int* num_procs, int* procs_calc, key_data_t *key);
            int store_data(proc_table_t p_table, msg_data_t data_msg, int num_procs);
            int register_proccess_and_key_table(int proc_id, int key_id, key_table_t k_table, proc_table_t p_table);
            
            
		}
		*/
	}
	
	return 0;
}

int print_key_table(key_table_t key_table, int nkeys) {
    
    int i;
    char format[] = "ID: %d | Cipher: %s | Key: %lu | Decrypted: %s | Assigned: %s | NumProcsAssigned %d\n";
    
    if (NULL == key_table || nkeys < 0)
        return -1;
    
    printf("%s\n\n", "Keys' table is:");
    for (i = 0; i < nkeys; ++i) {
        printf(format,
                key_table[i].key_id,
                key_table[i].key.cypher,
                key_table[i].key.key_number,
                key_table[i].decrypted_flag ? "TRUE" : "FALSE",
                key_table[i].assigned_flag ? "TRUE" : "FALSE",
                key_table[i].num_procs_list
        );
    }
    
    return 0;
}

int print_proc_table(proc_table_t proc_table, int nprocs) {

    int i;
    char format[] = "Proc_id: %d | Occupied: %s | Keys found: %d | Calls to rand/crypt: %lu\n";
    
    if (NULL == proc_table || nprocs < 0)
        return -1;
    
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

/***************************************
 *  initialize_tables           * 
 ***************************************
 *                                     *
 *  initialize both the keys´          *
 *  and procces´ table                 *
 *                                     *
 *  Return: in case of error -1        *
 *			Otherwise, returns 1       *
 ***************************************
 *               TESTED                * 
 ***************************************/
int initialize_tables(key_table_t k_table, proc_table_t p_table, int n_proc, int num_keys) 
{
	int i, j;

	/* ======================  CHECKING PARAMETERS ====================== */

	if (0 >= n_proc) {
		fprintf(stderr, "%s\n", "initialize_tables: n_proc is 0 or less");
		return -1;
	}

	if (0 >= num_keys) {
		fprintf(stderr, "%s\n", "initialize_tables: num_keys is 0 or less");
		return -1;
	}

	/* KEY TABLE */
	for (i = 0; i < num_keys; i++) {

		k_table[i].key_id = i;
		k_table[i].key = key_generator(i);
		k_table[i].decrypted_flag = 0;
		k_table[i].assigned_flag = 0;
		k_table[i].num_procs_list = 0;

		if (NULL == (k_table[i].procs = malloc(n_proc * sizeof(int)))){ 
			fprintf(stderr, "%s\n", "initialize_tables: ERROR in malloc(procs)");
			return -1;
		}

		for(j = 0; j < n_proc; j++) k_table[i].procs[j] = -1; /* Ids of procceses working on that key */
	}


	/* PROCCESSES TABLE */
	for (i = 0; i < (n_proc - 1) ; i++) { /* ONLY CALCULATORS */
			
		p_table[i].proc_id = i + 1; /* Because proccess 0 is IO proccess. SO it starts in 1*/
		p_table[i].occupied_flag = 0;

		/* STATISTICS */
		p_table[i].stats.n_keys = num_keys;
		p_table[i].stats.n_rand_crypt_calls = 0;

		if (NULL == (p_table[i].stats.key_proccesing_times = malloc(num_keys * sizeof(double)))){ 
			fprintf(stderr, "%s\n", "initialize_tables: ERROR in malloc(key_proccesing_times)");
			return -1;
		}

		for(j = 0; j < num_keys; j++) p_table[i].stats.key_proccesing_times[j] = -1;

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
 ***************************************
 *               TESTED                * 
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
	// new_key.cypher[CRYPT_LENGTH] = '\0'; This should not be needed. Look in crypting.h. You fucked by not taking \0 into account
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
 ***************************************
 *               TESTED                * 
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
 ***************************************
 *               TESTED                * 
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
		return -1;
	} else if (NULL == MPI_Type) {
	    fprintf(stderr, "%s\n", "register_key_type: MPI_Type cannot be NULL");
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
	MPI_Address(&(data->key_number), &memory_address[3]);
	MPI_Address(&(data->cypher), &memory_address[4]);

	memory_address_distances[0] = memory_address[1] - memory_address[0];
	memory_address_distances[1] = memory_address[2] - memory_address[0];
	memory_address_distances[2] = memory_address[3] - memory_address[0];
	memory_address_distances[3] = memory_address[4] - memory_address[0];

	/* Create struct in MPI */
	if (MPI_SUCCESS != MPI_Type_struct(N_KEY_ELEMENTS, lengths, memory_address_distances, types, MPI_Type) ) {
		fprintf(stderr, "%s\n", "register_key_type: ERROR in MPI_Type_struct");
		return -1;
	}

	/* Certificate the struct called pMPI_new_data_type before being used */
	if (MPI_SUCCESS != MPI_Type_commit(MPI_Type) ) {
		fprintf(stderr, "%s\n", "register_key_type: ERROR in MPI_Type_Commit (2)");
		return -1;
	}

	return 1;
}

/***************************************
 *  register_decrypt_msg              * 
 ***************************************
 *                                     *
 *  Generates the struct containing    *
 *  the key and the size of the key    *
 *  in MPI datatypes                   *
 *                                     *
 *  Return: in case of error -1        *
 			Otherwise, returns 1       *
 ***************************************
 *               TESTED                * 
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
		return -1;
	} else if (NULL == MPI_Type) {
	    fprintf(stderr, "%s\n", "register_decrypt_msg: MPI_Type cannot be NULL");
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
		fprintf(stderr, "%s\n", "register_decrypt_msg: ERROR in MPI_Type_struct");
		return -1;
	}

	/* Certificate the struct called pMPI_new_data_type before being used */
	if (MPI_SUCCESS != MPI_Type_commit(MPI_Type) ) {
		fprintf(stderr, "%s\n", "register_decrypt_msg: ERROR in MPI_Type_Commit");
		return -1;
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
 ***************************************
 *               TESTED                * 
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
		return -1;
	} else if (NULL == MPI_Type) {
	    fprintf(stderr, "%s\n", "register_decrypt_msg: MPI_Type cannot be NULL");
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
		fprintf(stderr, "%s\n", "register_data_msg: ERROR in MPI_Type_struct");
		return -1;
	}

	/* Certificate the struct called pMPI_new_data_type before being used */
	if (MPI_SUCCESS != MPI_Type_commit(MPI_Type) ) {
		fprintf(stderr, "%s\n", "register_data_msg: ERROR in MPI_Type_Commit");
		return -1;
	}

	return 1;
}

/***************************************
 * fill_decrypt_msg                    * 
 ***************************************
 *                                     *
 *  Fills the data_msg_t we'll send    *
 *                                     *
 *                                     *
 *                                     *
 *  Return: in case of error -1        *
 			Otherwise, returns 1       *
 ***************************************
 *               TESTED                * 
 ***************************************/
int fill_decrypt_msg(msg_decrypt_t *decrypt_msg, key_data_t key , unsigned long min_value, unsigned long max_value) {

    if (NULL == decrypt_msg) {
        fprintf(stderr, "%s\n", "fill_decrypt_msg: decrypt_msg cannot be NULL");
		return -1;
    }

	decrypt_msg->message_id=DECRYPT_MESSAGE_TAG;
	decrypt_msg->key=key;
	decrypt_msg->min_value=min_value;
	decrypt_msg->max_value=max_value;
	
	return 1;
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
 ***************************************
 *               TESTED                * 
 ***************************************/
int are_there_keys_not_decrypted(key_table_t k_table, int num_keys) 
{
    int i;    
    
	/* ======================  CHECKING PARAMETERS ====================== */

	if (0 >= num_keys) {
		fprintf(stderr, "%s\n", "are_there_keys_not_decrypted: num_keys is 0 or less");
		return -2;
	} else if (NULL == k_table) {
	    fprintf(stderr, "%s\n", "are_there_keys_not_decrypted: k_table cannot be NULL");
		return -2;
	}

	for (i = 0; i < num_keys; i++) {
		if (0 == k_table[i].decrypted_flag) 
			return i;
	}

	return -1; // All the keys have been decrypted
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
 ***************************************
 *               TESTED                * 
 ***************************************/
int search_free_procs(proc_table_t p_table, int num_proc, int* proccess_id) 
{	
	/* ======================  CHECKING PARAMETERS ====================== */
    int  i;
    
	if (0 >= num_proc) {
		fprintf(stderr, "%s\n", "search_free_procs: num_proc is 0 or less");
		return -1;
	} else if (NULL == p_table) {
	    fprintf(stderr, "%s\n", "search_free_procs: p_table cannot be NULL");
		return -1;
	} else if (NULL == proccess_id) {
	    fprintf(stderr, "%s\n", "search_free_procs: proccess_id cannot be NULL");
		return -1;
	}

	/* It´s n - 1 because we don´t count IO_Proccess on the table. Only calculators  */
	for (i = 0 ; i < (num_proc - 1); i++) { 
		printf("Proc number %d is occupied?: %d\n", i, p_table[i].occupied_flag); /* DEBUG */
		if (0 == p_table[i].occupied_flag){ 
			*proccess_id = p_table[i].proc_id;
			return 1;
		}
	}

	return 0; // All the proccesses are occupied
}

int distribute_work(int num_procs, unsigned long *starting_values)
{
    int i = -1;
    unsigned long interval = 0;
    
    if (num_procs <= 0) {
        fprintf(stderr, "%s\n", "distribute_work: num_procs is 0 or less");
		return -1;
    } else if (NULL == starting_values) {
        fprintf(stderr, "%s\n", "distribute_work: starting_values cannot be NULL");
		return -1;
    }
    
    interval = (MAX - MIN) / num_procs;
    
    printf("\n\n%lu\n\n", interval); /* DEBUG */
    
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
 ***************************************
 *               TESTED                * 
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
	msg_request_t request_msg;

	/* MPI additional parameters for Send an Recv */
	MPI_Request request;
	MPI_Status status;

	/* Minimum and maximum values */
	unsigned long min_search_value = MIN;
	unsigned long max_search_value = MAX;

	/* ======================  CHECKING PARAMETERS ====================== */

	if (-1 == proc_id) {
		fprintf(stderr, "%s\n", "assign_key_to_proccess: procces_id is -1");
		return -1;
	} else if (0 == num_keys) {
		fprintf(stderr, "%s\n", "assign_key_to_proccess: num_keys is 0");
		return -1;
	} else if (0 == num_procs) {
		fprintf(stderr, "%s\n", "assign_key_to_proccess: num_procs is 0");
		return -1;
	} else if (NULL == k_table) {
	    fprintf(stderr, "%s\n", "assign_key_to_proccess: k_table cannot be NULL");
		return -1;
	} else if (NULL == p_table) {
	    fprintf(stderr, "%s\n", "assign_key_to_proccess: p_table cannot be NULL");
		return -1;
	}
	
	if (NULL == (procs_calc = malloc(num_procs * sizeof(int)))) { 
		fprintf(stderr, "%s\n", "assign_key_to_proccess: ERROR in malloc(procs_calc)");
		return -1;
	}

	for(i = 0; i < num_procs ; i++)
	    procs_calc[i] = -1;

	
	/* Although its main purpose is search keys with the less number of procceses, it also works for what we want here */
	/* Which is finding keys with 0 proccesses asigned */	
	keys_without_procs_flag = search_keys_with_min_num_of_procs(k_table, num_keys, &num_procs_calc, procs_calc, &key_to_assign);
	
	if (NULL == (starting_values = malloc(num_procs_calc * sizeof(unsigned long)))) {
	    fprintf(stderr, "%s\n", "assign_key_to_proccess: ERROR in malloc (starting_values)");
		return -1;
	}
		
	if (1 == keys_without_procs_flag) {
		if (-1 == fill_decrypt_msg(&decrypt_msg, key_to_assign , MIN, MAX) ) {
			fprintf(stderr, "%s\n", "assign_key_to_proccess: ERROR in fill_data_msg (1)");
			return -1;
		}

		if (MPI_SUCCESS != MPI_Send(&decrypt_msg, 1, MPI_DECRYPT_MSG_T, proc_id, DECRYPT_MESSAGE_TAG, MPI_COMM_WORLD) ) { 
			fprintf(stderr, "%s\n", "assign_key_to_proccess: ERROR in MPI_Send (1)");
			return -1;
		}

		/* Register the proccess calculating the key */
		if (-1 == register_proccess_and_key_table(proc_id, key_to_assign.key_id, k_table, p_table) ){
			fprintf(stderr, "%s\n", "assign_key_to_proccess: ERROR in register_proccess_and_key_table");
			return -1;
		}

	} else { // All the keys have been asigned
		for (i = 0; i < num_procs_calc; i++) {
			if (MPI_SUCCESS != MPI_Isend(&request_msg, 1, MPI_INT, procs_calc[i], REQUEST_DATA_MESSAGE_TAG, MPI_COMM_WORLD, &request) ) {
				fprintf(stderr, "%s\n", "assign_key_to_proccess: ERROR in MPI_Send (2)");
				return -1;
			}
		}

		for (i = 0; i < num_procs_calc; i++) {

			if (MPI_SUCCESS != MPI_Recv(&data_msg , 1, MPI_DATA_MSG_T, procs_calc[i], DATA_MESSAGE_TAG, MPI_COMM_WORLD, &status) ) {
				fprintf(stderr, "%s\n", "assign_key_to_proccess: ERROR in MPI_Recv ");
				return -1;
			}

			if (-1 == store_data(p_table, data_msg, num_procs) ){
				fprintf(stderr, "%s\n", "assign_key_to_proccess: ERROR in store_data");
				return -1;
			}

		}

		/* Distribuir trabajo equitativamente */
		distribute_work(num_procs_calc + 1, starting_values); // +1 because there is a new process
		
		/* Notify the procceses that their key is going to be calculated by a new proccess */
		for (i = 0; i < num_procs_calc - 1; i++) {

			if (-1 == fill_decrypt_msg(&decrypt_msg, key_to_assign , starting_values[i], starting_values[i + 1] - 1) ) {
				fprintf(stderr, "%s\n", "assign_key_to_proccess: ERROR in fill_data_msg (1)");
				return -1;
			}

			if (MPI_SUCCESS != MPI_Send(&decrypt_msg, 1, MPI_DECRYPT_MSG_T, procs_calc[i], DECRYPT_MESSAGE_TAG, MPI_COMM_WORLD) ) {
				fprintf(stderr, "%s\n", "assign_key_to_proccess: ERROR in MPI_Send (3)");
				return -1;
			}
		}
		
		/* Send the key to the new proccess */
		if (-1 == fill_decrypt_msg(&decrypt_msg, key_to_assign , starting_values[num_procs_calc - 1], MAX) ) {
			fprintf(stderr, "%s\n", "assign_key_to_proccess: ERROR in fill_data_msg (2)");
			return -1;
		}

		if (MPI_SUCCESS != MPI_Send(&decrypt_msg, 1, MPI_DECRYPT_MSG_T,proc_id, DECRYPT_MESSAGE_TAG, MPI_COMM_WORLD) ) {
			fprintf(stderr, "%s\n", "assign_key_to_proccess: ERROR in MPI_Send (4)");
			return -1;
		}

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
 *  Return: if a key with no procceses  *
 *			is found returns 1          *
 *			Otherwise, returns 0        *
 ****************************************
 *               TESTED                * 
 ***************************************/
int search_keys_with_min_num_of_procs(key_table_t k_table, int num_keys, int* num_procs, int* procs_calc, key_data_t *key)
{
	int i = 0;
	int key_id = -1;
	*num_procs = k_table[0].num_procs_list;
	*key = k_table[0].key;

	/* ======================  CHECKING PARAMETERS ====================== */

	if (0 == num_keys) {
		fprintf(stderr, "%s\n", "search_keys_with_min_num_of_procs: num_keys is 0");
		return -1;
	} else if (NULL == k_table) {
	    fprintf(stderr, "%s\n", "search_keys_with_min_num_of_procs: k_table cannot be NULL");
		return -1;
	} else if (NULL == num_procs) {
	    fprintf(stderr, "%s\n", "search_keys_with_min_num_of_procs: num_procs cannot be NULL");
		return -1;
	} else if (NULL == procs_calc) {
	    fprintf(stderr, "%s\n", "search_keys_with_min_num_of_procs: procs_calc cannot be NULL");
		return -1;
	} else if (NULL == key) {
	    fprintf(stderr, "%s\n", "search_keys_with_min_num_of_procs: key cannot be NULL");
		return -1;
	}

	for (i = 0 ; i < num_keys; i++) {

		/* If there are 2 or more keys with the same number of proccesses, p.e. 0 procceses, we get the first one we picked. */
		if (*num_procs > k_table[i].num_procs_list) { /* That´s why we put > and not >= */
			*num_procs = k_table[i].num_procs_list;
			*key = k_table[i].key;	
			key_id = i;
		}

	}

	if (0 == (*num_procs) ){ /* There is at least one key with no proccesses asigned */
		return 1;
	}

	/* Get the list of proccesses calculating the key */
	for (i = 0 ; i < k_table[key_id].num_procs_list; i++)
		procs_calc[i] = k_table[key_id].procs[i];

	return 0;
}

/***************************************
 *  register_proccess_and_key_table    * 
 ***************************************
 *                                     *
 *  Adds a new proccess to the list    *
 *	of proccess doing a specific key   *
 *                                     *
 *  Return: in case of error -1        *
 *			Otherwise, returns 1       *
 ***************************************
 *               TESTED                * 
 ***************************************/
int register_proccess_and_key_table(int proc_id, int key_id, key_table_t k_table, proc_table_t p_table)
{
	int num_procs_list = 0;

	/* ======================  CHECKING PARAMETERS ====================== */

	if (-1 == proc_id) {
		fprintf(stderr, "%s\n", "register_proccess_and_key_table: key_id is -1");
		return -1;
	} else if (-1 == key_id) {
		fprintf(stderr, "%s\n", "register_proccess_and_key_table: key_id is -1");
		return -1;
	} else if (NULL == k_table) {
	    fprintf(stderr, "%s\n", "register_proccess_and_key_table: k_table cannot be NULL");
		return -1;
	} else if (NULL == p_table) {
	    fprintf(stderr, "%s\n", "register_proccess_and_key_table: p_table cannot be NULL");
		return -1;
	}

	/* It´s proc_id - 1 because the first proccess (id = 1) is in position number 0 in the table */
	p_table[proc_id - 1].occupied_flag = 1; /* A key has been asigned to him */

	num_procs_list = k_table[key_id].num_procs_list;
	k_table[key_id].procs[num_procs_list] = proc_id;

	(k_table[key_id].num_procs_list)++;

	return 0;
}

/***************************************
 * store_data                          * 
 ***************************************
 *                                     *
 *  Fills the data_msg_t we'll send    *
 *                                     *
 *                     				   *
 *                                     *
 *  Return: in case of error -1        *
 *			Otherwise, returns 1       *
 ***************************************
 *               TESTED                * 
 ***************************************/
int store_data(proc_table_t p_table, msg_data_t data_msg, int num_procs){
	
	int n_proc;
	
	n_proc = data_msg.proccess_id - 1;
	if (0 <= n_proc && n_proc < num_procs) {
	    p_table[n_proc].stats.n_keys += 1;
		p_table[n_proc].stats.n_rand_crypt_calls += data_msg.num_tries;
		p_table[n_proc].stats.key_proccesing_times[p_table[n_proc].stats.n_keys - 1] = data_msg.time;
		return 1;
	} else {
	    return -1;
	}
}
