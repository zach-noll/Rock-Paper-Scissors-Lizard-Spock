#pragma once
#include "HardCoded.h"

extern DWORD WINAPI KeyboardInputThread(SOCKET *main_socket);
extern DWORD WINAPI ListenThread(SOCKET *main_socket);
extern DWORD WINAPI ClientThread(SOCKET *t_socket);

extern TransferResult_t SendBuffer(const char* Buffer, int BytesToSend, SOCKET sd);
extern TransferResult_t SendString(const char *Str, SOCKET sd);
extern TransferResult_t ReceiveBuffer(char* OutputBuffer, int BytesToReceive, SOCKET sd);
extern TransferResult_t ReceiveString(char** OutputStrPtr, SOCKET sd);

extern int FindFirstUnusedThreadSlot();
extern void CleanupWorkerThreads();
extern int ConvertMoveToNum(char *player_move);
extern char *ConvertNumToMove(int player_move_num);
extern char *BuildMessageToSend(t_message *msg_struct);
extern void FreeMessageStructMemory(t_message *msg_struct);
extern int FindMessageLength(char *accepted_msg);
extern t_message *DivideMessageToCategories(char *msg_2_divide, int msg_len);
extern HANDLE GetNameEvent(int reset, int initial_state, char event_name[20]);
extern int FindWinner(int player1_move_number, int player2_move_number);
extern int DoesFileExist(const char *fname);
extern int AllocateAndSend(char *message2allocate, SOCKET transfer_socket);
extern BOOL CloseHandles(HANDLE handle1, HANDLE handle2, HANDLE handle3, HANDLE handle4, HANDLE handle5, HANDLE handle6);