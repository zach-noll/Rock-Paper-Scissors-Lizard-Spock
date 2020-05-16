
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS


#include "HardCoded.h"
#include "Functions.h"

/************ Main **********************/
void main(int argc, char *argv[]) {
	//All threads and main should exit with 0 if successful not 0 if failed.
	if (argc != 2) {
		printf("USAGE: <server.exe> <port number>");
		Sleep(10000);
		return;
	}

	MainServer(argc, argv);
	return;
}
