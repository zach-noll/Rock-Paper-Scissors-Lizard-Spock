#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "HardCoded.h"
#include "Functions.h"


/*********************** Global Paramaeters *************************************/

char msg_from_keyboard[6] = "";
char PlayerOne[10] = "";
char PlayerTwo[10] = "";

BOOL done_flag = FALSE;
int server_port;
int number_of_clients_connected;
int player1_move_number;
int player2_move_number;
int player_replay_array[2] = { 0 };
int number_of_players = 0;

HANDLE number_of_players_mutex;
HANDLE number_of_clients_mutex;
HANDLE game_session_mutex;

HANDLE thread_handles[NUM_OF_WORKER_THREADS];
SOCKET client_sockets[NUM_OF_WORKER_THREADS];

/********************** Functions ****************************************************/

int FindFirstUnusedThreadSlot()
{	//Finds unused thread slot in thread array, returns index
	int ind;

	for (ind = 0; ind < NUM_OF_WORKER_THREADS; ind++)
	{
		if (thread_handles[ind] == NULL)
			break;
		else
		{
			// poll to check if thread finished running:
			DWORD Res = WaitForSingleObject(thread_handles[ind], 10);

			if (Res == WAIT_OBJECT_0) // this thread finished running
			{
				CloseHandle(thread_handles[ind]);
				thread_handles[ind] = NULL;
				break;
			}
		}
	}

	return ind;
}

void CleanupWorkerThreads()
{	//closes thread handles
	int ind;


	if (CloseHandle(number_of_players_mutex) && CloseHandle(number_of_clients_mutex) && CloseHandle(game_session_mutex)) {
		printf("Could not close global thread handles");
		return;
	}
	number_of_clients_mutex;
	game_session_mutex;

	for (ind = 0; ind < NUM_OF_WORKER_THREADS; ind++)
	{
		if (thread_handles[ind] != NULL)
		{
			// poll to check if thread finished running:
			DWORD Res = WaitForSingleObject(thread_handles[ind], INFINITE);

			if (Res == WAIT_OBJECT_0)
			{
				closesocket(client_sockets[ind]);
				CloseHandle(thread_handles[ind]);
				thread_handles[ind] = NULL;
				break;
			}
			else
			{
				printf("Waiting for thread failed. Ending program\n");
				return;
			}
		}
	}
}

int ConvertMoveToNum(char *player_move) {
	//converts move to its corresponding number

	if (strcmp(player_move, "ROCK") == 0)
		return 0;
	else if (strcmp(player_move, "PAPER") == 0)
		return 1;
	else if (strcmp(player_move, "SCISSORS") == 0)
		return 2;
	else if (strcmp(player_move, "LIZARD") == 0)
		return 3;
	else if (strcmp(player_move, "SPOCK") == 0)
		return 4;

}

char *ConvertNumToMove(int player_move_num) {
	//converts move number to its corresponding move

	char player_move[10] = "";
	switch (player_move_num) {
	case 0:
		strcpy(player_move, "ROCK");
		break;
	case 1:
		strcpy(player_move, "PAPER");
		break;
	case 2:
		strcpy(player_move, "SCISSORS");
		break;
	case 3:
		strcpy(player_move, "LIZARD");
		break;
	case 4:
		strcpy(player_move, "SPOCK");
		break;
	}
	return player_move;
}

void FreeMessageStructMemory(t_message *msg_struct) {
	//frees memory allocated for struct
	free(msg_struct->msg);
	for (int i = 0; i < msg_struct->num_of_params; i++)
		free(msg_struct->params[i]);

	free(msg_struct->params);
	free(msg_struct);
}

char *BuildMessageToSend(t_message *msg_struct) {
	//Recieves message structure, and combines its fields into a string to send via socket

	char *message = malloc(msg_struct->msg_len * sizeof(char) + 1);
	memset(message, '\0', msg_struct->msg_len + 1);
	if (message == NULL) {
		printf("Could no allocate memory to build message to send");
		return NULL;
	}

	if (msg_struct->num_of_params == 0) {
		memcpy(message, msg_struct->msg, strlen(msg_struct->msg));
		return message;
	}
	else {
		memcpy(message, msg_struct->msg, strlen(msg_struct->msg));
		message[strlen(msg_struct->msg)] = ':';
		for (int i = 0; i < msg_struct->num_of_params; i++) {
			strcat(message, msg_struct->params[i]);
			if (i < msg_struct->num_of_params - 1) {
				strcat(message, ";");
			}
		}
		strcat(message, "\n");
		return message;
	}
}

int FindMessageLength(char *accepted_msg) {
	//Finds message length - terminating character is \n
	int i = 0;
	while (1) {
		if (accepted_msg[i] == '\n') break;
		i++;
	}
	return i + 2;
}

int FindWinner(int player1_move_number, int player2_move_number) {
	//Finds winner, if return 1, player1 won, if return 2, player 2won, if tie, returns 0

	if (player1_move_number == 0 && player2_move_number == 1) return 2;
	if (player1_move_number == 0 && player2_move_number == 2) return 1;
	if (player1_move_number == 0 && player2_move_number == 3) return 1;
	if (player1_move_number == 0 && player2_move_number == 4) return 2;

	if (player1_move_number == 1 && player2_move_number == 0) return 1;
	if (player1_move_number == 1 && player2_move_number == 2) return 2;
	if (player1_move_number == 1 && player2_move_number == 3) return 2;
	if (player1_move_number == 1 && player2_move_number == 4) return 1;

	if (player1_move_number == 2 && player2_move_number == 0) return 2;
	if (player1_move_number == 2 && player2_move_number == 1) return 1;
	if (player1_move_number == 2 && player2_move_number == 3) return 1;
	if (player1_move_number == 2 && player2_move_number == 4) return 2;

	if (player1_move_number == 3 && player2_move_number == 0) return 2;
	if (player1_move_number == 3 && player2_move_number == 1) return 1;
	if (player1_move_number == 3 && player2_move_number == 2) return 2;
	if (player1_move_number == 3 && player2_move_number == 4) return 1;

	if (player1_move_number == 4 && player2_move_number == 0) return 1;
	if (player1_move_number == 4 && player2_move_number == 1) return 2;
	if (player1_move_number == 4 && player2_move_number == 2) return 1;
	if (player1_move_number == 4 && player2_move_number == 3) return 2;

	return 0;//TIE
}

int DoesFileExist(const char *fname)
{
	//Returns 1 if the filename exists, 0 if it does not
	FILE *file;
	if ((file = fopen(fname, "r")))
	{
		fclose(file);
		return 1;
	}
	return 0;
}

char * read_from_file(int player_id, FILE * fp) {
	//Reads the player move from the file, if player 1 reads first line, if player 2 reads second line

	char move[10] = "";
	if (player_id == 2) {
		fp = fopen("GameSession.txt", "r");
		if (fp != NULL) {
			fgets(move, 10, fp);
		}
	}
	else {
		fp = fopen("GameSession.txt", "r");
		if (fp != NULL) {
			fgets(move, 10, fp);
			fgets(move, 10, fp);
		}
	}
	fclose(fp);
	move[strlen(move) - 1] = '\0';

	return move;

}

HANDLE GetNameEvent(int reset, int initial_state, char event_name[20]) {
	//creates event

	HANDLE new_event;
	DWORD last_error;

	/* Get handle to event by name. If the event doesn't exist, create it */
	new_event = CreateEvent(
		NULL, /* default security attributes */
		reset,       /*1 - manual-reset event, 0 - auto reset */
		initial_state,      /*0 - initial state is non-signaled, 1 - initial state is signaled */
		event_name);         /* name */
	/* Check if succeeded and handle errors */

	last_error = GetLastError();
	/* If last_error is ERROR_SUCCESS, then it means that the event was created.
	   If last_error is ERROR_ALREADY_EXISTS, then it means that the event already exists */

	return new_event;
}

t_message *DivideMessageToCategories(char *msg_2_divide, int msg_len) {
	//recieves message and message length, then returns message struct with paramaters and message
	t_message *divided_msg;
	int i = 0;
	int j = 0;
	int k = 0;
	int param_index = 0;
	int param_len = 0;

	divided_msg = (t_message *)malloc(sizeof(t_message));
	if (divided_msg == NULL) {
		printf("Could not allocate memory");
		return NULL;
	}

	divided_msg->msg_len = msg_len;

	while (msg_2_divide[i] != ':' && msg_2_divide[i] != '\n') {
		i++;
	}
	if (msg_2_divide[i] == ':') divided_msg->num_of_params = 1;

	else if (msg_2_divide[i] == '\n') {
		divided_msg->num_of_params = 0;

	}


	divided_msg->msg = (char *)malloc(i * sizeof(char) + 1);
	if (divided_msg->msg == NULL) {
		printf("Could not allocate memory");
		return NULL;
	}

	strncpy(divided_msg->msg, msg_2_divide, i);
	divided_msg->msg[i] = '\0';

	divided_msg->msg_len = msg_len;


	j = i;
	while (1) {
		if (msg_2_divide[j] == ';' || (msg_2_divide[j] == '\n')) break;
		if (msg_2_divide[j] == ';') divided_msg->num_of_params++;
		j++;
	}

	divided_msg->params = malloc(divided_msg->num_of_params * sizeof(char *));
	if (divided_msg->params == NULL) {
		printf("Could not allocate memory");
		return NULL;
	}
	param_index = 0;

	while (1) {
		if (msg_2_divide[i] == ';' || (msg_2_divide[i] == '\n')) break;
		j = i;
		i++;
		while (1) {
			if (msg_2_divide[i] == ';' || (msg_2_divide[i] == '\n')) break;
			i++;
		}
		param_len = i - j;

		divided_msg->params[param_index] = (char *)malloc(param_len * sizeof(char) + 1);

		if (divided_msg->params[param_index] == NULL) {
			printf("Could not allocate memory");
			return NULL;
		}
		for (k = 0; k < param_len; ++k) {
			divided_msg->params[param_index][k] = msg_2_divide[j + k + 1];
		}
		divided_msg->params[param_index][k - 1] = '\0';

		param_index++;

	}
	return divided_msg;
}

int AllocateAndSend(char *message2allocate, SOCKET transfer_socket) {
	//Allocates memory for message to send, sends the message then frees the memory
	TransferResult_t send_res;
	char * message2send = malloc(strlen(message2allocate) + 1);
	if (message2send == NULL) return 1;

	strcpy(message2send, message2allocate);
	send_res = SendString(message2send, transfer_socket);

	if (send_res != TRNS_SUCCEEDED) {
		free(message2send);
		return 1;
	}
	free(message2send);
	return(0);
}

/*********************** Thread Functions ******************************************/

DWORD WINAPI ClientThread(SOCKET *t_socket)
{
	/* This thread is created for each client that connects. It runs linearly with events and labels to 
	synchronize between threads. I could have split into seperate functions to create smaller code, but this 
	would have reduced code readability */
	char *MessageToSend = NULL;
	char *AcceptedMsg = NULL;
	char *PlayerName = NULL;
	t_message *transfer_message = NULL;

	char server_move[10] = { 0 };

	int ret_val = -1;
	int msg_len = -1;
	int wait_time_in_seconds = 15;
	int winner; //0 - Tie, 1 - player1, 2 - player2
	int player_id = 0;
	int server_move_number = -1;

	char player_move[10] = "";
	char player1_move[10] = "";
	char player2_move[10] = "";


	FILE * fp = NULL;

	TransferResult_t send_res;
	TransferResult_t recv_res;


	HANDLE player_two_joined_event;
	HANDLE player1_replay_event;
	HANDLE player2_replay_event;
	HANDLE player1_write_to_gamesession_event;
	HANDLE player2_write_to_gamesession_event;
	HANDLE game_session_write_event;
	DWORD wait_res;

	player_two_joined_event = GetNameEvent(0, 0, "player_two_joined");
	player1_replay_event = GetNameEvent(0, 0, "player1_replay");
	player2_replay_event = GetNameEvent(0, 0, "player2_replay");
	game_session_write_event = GetNameEvent(0, 1, "GameSession");
	player1_write_to_gamesession_event = GetNameEvent(0, 0, "player1_moved");
	player2_write_to_gamesession_event = GetNameEvent(0, 0, "player2_moved");


	//Creates mutexes to lock file for writing and the global parameters//
	number_of_players_mutex = CreateMutex(NULL, FALSE, NULL);
	game_session_mutex = CreateMutex(NULL, FALSE, NULL);
	if (number_of_players_mutex == NULL || game_session_mutex == NULL) {
		printf("Could not create mutex");
		goto CLIENT_DISCONNECT;
	}

	//Creates events used to synchronize client vs client threads


	if (player_two_joined_event == NULL || player1_replay_event == NULL || player2_replay_event == NULL) {
		printf("Could not get event\n");
		goto CLIENT_DISCONNECT;
	}

	// here server recieves message CLIENT_REQUEST:playername and divides into categories, saving the name for easier to read code
	if (AcceptedMsg == NULL)
		recv_res = ReceiveString(&AcceptedMsg, *t_socket);
	;
	if (recv_res != TRNS_SUCCEEDED) {
		goto CLIENT_DISCONNECT;
	}

	transfer_message = DivideMessageToCategories(AcceptedMsg, FindMessageLength(AcceptedMsg));
	PlayerName = malloc(transfer_message->msg_len * sizeof(char) + 1);
	if (PlayerName == NULL) {
		printf("Could not allocate memory.\n");
		goto CLIENT_DISCONNECT;
	}

	strcpy(PlayerName, transfer_message->params[0]);


	//Sends SERVER_APPROVED back to client
	if (AllocateAndSend("SERVER_APPROVED\n", *t_socket) == 1) goto CLIENT_DISCONNECT;
	



SERVER_MENU:
	//Sends SERVER_MAIN_MENU to client
	if (AllocateAndSend("SERVER_MAIN_MENU\n", *t_socket) == 1) goto CLIENT_DISCONNECT;


	//recieves input from client on what user would like to do
	*AcceptedMsg = NULL;
	AcceptedMsg = NULL;
	recv_res = ReceiveString(&AcceptedMsg, *t_socket);

	if (recv_res != TRNS_SUCCEEDED) {
		goto CLIENT_DISCONNECT;
	}


	transfer_message = DivideMessageToCategories(AcceptedMsg, FindMessageLength(AcceptedMsg));

	/******************** Client plays against server **************************************/
	if (strcmp(transfer_message->msg, "CLIENT_CPU") == 0) {//Play against server

	CLIENT_CPU:

		winner = -1;

		if (AllocateAndSend("SERVER_PLAYER_MOVE_REQUEST\n", *t_socket) == 1) goto CLIENT_DISCONNECT;

		srand((unsigned)time(NULL));
		server_move_number = rand() % 5;

		*AcceptedMsg = NULL;
		AcceptedMsg = NULL;
		recv_res = ReceiveString(&AcceptedMsg, *t_socket);
		if (recv_res != TRNS_SUCCEEDED) {
			goto CLIENT_DISCONNECT;
		}

		transfer_message = DivideMessageToCategories(AcceptedMsg, strlen(AcceptedMsg));

		player2_move_number = ConvertMoveToNum(transfer_message->params[0]);

		winner = FindWinner(server_move_number, player2_move_number);

		strcpy(server_move, ConvertNumToMove(server_move_number));

		MessageToSend = malloc(256 * sizeof(char));

		if (MessageToSend == NULL) goto CLIENT_DISCONNECT;

		if (winner == 1) sprintf(MessageToSend, "SERVER_GAME_RESULTS:Server;%s;%s;Server\n", server_move, transfer_message->params[0]);
		else if (winner == 2) sprintf(MessageToSend, "SERVER_GAME_RESULTS:Server;%s;%s;Player\n", server_move, transfer_message->params[0]);
		else sprintf(MessageToSend, "SERVER_GAME_RESULTS:Server;%s;%s;Tie\n", server_move, transfer_message->params[0]);

		send_res = SendString(MessageToSend, *t_socket);
		if (send_res != TRNS_SUCCEEDED) {
			goto CLIENT_DISCONNECT;
		}

		free(MessageToSend);

		if (AllocateAndSend("SERVER_GAME_OVER_MENU\n", *t_socket) == 1) goto CLIENT_DISCONNECT;



		*AcceptedMsg = NULL;
		AcceptedMsg = NULL;
		recv_res = ReceiveString(&AcceptedMsg, *t_socket);

		if (recv_res != TRNS_SUCCEEDED) {
			goto CLIENT_DISCONNECT;
		}

		transfer_message = DivideMessageToCategories(AcceptedMsg, strlen(AcceptedMsg));

		if (strcmp(transfer_message->msg, "CLIENT_REPLAY") == 0) goto CLIENT_CPU;
		else goto SERVER_MENU;
	}

	/******************** Client plays against client **************************************/
	else if (strcmp(AcceptedMsg, "CLIENT_VERSUS\n") == 0) {
		wait_res = WaitForSingleObject(number_of_players_mutex, INFINITE);
		if (wait_res != WAIT_OBJECT_0) goto CLIENT_DISCONNECT;
		number_of_players++;
		ret_val = ReleaseMutex(number_of_players_mutex); //unlock number of clients mutex
		if (FALSE == ret_val)
		{
			printf("Error when releasing number of clients mutex\n");
			goto CLIENT_DISCONNECT;
		}

	CLIENT_VERSUS:
		wait_res = WaitForSingleObject(number_of_clients_mutex, INFINITE); //lock number of client mutex
		if (wait_res != WAIT_OBJECT_0) goto CLIENT_DISCONNECT;

		if (number_of_clients_connected == 1) { //only one client connected

			ret_val = ReleaseMutex(number_of_clients_mutex); //unlock number of clients mutex
			if (FALSE == ret_val)
			{
				printf("Error when releasing number of clients mutex\n");
				goto CLIENT_DISCONNECT;
			}

			for (int i = 0; i < 30; i++) {
				Sleep(500);



				if (number_of_players == 1) continue;
				if (number_of_players == 2) goto TWO_CLIENTS_CONNECTED;


			}

			wait_res = WaitForSingleObject(number_of_players_mutex, INFINITE);
			if (wait_res != WAIT_OBJECT_0) goto CLIENT_DISCONNECT;
			number_of_players--;
			ret_val = ReleaseMutex(number_of_players_mutex); //unlock number of clients mutex
			if (FALSE == ret_val)
			{
				printf("Error when releasing number of clients mutex\n");
				goto CLIENT_DISCONNECT;
			}

			if (AllocateAndSend("SERVER_NO_OPPONENTS\n", *t_socket) == 1) goto CLIENT_DISCONNECT;

			goto SERVER_MENU;
		}
		else { //two clients are connected
		TWO_CLIENTS_CONNECTED:

			ret_val = ReleaseMutex(number_of_clients_mutex); //unlock number of clients mutex
			wait_res = WaitForSingleObject(game_session_write_event, INFINITE);// player 2 will wait until player 1 creates file
			if (wait_res != WAIT_OBJECT_0) goto CLIENT_DISCONNECT;

			if (DoesFileExist("GameSession.txt") == 0) {//file doesn't exist, this is player 1

				player_id = 1;
				strcpy(PlayerOne, PlayerName);

				fp = fopen("GameSession.txt", "w");
				if (fp == NULL) {
					printf("Could not open gamesession.txt");
					goto CLIENT_DISCONNECT;
				}

				ret_val = SetEvent(game_session_write_event);//player one signals to player two that the thread can check if file exists
				if (ret_val == NULL) goto CLIENT_DISCONNECT;



				wait_res = WaitForSingleObject(player_two_joined_event, INFINITE); //wait for player two to join
				if (wait_res != WAIT_OBJECT_0) goto CLIENT_DISCONNECT;

				if (AllocateAndSend("SERVER_PLAYER_MOVE_REQUEST\n", *t_socket) == 1) goto CLIENT_DISCONNECT;


				*AcceptedMsg = NULL;
				AcceptedMsg = NULL;
				recv_res = ReceiveString(&AcceptedMsg, *t_socket);
				if (recv_res != TRNS_SUCCEEDED) goto CLIENT_DISCONNECT;

				transfer_message = DivideMessageToCategories(AcceptedMsg, strlen(AcceptedMsg));


				strcpy(player1_move, transfer_message->params[0]);
				strcat(player1_move, "\n");
				fputs(player1_move, fp); //write move to file
				fclose(fp);

				ret_val = SetEvent(player1_write_to_gamesession_event); //signal to player 2 that player 1 has finished writing
				if (ret_val == NULL) goto CLIENT_DISCONNECT;

				wait_res = WaitForSingleObject(player2_write_to_gamesession_event, INFINITE); //wait player two to write to game session file
				if (wait_res != WAIT_OBJECT_0) goto CLIENT_DISCONNECT;

				player_replay_array[0] = 0;


				strcpy(player2_move, read_from_file(player_id, fp)); //reads player 2 move from file


				
				winner = FindWinner(ConvertMoveToNum(transfer_message->params[0]), ConvertMoveToNum(player2_move));
				/*winner = 0 - tie
				  winner = 1 - player 1 won
				  winner = 2 - player 2 won*/

				MessageToSend = malloc(256 * sizeof(char)); //allocates enough memory for the largest message that could be sent.
				if (MessageToSend == NULL) goto CLIENT_DISCONNECT;

				if (winner == 1) sprintf(MessageToSend, "SERVER_GAME_RESULTS:%s;%s;%s;You\n", PlayerTwo, player2_move, transfer_message->params[0]);
				else if (winner == 2) sprintf(MessageToSend, "SERVER_GAME_RESULTS:%s;%s;%s;%s\n", PlayerTwo, player2_move, transfer_message->params[0], PlayerTwo);
				else sprintf(MessageToSend, "SERVER_GAME_RESULTS:%s;%s;%s;Tie\n", PlayerTwo, player2_move, transfer_message->params[0]);

				send_res = SendString(MessageToSend, *t_socket);
				if (send_res != TRNS_SUCCEEDED) goto CLIENT_DISCONNECT;

				free(MessageToSend);

				Sleep(100);//sends message directly after another message, sleeps for 100 ms to give time for client to receive each message

				if (AllocateAndSend("SERVER_GAME_OVER_MENU\n", *t_socket) == 1) goto CLIENT_DISCONNECT;


				*AcceptedMsg = NULL;
				AcceptedMsg = NULL;
				recv_res = ReceiveString(&AcceptedMsg, *t_socket);
				if (recv_res != TRNS_SUCCEEDED) goto CLIENT_DISCONNECT;

				//deletes gamesession.txt at the end of the game
				if (remove("GameSession.txt") == -1) {
					printf("Could not delete GameSession.txt");
					goto CLIENT_DISCONNECT;
				}

				if (strcmp(AcceptedMsg, "CLIENT_REPLAY\n") == 0) { //player 1 wants to replay
					player_replay_array[0] = 1;

					ret_val = SetEvent(player1_replay_event); //signals to player 2 that player one has made decision
					if (ret_val == NULL) goto CLIENT_DISCONNECT;

					wait_res = WaitForSingleObject(player2_replay_event, INFINITE); //wait for player 2 to make decision
					if (wait_res != WAIT_OBJECT_0) goto CLIENT_DISCONNECT;

					if (player_replay_array[1] == 1) { //player 2 wants to replay

						goto CLIENT_VERSUS;
					}
					else {

						MessageToSend = malloc(256 * sizeof(char));
						if (MessageToSend == NULL) goto CLIENT_DISCONNECT;
						sprintf(MessageToSend, "SERVER_OPPONENT_QUIT:%s\n", PlayerTwo);


						send_res = SendString(MessageToSend, *t_socket); //player 2 does not want to replay
						if (send_res != TRNS_SUCCEEDED) goto CLIENT_DISCONNECT;

						free(MessageToSend);

						player_replay_array[0] = 0;
						goto SERVER_MENU;
					}
				}
				else {
					ret_val = SetEvent(player1_replay_event);//signal to player 2 that player one has made decision
					if (ret_val == NULL) goto CLIENT_DISCONNECT;

					goto SERVER_MENU;
				}
			}
			else { //file GameSession.txt exists, this is player2

				ret_val = SetEvent(game_session_write_event);
				if (ret_val == NULL) goto CLIENT_DISCONNECT;

				player_id = 2;
				strcpy(PlayerTwo, PlayerName);

				ret_val = SetEvent(player_two_joined_event); //signal to player 1 that player 2 has joined
				if (ret_val == NULL) goto CLIENT_DISCONNECT;

				if (AllocateAndSend("SERVER_PLAYER_MOVE_REQUEST\n", *t_socket) == 1) goto CLIENT_DISCONNECT;

				*AcceptedMsg = NULL;
				AcceptedMsg = NULL;
				recv_res = ReceiveString(&AcceptedMsg, *t_socket);
				if (recv_res != TRNS_SUCCEEDED) goto CLIENT_DISCONNECT;

				transfer_message = DivideMessageToCategories(AcceptedMsg, strlen(AcceptedMsg));

				wait_res = WaitForSingleObject(player1_write_to_gamesession_event, INFINITE);//wait for player 1 to write move to file
				if (wait_res != WAIT_OBJECT_0) goto CLIENT_DISCONNECT;

				player_replay_array[1] = 0;

				fp = fopen("GameSession.txt", "a");
				if (fp == NULL) {
					printf("Could not open gamesession.txt");
					closesocket(*t_socket);
					return SERVER_DISCONNECT;
				}

				strcpy(player2_move, transfer_message->params[0]);
				strcat(player2_move, "\n");
				fputs(player2_move, fp); //write move to file
				fclose(fp);

				ret_val = SetEvent(player2_write_to_gamesession_event); //signal player 1 that player 2 is done writing
				if (ret_val == NULL) goto CLIENT_DISCONNECT;

				wait_res = WaitForSingleObject(game_session_mutex, INFINITE);
				if (wait_res != WAIT_OBJECT_0) goto CLIENT_DISCONNECT;

				strcpy(player1_move, read_from_file(player_id, fp)); //reads player 1 move from file

				ret_val = ReleaseMutex(game_session_mutex);
				if (ret_val == NULL) goto CLIENT_DISCONNECT;

				winner = FindWinner(ConvertMoveToNum(player1_move), ConvertMoveToNum(transfer_message->params[0]));
				/*winner = 0 - tie
				  winner = 1 - player 1 won
				  winner = 2 - player 2 won*/
				
				
				MessageToSend = malloc(256 * sizeof(char)); //allocates enough memory for largest message size possible
				if (MessageToSend == NULL) goto CLIENT_DISCONNECT;

				if (winner == 2) sprintf(MessageToSend, "SERVER_GAME_RESULTS:%s;%s;%s;You\n", PlayerOne, player1_move, transfer_message->params[0]);
				else if (winner == 1) sprintf(MessageToSend, "SERVER_GAME_RESULTS:%s;%s;%s;%s\n", PlayerOne, player1_move, transfer_message->params[0], PlayerOne);
				else sprintf(MessageToSend, "SERVER_GAME_RESULTS:%s;%s;%s;Tie\n", PlayerOne, player1_move, transfer_message->params[0]);

				send_res = SendString(MessageToSend, *t_socket);
				if (send_res != TRNS_SUCCEEDED) goto CLIENT_DISCONNECT;

				free(MessageToSend); //done with sending message, frees allocated memory
				Sleep(100);


				if (AllocateAndSend("SERVER_GAME_OVER_MENU\n", *t_socket) == 1) goto CLIENT_DISCONNECT;



				*AcceptedMsg = NULL;
				AcceptedMsg = NULL;
				ret_val = ReceiveString(&AcceptedMsg, *t_socket);
				if (ret_val == NULL) goto CLIENT_DISCONNECT;

				if (strcmp(AcceptedMsg, "CLIENT_REPLAY\n") == 0) { //player two wants to replay
					player_replay_array[1] = 1;

					ret_val = SetEvent(player2_replay_event); //signal to player 1 that player 2 decision has been made
					if (ret_val == NULL) goto CLIENT_DISCONNECT;

					wait_res = WaitForSingleObject(player1_replay_event, INFINITE); //wait for player 1 to make decision
					if (wait_res != WAIT_OBJECT_0) goto CLIENT_DISCONNECT;

					if (player_replay_array[0] == 1) { //player 1 wants to replay


						goto CLIENT_VERSUS; //start replay
					}
					else {

						player_replay_array[1] = 0; //player 1 does not want to replay
						MessageToSend = malloc(256 * sizeof(char));
						if (MessageToSend == NULL) goto CLIENT_DISCONNECT;
						sprintf(MessageToSend, "SERVER_OPPONENT_QUIT:%s\n", PlayerOne);
						send_res = SendString(MessageToSend, *t_socket);
						if (send_res != TRNS_SUCCEEDED) goto CLIENT_DISCONNECT;
						free(MessageToSend);
						goto SERVER_MENU;
					}
				}
				else {
					ret_val = SetEvent(player2_replay_event);
					if (ret_val == NULL) goto CLIENT_DISCONNECT;
					goto SERVER_MENU;
				}
			}
		}
	}

	/******************** Client wants to disconnect **************************************/
	else if (strcmp(AcceptedMsg, "CLIENT_DISCONNECT\n")) goto CLIENT_DISCONNECT;


CLIENT_DISCONNECT:
	//Client disconnect - subtracts 1 from number of clients connected, frees memory and closes socket. 
	ret_val = CloseHandles(player_two_joined_event ,player1_replay_event, player2_replay_event, player1_write_to_gamesession_event, player2_write_to_gamesession_event, game_session_write_event);
	if (ret_val == FALSE) return EXIT_CODE;
	

	

	wait_res = WaitForSingleObject(number_of_clients_mutex, INFINITE);
	if (wait_res != WAIT_OBJECT_0) {
		printf("Could not lock number of clients mutex");
		return SERVER_DISCONNECT;
	}
	number_of_clients_connected--;

	ret_val = ReleaseMutex(number_of_clients_mutex);
	if (FALSE == ret_val)
	{
		printf("Error when releasing number of clients mutex\n");
		return SERVER_DISCONNECT;
	}
	
	FreeMessageStructMemory(transfer_message);
	closesocket(*t_socket);
	printf("Client disconnected.\n");
	printf("Waiting for client to connect...\n");
	return 0;
}

DWORD WINAPI KeyboardInputThread(SOCKET *main_socket) {
	//Thread that listens for input 'exit' in server windows
	while (1) {
		scanf("%s", &msg_from_keyboard);
		

		if (strcmp(msg_from_keyboard, "exit") == 0) {
			done_flag = TRUE;
			for (int i = 0; i < NUM_OF_WORKER_THREADS; i++) {
				if (client_sockets[i] != NULL)
					closesocket(client_sockets[i]);
					
			}
			closesocket(*main_socket);
			return 0;
		}
	}
}

DWORD WINAPI ListenThread(SOCKET *main_socket) {
	//Thread that listens for new clients attempting to connect. Will only allow 2 connections, 3rd connection will be denied
	TransferResult_t send_res;
	DWORD wait_res;
	BOOL ret_val;
	number_of_clients_connected = 0;
	int Ind;

	if (number_of_clients_mutex == NULL) {
		printf("could not create mutex");
		return SERVER_DISCONNECT;
	}
	for (Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++)
		thread_handles[Ind] = NULL;

	while (1) {//start listening

		SOCKET AcceptSocket = accept(*main_socket, NULL, NULL);
		if (AcceptSocket == INVALID_SOCKET && done_flag != TRUE)
		{
			printf("Accepting connection with client failed, error %ld\n", WSAGetLastError());
			return EXIT_CODE;

		}
		else if (AcceptSocket == INVALID_SOCKET) {//exit was entered to server terminal
			return 0;
		}

		printf("Client Connected.\nWaiting for client to connect...\n");

		Ind = FindFirstUnusedThreadSlot();

		wait_res = WaitForSingleObject(number_of_clients_mutex, INFINITE);
		if (wait_res != WAIT_OBJECT_0) {
			printf("Could not lock number of clients mutex");
			return SERVER_DISCONNECT;
		}
		number_of_clients_connected++;

		ret_val = ReleaseMutex(number_of_clients_mutex);
		if (FALSE == ret_val)
		{
			printf("Error when releasing number of clients mutex\n");
			return SERVER_DISCONNECT;
		}


		if (Ind == NUM_OF_WORKER_THREADS) //no slot is available
		{
			printf("3rd client attempted to connect, denying access to server and closing socket...\nWaiting for client to connect...\n");

			send_res = SendString("SERVER_DENIED:Two clients are already connected. Disconnecting...\n", AcceptSocket);
			closesocket(AcceptSocket); //Closing the socket, dropping the connection.

		}
		else
		{
			client_sockets[Ind] = AcceptSocket; // shallow copy: don't close 
											  // AcceptSocket, instead close 
											  // client_sockets[ind] when the
											  // time comes.
			thread_handles[Ind] = CreateThread(
				NULL,
				0,
				(LPTHREAD_START_ROUTINE)ClientThread,
				&(client_sockets[Ind]),
				0,
				NULL
			);


		}
	}

}

BOOL CloseHandles(HANDLE handle1, HANDLE handle2, HANDLE handle3, HANDLE handle4, HANDLE handle5, HANDLE handle6) {
	BOOL ret_val;
	ret_val = CloseHandle(handle1);
	if (ret_val == FALSE) {
		printf("Error when closing handles. Returning 0x55");
			return FALSE;
	}
	ret_val = CloseHandle(handle2);
	if (ret_val == FALSE) {
		printf("Error when closing handles. Returning 0x55");
		return FALSE;
	}
	ret_val = CloseHandle(handle3);
	if (ret_val == FALSE) {
		printf("Error when closing handles. Returning 0x55");
		return FALSE;
	}
	ret_val = CloseHandle(handle4);
	if (ret_val == FALSE) {
		printf("Error when closing handles. Returning 0x55");
		return FALSE;
	}
	ret_val = CloseHandle(handle5);
	if (ret_val == FALSE) {
		printf("Error when closing handles. Returning 0x55");
		return FALSE;
	}
	ret_val = CloseHandle(handle6);
	if (ret_val == FALSE) {
		printf("Error when closing handles. Returning 0x55");
		return FALSE;
	}
	return TRUE;
}

/******************************** Main Server Function *******************************/

int MainServer(int argc, char *argv[]) {

	

	SOCKET main_socket = INVALID_SOCKET;
	HANDLE listen_thread_handle = INVALID_SOCKET;
	unsigned long address;
	SOCKADDR_IN service;
	HANDLE keyboard_thread_handle = NULL;
	int bind_res;
	int listen_res;

	int ret_val;
	static const BOOL IS_MANUAL_RESET = TRUE;
	static const BOOL IS_INITIALLY_SET = FALSE;

	number_of_clients_mutex = CreateMutex(NULL, FALSE, NULL);


	// get port number and allocate memory for it
	server_port = malloc(sizeof(int));
	if (server_port == NULL) {
		printf("Could not allocate memory for port. Closing program\n");
		return EXIT_FAILURE;
	}

	sscanf(argv[1], "%d", &server_port);

	// Initialize Winsock

	WSADATA wsaData;
	int startup_res = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (startup_res != NO_ERROR)
	{
		printf("Error %ld at WSAStartup( ), ending program. \n", WSAGetLastError());
		// Tell the user that we could not ifnd a usable WinSock DLL

		return EXIT_FAILURE;
	}

	/* WinSock DLL is acceptable. Proceed. */

	// Create a socket.

	main_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (main_socket == INVALID_SOCKET) {
		printf("Error at socket()L %ld\n", WSAGetLastError());
		goto server_cleanup_1;
	}


	address = inet_addr(SERVER_ADDRESS_STR);
	if (address == INADDR_NONE)
	{
		printf("The string \"%s\" cannot be converted into an ip address. ending program.\n",
			SERVER_ADDRESS_STR);
		goto server_cleanup_2;
	}

	service.sin_family = AF_INET;
	service.sin_addr.s_addr = address;
	service.sin_port = htons(server_port);

	// Call the bind function, passing the created socket and the sockaddr_in structure as parameters. 
	// Check for general errors.
	bind_res = bind(main_socket, (SOCKADDR*)&service, sizeof(service));
	if (bind_res == SOCKET_ERROR)
	{
		printf("bind( ) failed with error %ld. Ending program\n", WSAGetLastError());
		goto server_cleanup_2;
	}

	// Listen on the Socket.
	listen_res = listen(main_socket, SOMAXCONN);
	if (listen_res == SOCKET_ERROR)
	{
		printf("Failed listening on socket, error %ld.\n", WSAGetLastError());
		goto server_cleanup_2;
	}


	printf("Waiting for a client to connect...\n");

	/* Thread which looks for 'exit' input on keyboard */
	keyboard_thread_handle = CreateThread(NULL,
		NULL,
		(LPTHREAD_START_ROUTINE)KeyboardInputThread,
		&main_socket,
		NULL,
		NULL);
	if (keyboard_thread_handle == NULL) {
		printf("Could not open keyboard thread\n");
		goto server_cleanup_3;
	}

	/* Thread that listens for new connections */
	listen_thread_handle = CreateThread(
		NULL,
		NULL,
		(LPTHREAD_START_ROUTINE)ListenThread,
		&main_socket,
		NULL,
		NULL);

	if (listen_thread_handle == NULL) {
		printf("Could not open thread handle");
		goto server_cleanup_3;
	}

	/* Here the main function will wait until 'exit' is entered at keybaord to close program */
	ret_val = WaitForSingleObject(keyboard_thread_handle, INFINITE);

	goto server_cleanup_1;
	if (WAIT_OBJECT_0 != ret_val) {
		printf("Error when waiting for keyboard input\n");
		
	}

server_cleanup_3:

	CleanupWorkerThreads(); //close all thread handles

server_cleanup_2:

	if (closesocket(main_socket) == SOCKET_ERROR) //close main socket
		printf("Failed to close main_socket, error %ld. Ending program\n", WSAGetLastError());

server_cleanup_1:

	if (WSACleanup() == SOCKET_ERROR) //cleanup
		printf("Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError());

	return 0;
}