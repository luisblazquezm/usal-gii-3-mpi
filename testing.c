#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"

typedef struct {
	int num1;
	int num2;
	int total;
}suma;

int IO_process(char *argv[], int num_proc);
int calculator_process(int process_id);
int construct_decrypt_msg(int num_keys, suma* data, MPI_Datatype* MPI_Type);

int main(int argc, char *argv[]) {

    MPI_Init(&argc, &argv);

		int id = -1;
		int num_proc = 0;

		if (MPI_SUCCESS != MPI_Comm_rank(MPI_COMM_WORLD, &id)) {
			fprintf(stderr, "%s\n", "main: ERROR in MPI_Comm_rank");
			return -1;
		}

		if (MPI_SUCCESS != MPI_Comm_size(MPI_COMM_WORLD, &num_proc)) {
			fprintf(stderr, "%s\n", "initialice_table_of_keys: ERROR in MPI_Comm_size");
			return -1;
		}


		printf("Hello there! NEW ONE, I am process [nº %d]\n", id);

		if (id == 0) {
			IO_process(argv, num_proc);
		}

		calculator_process(id);

	MPI_Finalize();
}

int IO_process(char *argv[], int num_proc) {

	fprintf(stdout, "Hello there! IO talking, I am process [nº 0]\n");
	int source = 1;
	int REQUEST = 1;
	int ANSWER = 2;

	MPI_Status status;
	MPI_Datatype MPI_suma; // THE NEW TYPE IN MPI
	suma msg;

	construct_decrypt_msg(2, &msg, &MPI_suma);

	while (1){
		
	    if (MPI_SUCCESS != MPI_Recv(&msg , 1, MPI_suma, MPI_ANY_SOURCE, REQUEST, MPI_COMM_WORLD, &status) ) {
			fprintf(stderr, "%s\n", "calculator_process: ERROR in MPI_Recv");
			return -1;
		}

		msg.total = msg.num1 + msg.num2;

		fprintf(stdout, "I am process nº 0: MESSAGE RECEIVED!!!! \n");

		if (MPI_SUCCESS != MPI_Send(&msg , 1, MPI_suma, source, ANSWER, MPI_COMM_WORLD) ) {
			fprintf(stderr, "%s\n", "IO_process: ERROR in MPI_Send");
			return -1;
		}
	}


}

int calculator_process(int id) 
{
	fprintf(stdout, "Hello, I am process nº %d\n", id);

	int source = 0;
	int destination = 0;

	int REQUEST = 1;
	int ANSWER = 2;

	int message_received_flag = 0;
    int new_key_flag = 0;
	int num_tries = 0;

	MPI_Status status;
	MPI_Datatype MPI_suma; // THE NEW TYPE IN MPI
	suma msg;
	construct_decrypt_msg(2, &msg, &MPI_suma);

	msg.num1 = 1; 
	msg.num2 = 1; 
	msg.total = 0;

	while(1)
	{

		fprintf(stdout, "I am process nº %d: MESSAGE SEND TO 0 \n", id);

		/* Assign key to a proccess */
		if (MPI_SUCCESS != MPI_Send(&msg , 1, MPI_suma, source, REQUEST, MPI_COMM_WORLD) ) {
			fprintf(stderr, "%s\n", "IO_process: ERROR in MPI_Send");
			return -1;
		}
		
		/* Data reception from the calculator proccesses */
		if (MPI_SUCCESS != MPI_Recv(&msg , 1, MPI_suma, source, ANSWER, MPI_COMM_WORLD, &status) ) {
			fprintf(stderr, "%s\n", "calculator_process: ERROR in MPI_Recv");
			return -1;
		}

		fprintf(stdout, "I am process nº %d: MESSAGE RECEIVED FROM 0 \n", id);
		fprintf(stdout, "I am process nº %d: Total is ------------------> %d \n", id, msg.total);

		msg.num1 = msg.total;
		msg.num2 = msg.total;
	}
}

int construct_decrypt_msg(int num_keys, suma* data, MPI_Datatype* MPI_Type) 
{
	MPI_Datatype types[3];
	int lengths[3];
	MPI_Aint memory_address[3 + 1];
	MPI_Aint memory_address_distances[3];

	/* 
	 * Indicates the types of the struct
	 *
	 * typedef struct {
	 *		int numero1;
	 *		int numero2;
	 		int numero3;
	 *	} suma;
	 *
	 */
	types[0] = MPI_INT;
	types[1] = MPI_INT;
	types[2] = MPI_INT;

	/* Indicate the numbers of elements of each type */
	lengths[0] = 1;
	lengths[1] = 1;
	lengths[2] = 1;

	/* Calculate the position of the elements in the memory address regarding the beginning of the struct */
	MPI_Address(data, &memory_address[0]);
	MPI_Address(&(data->num1), &memory_address[1]);
	MPI_Address(&(data->num2), &memory_address[2]);
	MPI_Address(&(data->total), &memory_address[3]);

	memory_address_distances[0] = memory_address[1] - memory_address[0];
	memory_address_distances[1] = memory_address[2] - memory_address[0];
	memory_address_distances[2] = memory_address[3] - memory_address[0];

	/* Create struct in MPI */
	if (MPI_SUCCESS != MPI_Type_struct(3, lengths, memory_address_distances, types, MPI_Type) ) {
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
