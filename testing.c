#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {

    calculator_process(1);

    return 0;
}

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
