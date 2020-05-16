#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <winsock2.h>

#include "Socket.h"
#include "SocketSendRecvTools.h"


/*********************** Global Paramaeters *************************************/

SOCKET m_socket;
SOCKADDR_IN clientService;
char message_from_keyboard[9];//the longest message from the keyboard is scissors 
char *Acceptedmessage_timeout = NULL;
char global_ip[256];
int global_port;
int main_menu = 0;
int game_over = 0;
int move = 0;
int disconnect_flag = 0; //0 - dont disconnect, 1 - disconnect

typedef struct {
	char *msg;
	char **params;
	int msg_len;
	int num_of_params;
}t_message;



/********************** Functions ****************************************************/

t_message *DivideMessageToCategories(char *msg_2_divide, int msg_len) {//This function gets a message from the server and seperates to 4 cattegories into t_message struct
	t_message *divided_msg;
	int i = 0;
	int j = 0;
	int k = 0;
	int param_index = 0;
	int param_len = 0;

	divided_msg = (t_message *)malloc(sizeof(t_message));
	if (divided_msg == NULL) {
		printf("Could not allocate memory");
		return EXIT_FAILURE;
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
		return EXIT_FAILURE;
	}

	strncpy(divided_msg->msg, msg_2_divide, i);
	divided_msg->msg[i] = '\0';

	divided_msg->msg_len = msg_len;


	j = i;
	while (msg_2_divide[j] != '\n') {
		if (msg_2_divide[j] == ';') divided_msg->num_of_params++;
		j++;
	}

	divided_msg->params = malloc(divided_msg->num_of_params * sizeof(char *));
	if (divided_msg->params == NULL) {
		printf("Could not allocate memory");
		return EXIT_FAILURE;
	}
	param_index = 0;
	while (msg_2_divide[i] != '\n') {
		j = i;
		i++;
		while (msg_2_divide[i] != ';' && msg_2_divide[i] != '\n') {
			i++;
		}
		param_len = i - j;

		divided_msg->params[param_index] = (char *)malloc(param_len * sizeof(char) + 1);

		if (divided_msg->params[param_index] == NULL) {
			printf("Could not allocate memory");
			return;
		}
		for (k = 0; k < param_len; ++k) {
			divided_msg->params[param_index][k] = msg_2_divide[j + k + 1];
		}
		divided_msg->params[param_index][k - 1] = '\0';

		param_index++;

	}
	return divided_msg;
}

FreeMessageMemory(t_message *msg_struct) {//frees memory allocated for struct
	free(msg_struct->msg);
	for (int i = 0; i < msg_struct->num_of_params; i++)
		free(msg_struct->params[i]);

	free(msg_struct->params);
	free(msg_struct);
}

void menu() {//This function print the main_menu
	printf("Choose what to do next:\n1.Play against another client\n2.Play against the server\n3.Quit\n");
	return;
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


static DWORD RecvDataThread(void)//This function gets a message from the server and print the relevant message to the scrren
{
	TransferResult_t RecvRes;
	DWORD ret_val;
	HANDLE string_from_keyboard = GetNameEvent(0, 0, "keyboard input");

	while (1)
	{
		char *Acceptedmessage = NULL;

		int timeout = 100; //wait for server to respond
		int i = 0;
		t_message *received_message_struct = NULL;
		RecvRes = Recv_TimeOut(m_socket, timeout, &Acceptedmessage);//recv message with time out

		if (RecvRes == TRNS_FAILED)
		{
			if (disconnect_flag == 0) {
				printf("Lost connection to server on %s %d\n", global_ip, global_port);
				goto Try_Reconnect_Timeout;
			}
			return 0;
		}
		else if (RecvRes == TRNS_DISCONNECTED)
		{
			if (disconnect_flag == 0) {

				printf("Lost connection to server on %s %d\n", global_ip, global_port);
				goto Try_Reconnect_Timeout;
			}
			return 0;
		}
		else if (RecvRes == TIME_OUT)
			goto Time_Out;
		else if (STRINGS_ARE_EQUAL(Acceptedmessage, "SERVER_NO_OPPONENTS\n")) {
			printf("There are no opponents\n");
		}

		if (STRINGS_ARE_EQUAL(Acceptedmessage, "SERVER_PLAYER_MOVE_REQUEST\n")) {
			move = 1;
			printf("Choose a move from the list:Rock,Paper,Scissors,Lizard or spock:");

		}


		else if (STRINGS_ARE_EQUAL(Acceptedmessage, "SERVER_MAIN_MENU\n")) {
			main_menu = 1;
			menu();

		}
		else if (STRINGS_ARE_EQUAL(Acceptedmessage, "SERVER_GAME_OVER_MENU\n")) {
			game_over = 1;
			printf("Choose what to do next:\n1.Play again\n2.Return to main menu\n");

		}


		while (1) {
			if (Acceptedmessage[i] == '\n') break;//check the length of message with paramters
			i++;
		}

		received_message_struct = DivideMessageToCategories(Acceptedmessage, i);//send messages with paramters to DivideMessageToCategories


		if (STRINGS_ARE_EQUAL(received_message_struct->msg, "SERVER_OPPONENT_QUIT")) {
			printf("%s has left the game!\n", received_message_struct->params[0]);
		}
		else if (STRINGS_ARE_EQUAL(received_message_struct->msg, "SERVER_GAME_RESULTS")) {
			if (STRINGS_ARE_EQUAL(received_message_struct->params[3], "Tie")) {
				printf("You played: %s\n%s played: %s\n", received_message_struct->params[2], received_message_struct->params[0], received_message_struct->params[1]);
			}

			else
				printf("You played:%s\n%s played: %s\n%s won!\n", received_message_struct->params[2], received_message_struct->params[0], received_message_struct->params[1], received_message_struct->params[3]);

		}
		else if (STRINGS_ARE_EQUAL(received_message_struct->msg, "SERVER_INVITE")) {
			printf("A game is going to start against:%s\n", received_message_struct->params[0]);
		}
		free(Acceptedmessage);
	}
	return 0;
Try_Reconnect:
	if (connect(m_socket, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR) {
		printf("Failed connecting to server on %s %d\n", global_ip, global_port);

	Try_Reconnect_Timeout:
		printf("Choose what to do next\n1.Try to reconnect\n2.Exit\n");
		/*gets_s(message_from_keyboard, sizeof(message_from_keyboard));*/

		ret_val = WaitForSingleObject(string_from_keyboard, INFINITE);
		if (ret_val != WAIT_OBJECT_0) {
			printf("Error while waiting for string from keyboard event\n");
			return 0;
		}
		if (STRINGS_ARE_EQUAL(message_from_keyboard, "1"))
			goto Try_Reconnect;
		else return 0;
	}
Time_Out:
	printf("Connection to server on %s:%s has been lost", global_ip, global_port);
	goto Try_Reconnect_Timeout;

}

static DWORD SendDataThread(void)//This function send the relevant messageto the server,the functon gets  a message from the keyboard and according to the coose of the user it decides what to send to the server
{

	char message2send[28];//the longest message is CLIENT_PLAYER_MOVE
	TransferResult_t SendRes;
	HANDLE string_from_keyboard;
	DWORD ret_val;

	string_from_keyboard = GetNameEvent(0, 0, "keyboard input");
	while (1)

	{
		gets_s(message_from_keyboard, sizeof(message_from_keyboard)); //Reading a string from the keyboard

		ret_val = SetEvent(string_from_keyboard);

		strcpy(message_from_keyboard, _strupr(message_from_keyboard));

		if (STRINGS_ARE_EQUAL(message_from_keyboard, "1") && main_menu == 1) {//play against another client
			ret_val = ResetEvent(string_from_keyboard);
			main_menu = 0;
			sprintf(message2send, "CLIENT_VERSUS\n");
		}
		else if (STRINGS_ARE_EQUAL(message_from_keyboard, "2") && main_menu == 1) {//play against the server
			main_menu = 0;
			sprintf(message2send, "CLIENT_CPU\n");
		}
		else if (STRINGS_ARE_EQUAL(message_from_keyboard, "3") && main_menu == 1) {//quit
			main_menu = 0;
			sprintf(message2send, "CLIENT_DISCONNECT\n");
			disconnect_flag = 1;
		}
		//choose the move
		else if (STRINGS_ARE_EQUAL(message_from_keyboard, "ROCK") && move == 1) {
			move = 0;
			sprintf(message2send, "CLIENT_PLAYER_MOVE:ROCK\n");
		}
		else if (STRINGS_ARE_EQUAL(message_from_keyboard, "PAPER") && move == 1) {
			move = 0;
			sprintf(message2send, "CLIENT_PLAYER_MOVE:PAPER\n");
		}
		else if (STRINGS_ARE_EQUAL(message_from_keyboard, "SCISSORS") && move == 1) {
			move = 0;
			sprintf(message2send, "CLIENT_PLAYER_MOVE:SCISSORS\n");
		}
		else if (STRINGS_ARE_EQUAL(message_from_keyboard, "LIZARD") && move == 1) {
			move = 0;
			sprintf(message2send, "CLIENT_PLAYER_MOVE:LIZARD\n");
		}
		else if (STRINGS_ARE_EQUAL(message_from_keyboard, "SPOCK") && move == 1) {
			move = 0;
			sprintf(message2send, "CLIENT_PLAYER_MOVE:SPOCK\n");
		}

		//server game_over
		else if (STRINGS_ARE_EQUAL(message_from_keyboard, "1") && game_over == 1) {
			game_over = 0;
			sprintf(message2send, "CLIENT_REPLAY\n");
		}
		else if (STRINGS_ARE_EQUAL(message_from_keyboard, "2") && game_over == 1) {
			game_over = 0;
			sprintf(message2send, "CLIENT_MAIN_MENU\n");
		}

		SendRes = SendString(message2send, m_socket);

		if (STRINGS_ARE_EQUAL(message2send, "CLIENT_DISCONNECT\n")) return 0;
	}

}

void MainClient(char *server_ip, char * server_port, char * username)//The main function of the client.
{
	char message2send[256];

	SOCKADDR_IN clientService;
	TransferResult_t RecvRes;
	TransferResult_t SendRes;
	HANDLE hThread[2];


	WSADATA wsaData;
	int j = 0;
	strcpy(global_ip, server_ip);
	global_port = atoi(server_port);

	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR)
		printf("Error at WSAStartup()\n");

	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);//create the socket

	if (m_socket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		WSACleanup();
		return;

	}
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr(SERVER_ADDRESS_STR); //Setting the IP address to connect to
	clientService.sin_port = htons(global_port); //Setting the port to connect to.

Try_Reconnect://if the connection to the server didnt success
	if (connect(m_socket, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR) {
		printf("Failed connecting to server on %s %s\nChoose what to do next\n1.Try to reconnect\n2.Exit\n", server_ip, server_port);
		gets_s(message_from_keyboard, sizeof(message_from_keyboard));
		if (STRINGS_ARE_EQUAL(message_from_keyboard, "1"))
			goto Try_Reconnect;
		WSACleanup();
		goto Client_Cleanup;
	}
	else {//if the connection succeeded
		printf("Connected to server on %s %s\n", server_ip, username);
		sprintf(message2send, "CLIENT_REQUEST:%s\n", username);
		SendRes = SendString(message2send, m_socket);


		if (SendRes == TRNS_FAILED)
		{
			printf("Failed connection to server on %s:%s.\n Choose what to do next:\n1.Try to reconnect\n2.Exit\n", server_ip, server_port);
			gets_s(message_from_keyboard, sizeof(message_from_keyboard));
			if (STRINGS_ARE_EQUAL(message_from_keyboard, "1"))
				goto Try_Reconnect;
			else
				return 0x555;
		}
	}

	char *Acceptedmessage = NULL;
	int i = 0;


	RecvRes = ReceiveString(&Acceptedmessage, m_socket);

	if (RecvRes == TRNS_FAILED)
	{

		printf("Failed connection to server on %s:%s.\n Choose what to do next:\n1.Try to reconnect\n2.Exit\n",server_ip, server_port);
		gets_s(message_from_keyboard, sizeof(message_from_keyboard));
		if (STRINGS_ARE_EQUAL(message_from_keyboard, "1"))
			goto Try_Reconnect;
		else
			return 0x555;
	}

	else if (RecvRes == TRNS_DISCONNECTED)
	{

		printf("Failed connection to server on %s:%s.\n Choose what to do next:\n1.Try to reconnect\n2.Exit\n");
		gets_s(message_from_keyboard, sizeof(message_from_keyboard));
		if (STRINGS_ARE_EQUAL(message_from_keyboard, "1"))
			goto Try_Reconnect;
		else
			return 0x555;
	}


	if STRINGS_ARE_EQUAL(Acceptedmessage, "SERVER_APPROVED\n") {
	}

	t_message *received_message_struct = NULL;

	while (1) {
		if (Acceptedmessage[i] == '\n') break;
		i++;
	}

	received_message_struct = DivideMessageToCategories(Acceptedmessage, i);//if the server denied tge connection
	if STRINGS_ARE_EQUAL(Acceptedmessage, "SERVER_DENIED\n") {
		printf("Server on %s denied the connection.\n The reason is:%s \n Choose what to do next:\n1.Try to reconnect\n2.Exit\n", server_ip, received_message_struct->params[0]);
		gets_s(message_from_keyboard, sizeof(message_from_keyboard));
		if (STRINGS_ARE_EQUAL(message_from_keyboard, "1"))
			goto Try_Reconnect;
		else
			return 0x555;
	}


	hThread[0] = CreateThread(//create 2 threads
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)SendDataThread,
		NULL,
		0,
		NULL
	);
	hThread[1] = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)RecvDataThread,
		NULL,
		0,
		NULL
	);



	WaitForMultipleObjects(2, hThread, FALSE, INFINITE);

	TerminateThread(hThread[0], 0x555);
	TerminateThread(hThread[1], 0x555);

	CloseHandle(hThread[0]);
	CloseHandle(hThread[1]);

Client_Cleanup:

	closesocket(m_socket);

	WSACleanup();

	return;
}