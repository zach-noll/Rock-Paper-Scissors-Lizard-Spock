
#include "MainClient.h"
#define MAX_SERVER_IP   16
#define MAX_SERVER_PORT 5
#define MAX_USERNAME_LEN 21

int main(int argc, char* argv[])
{
	if (argc != 4) {
		printf("USAGE: <client.exe> <ip of server> <port number> <name of client>");
		Sleep(10000);
	}
	//variables
	char server_ip[MAX_SERVER_IP], server_port[MAX_SERVER_PORT], username[MAX_USERNAME_LEN];

	//extract server ip, server port, username
	strcpy(server_ip, argv[1]);
	strcpy(server_port, argv[2]);
	strcpy(username, argv[3]);

	MainClient(server_ip, server_port, username);
}