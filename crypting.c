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
	Message_type message;
	MPI_Datatype pMPI_Message_type;

	/* Here comes all the shitty stuff */
	if (-1 == MPI_Comm_rank(MPI_COMM_WORLD, &id)) {
		fprintf(stderr, "%s\n", "main: ERROR in MPI_Comm_rank");
	}

	printf("Hello, I am process nº %d\n", id);



	if (id == 0) {
		IO_process(&argv);
	}

	calculator_process(id);

	MPI_Finalize();
	return 0;
}

/***************************************************
 *  1) Wait for the key to arrive from IO process  *
 *  2) Try to decrypt it                           *
 *     2.1) Found it --> notifies the rest of      *
 *			processes                              *
 *     2.2) Not Found it --> keeps trying :)       *
 *  3) Receives a new key                          *
 ***************************************************/
int calculator_process(int process_id) 
{
	int source = IO_PROCESS_ID;
	int message_id = process_id;
	int message_received_flag = 0;
    int new_key_flag = 0;

	Message_type message;
	Key_type dec_key;
	int num_tries = 0;

	MPI_Status status;

	while(1)
	{
		/* Waiting to receive the key from IO proccess */
		if (-1 == MPI_Recv(message , 1, pMPI_Message_type, source, /* Type of message to receive */, MPI_COMM_WORLD, &status) ) {
			fprintf(stderr, "%s\n", "calculator_process: ERROR in MPI_Recv");
			return -1;
		}

		while(!message_received_flag) { // We check it with MPI_lprobe

			message.key = key_decrypter(message.key, &dec_key, message.length);

			if (0 < num_tries) {
				// Notify
				new_key_flag = 1;
			}

			/* Check if there is any message */
			if (-1 == MPI_lprobe(source, /* Type of message to receive */, MPI_COMM_WORLD, &message_received_flag, &status)) {
				fprintf(stderr, "%s\n", "calculator_process: ERROR in MPI_lprobe");
				return -1;
			}
		}

		if (new_key_flag)
			continue;


	}
}

/*************************************************
 *  1) Generate random keys                      *
 *  2) Encrypt the keys                          *
 *  3) Constructs the type of MPI for structs    *
 *  3) Send the keys to calculator proccesses    *
 *************************************************/
int IO_process(char *argv[]) 
{
	int numberOfKeys = 0;
	char **encrypted_keys; /* Array of encrypted keys */

	/* Reserve dynamic memory for (numberOfKeys) strings of (KEY_LENGTH) characters */
	if (NULL == (encrypted_keys = malloc(numberOfKeys * sizeof(char*)))){
		fprintf(stderr, "%s\n", "IO_process: ERROR in malloc(char*)");
		return -1;
	}

	for (int i = 0; i < numberOfKeys; i++) {
    	if (NULL == (encrypted_keys[i] = malloc(KEY_LENGTH * sizeof(char)))){
    		fprintf(stderr, "%s\n", "IO_process: ERROR in malloc(char)");
			return -1;
    	}
    }

	if(argv[1] == NULL){
		printf("Introduce the number of keys that you want generate\n");
	}else{
		numberOfKeys = atoi(argv[1]);
	}

	Key_type keys[numberOfKeys]; /* Array of generated random keys*/

	/* Generate random keys */
	key_generator(numberOfKeys, &keys);

	/* Encrypt the keys (only numbers) into encrypted_keys (strings) */
	for(int i = 0; i < numberOfKeys; i++) {
		encrypted_keys[i] = key_encrypter(keys[i]);
	}

	/* Creates the new type of struct so it can be availabe for all the proccesses */ 
	if (-1 == construct_message_type(atoi(argv[1]), &message, &pMPI_Message_type)) { //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< I HAVE DOUBTS ABOUT WHERE TO PUT THIS
		fprintf(stderr, "%s\n", "IO_process: construct_message_type");
		return -1;
	}

	/* SEND THE KEY */

	while(1){
		/* KEEPS WAITING FOR SOMEONE TO HAVE FINISHED. SO HE CAN GIVE IT ANOTHER KEY */
	}
}

/* Generates the struct containing the key and the size of the key in MPI datatypes */
int construct_message_type(int numberOfKeys, Message_type* pdata, MPI_Datatype* pMPI_new_data_type) 
{
	MPI_Datatype types[N_STRUCT_ELEMENTS];
	int lengths[N_STRUCT_ELEMENTS];
	MPI_Aint memory_address[N_STRUCT_ELEMENTS + 1];
	MPI_Aint memory_address_distances[N_STRUCT_ELEMENTS];

	MPI_Datatype pMPI_Char_array; /* char * type in MPI */

	/* Create array of char or char* in MPI to store the encrypted keys */
	if (-1 == MPI_Type_vector(numberOfKeys, 1, numberOfKeys, MPI_CHAR, &pMPI_Char_array) ) {
		fprintf(stderr, "%s\n", "construct_message_type: ERROR in MPI_Type_vector");
		return -1;
	}

	/* Certificate it before being used */
	if (-1 == MPI_Type_Commit(pMPI_Char_array) ) {
		fprintf(stderr, "%s\n", "construct_message_type: ERROR in MPI_Type_Commit 1");
		return -1;
	}

	/* 
	 * Indicates the types of the struct
	 *
	 * typedef struct {
	 *    char* key;
	 *    int process_id;
	 *    unsigned long num_tries;
	 *    double time;
	 * }
	 *
	 */
	types[0] = pMPI_Char_array;
	types[1] = MPI_INT;
	types[2] = MPI_UNSIGNED_LONG;
	types[3] = MPI_DOUBLE;

	/* Indicate the numbers of elements of each type */
	lengths[0] = 1;
	lengths[1] = 1;
	lengths[2] = 1;
	lengths[3] = 1;

	/* Calculate the position of the elements in the memory address regarding the beginning of the struct */
	MPI_Address(pdata, &memory_address[0]);
	MPI_Address(&(pdata->key), &memory_address[1]);
	MPI_Address(&(pdata->process_id), &memory_address[2]);
	MPI_Address(&(pdata->num_tries), &memory_address[3]);
	MPI_Address(&(pdata->time), &memory_address[4]);

	memory_address_distances[0] = memory_address[1] - memory_address[0];
	memory_address_distances[1] = memory_address[2] - memory_address[0];
	memory_address_distances[2] = memory_address[3] - memory_address[0];
	memory_address_distances[3] = memory_address[4] - memory_address[0];

	/* Create struct in MPI */
	if (-1 == MPI_Type_struct(N_STRUCT_ELEMENTS, lengths, memory_address_distances, types, pMPI_new_data_type) ) {
		fprintf(stderr, "%s\n", "construct_message_type: ERROR in MPI_Type_struct");
		return -1;
	}

	/* Certificate the struct called pMPI_new_data_type before being used */
	if (-1 == MPI_Type_Commit(pMPI_new_data_type) ) {
		fprintf(stderr, "%s\n", "construct_message_type: ERROR in MPI_Type_Commit 2");
		return -1;
	}
}

void key_generator(int numberOfKeys, Key_type *key)
{
	//double key[numberOfKeys];
	int i;

	srand(time(NULL));

	for(i = 0; i < numberOfKeys; i++){
		key[i] = MIN + rand() % (MAX - MIN);
		printf("%.0f \n", key[i]);
	}
}

/* INDICATION --> Returns the encrypted key */
char *key_encrypter(Key_type key) 
{
	return crypt(key, "aa");
}

/* INDICATION --> Returns the number of tries */
unsigned long long key_decrypter(char *encrypted_key, Key_type *decrypted_key, int key_length)
{
	unsigned long long num_tries;
	char decrypt_string[key_length];

	srand(1);
	do
	{
		sprintf(decrypt_string,"%08d",rand()%100000000);
		num_tries++;
		if (0 == strcmp(crypt(decrypt_string, "aa"), encrypted_key)) {
			printf("Encontrada: %s->%s (%lld)\n", encrypted_key, decrypt_string, intentos);
			break;
		}
	} while (1);

	*decrypted_key = strtoul(decrypt_string, NULL, 10);

	return num_tries;
}