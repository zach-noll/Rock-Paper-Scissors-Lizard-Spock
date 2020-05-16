#pragma once

#define SERVER_ADDRESS_STR "127.0.0.1"
#define NUM_OF_WORKER_THREADS 2


#define SERVER_DISCONNECT -2
#define EXIT_CODE 0x55

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <winsock2.h>
#include <Windows.h>
#include <time.h>
#pragma comment(lib, "ws2_32.lib")

typedef struct {
	char *msg;
	char **params;
	int msg_len;
	int num_of_params;
}t_message;

typedef enum { TRNS_FAILED, TRNS_DISCONNECTED, TRNS_SUCCEEDED } TransferResult_t;