/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: nw_info_converter.cpp - A network infomation converter
--
-- PROGRAM: nw-info-converter.exe
--
-- FUNCTIONS:
-- int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance, LPSTR lspszCmdParam, int nCmdShow)
-- LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
-- BOOL CALLBACK htoip_convert(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
-- BOOL CALLBACK iptoh_convert(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
-- BOOL CALLBACK ptosv_convert(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
-- BOOL CALLBACK svtop_convert(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
--
-- DATE: January 21, 2019
--
-- REVISIONS:
--
-- DESIGNER: Keishi Asai
--
-- PROGRAMMER: Keishi Asai
--
-- NOTES:
-- This program is a simple network information converter which can perform the following tasks.
-- 1. Resolve a user specified host name into a IP address
-- 2. Resolve a user specified IP address into an host name(s)
-- 3. Resolve a user specified service name/protocol into its port number
-- 4. Resolve a user specified port number/protocol into its service name
----------------------------------------------------------------------------------------------------------------------*/
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define STRICT
#define PORT 5150
#define DATA_BUFSIZE 61440

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <list>
#include "resource.h"
#include "resource1.h"
#include "resource2.h"
#include "resource3.h"
#include "resource4.h"
#include "resource5.h"
#include "resource6.h"
#include "custom_message.h"
#include "data_exchanger.h"
#pragma comment(lib, "Ws2_32.lib")

const char Name[] = "Network informatin Converter";
const LPCTSTR helpMessage =
	"Network information converter resolves a given network information\n"
	"to the associated information like Host name to IP address vice versa.\n"
	"Choose the task to perform from the menu items, \n"
	"and put necessary information on the dialogue boxes.";
const LPCTSTR helpCaption = "Help";

WNDCLASSEX Wcl;
HWND hwnd;
const int OUTPUT_BUF_SIZE = 512;
char result_buf[MAXGETHOSTSTRUCT];



// Server listen
typedef struct _SOCKET_INFORMATION {
	CHAR Buffer[DATA_BUFSIZE];
	WSABUF DataBuf;
	SOCKET Socket;
	DWORD BytesSEND;
	DWORD BytesRECV;
} SOCKET_INFORMATION, *LPSOCKET_INFORMATION;

typedef struct svparams {
	int packet_size;
	int exp_packet_num;
};

//typedef struct packet {
//	int id;
//	int packet_size;
//	int 

//} packet;
// ll
//typedef struct node {
//	char * data;
//	struct node * next;
//} node_t;
// ll

BOOL CreateSocketInformation(SOCKET s);
void FreeSocketInformation(DWORD Event);

DWORD EventTotal = 0;
WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
LPSOCKET_INFORMATION SocketArray[WSA_MAXIMUM_WAIT_EVENTS];
// Server listen

//void append_node(node_t * head, char * data, int data_size) {
//	node_t * current = head;
//
//	if (current == NULL) 
//	{
//		current = (node_t*)malloc(sizeof(node_t));
//		memcpy(current->data, data, data_size);
//		current->next = NULL;
//	}
//	else 
//	{
//		while (current->next != NULL) {
//			current = current->next;
//		}
//
//		/* now we can add a new variable */
//		current->next = (node_t*)malloc(sizeof(node_t));
//		memcpy(current->next->data, data, data_size);
//		current->next->next = NULL;
//	}
//}

//void remove_first_node(node_t ** head) {
//	node_t * next_node = NULL;
//
//	if (*head == NULL) {
//		return;
//	}
//
//	next_node = (*head)->next;
//	free(*head);
//	*head = next_node;
//}
//
//int packetizeDataFromFile(char *filename) {
//
//	return 1;
//}
//
//void packetizeData(int packet_size, int num_packets) {
//	int i;
//	int j;
//	int HEADER_SIZE = 24;
//	char * data = (char *)malloc(packet_size);
//	node_t * head = NULL;
//	char * padding = (char *)malloc(packet_size);
//	//char *sbuf;
//	////sbuf = (char*)malloc(packet_size);
//
//	
//	//sprintf_s(data, "%d", atoi);
//
//	for (i = 0; i < packet_size-1; i++)
//	{
//		j = (i < 26) ? i : i % 26;
//		padding[i] = 'a' + j;
//	}
//	
//	for (i = 0; i < num_packets; i++) {
//		append_node(head, padding, packet_size);
//	}
//}
//

char* replace_char(char* str, char find, char replace) {
	char *current_pos = strchr(str, find);
	for (char* p = current_pos; (current_pos = strchr(str, find)) != NULL; *current_pos = replace);
	return str;
}

char* openSelectFileDialog()
{
	char filename[MAX_PATH];

	OPENFILENAME ofn;
	ZeroMemory(&filename, sizeof(filename));
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;  // If you have a window to center over, put its HANDLE here
	ofn.lpstrFilter = "Text Files\0*.txt\0Any File\0*.*\0";
	ofn.lpstrFile = filename;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = "Select a File";
	ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

	if (GetOpenFileNameA(&ofn))
	{
		//OutputDebugStringA(filename);
		//return filename;
		return _strdup(filename);
		//MessageBox(windowHandle, TEXT("NOT YET IMPLEMENTED."), TEXT("Select a File"), MB_OK);
	}
	else
	{
		switch (CommDlgExtendedError())
		{
		case CDERR_DIALOGFAILURE:
		case CDERR_FINDRESFAILURE:
		case CDERR_INITIALIZATION:
		case CDERR_LOADRESFAILURE:
		case CDERR_LOADSTRFAILURE:
		case CDERR_LOCKRESFAILURE:
		case CDERR_MEMALLOCFAILURE:
		case CDERR_MEMLOCKFAILURE:
		case CDERR_NOHINSTANCE:
		case CDERR_NOHOOK:
		case CDERR_NOTEMPLATE:
		case CDERR_STRUCTSIZE:
		case FNERR_BUFFERTOOSMALL:
		case FNERR_INVALIDFILENAME:
		case FNERR_SUBCLASSFAILURE:
		default:
			break;
		}
		return NULL;
	}
}

BOOL CreateSocketInformation(SOCKET s)
{
	LPSOCKET_INFORMATION SI;

	if ((EventArray[EventTotal] = WSACreateEvent()) == WSA_INVALID_EVENT)
	{
		printf("WSACreateEvent() failed with error %d\n", WSAGetLastError());
		return FALSE;
	}

	if ((SI = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR,
		sizeof(SOCKET_INFORMATION))) == NULL)
	{
		printf("GlobalAlloc() failed with error %d\n", GetLastError());
		return FALSE;
	}

	// Prepare SocketInfo structure for use.

	SI->Socket = s;
	SI->BytesSEND = 0;
	SI->BytesRECV = 0;

	SocketArray[EventTotal] = SI;

	EventTotal++;

	return(TRUE);
}


void FreeSocketInformation(DWORD Event)
{
	LPSOCKET_INFORMATION SI = SocketArray[Event];
	DWORD i;

	closesocket(SI->Socket);

	GlobalFree(SI);

	WSACloseEvent(EventArray[Event]);

	// Squash the socket and event arrays

	for (i = Event; i < EventTotal; i++)
	{
		EventArray[i] = EventArray[i + 1];
		SocketArray[i] = SocketArray[i + 1];
	}

	EventTotal--;
}

DWORD WINAPI runUDPServer(LPVOID tUdpParams) 
{
	SOCKET Listen;
	SOCKET Accept;
	SOCKADDR_IN InternetAddr;
	DWORD Event;
	WSANETWORKEVENTS NetworkEvents;
	WSADATA wsaData;
	DWORD Ret;
	DWORD Flags;
	DWORD RecvBytes;
	DWORD SendBytes;

	// FILE WRITE
	FILE *fp;
	char filename[] = "udptest.txt";

	//

	// For UDP
	SOCKADDR_IN sin;
	int sin_len;
	sin_len = sizeof(sin);

	// For UDP
	svparams *svp = (svparams *)tUdpParams;
	printf("%d, %d", svp->exp_packet_num, svp->packet_size);

	if ((Ret = WSAStartup(0x0202, &wsaData)) != 0)
	{
		printf("WSAStartup() failed with error %d\n", Ret);
		return TRUE;
	}

	// UDP
	//if ((Listen = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	if ((Listen = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		printf("socket() failed with error %d\n", WSAGetLastError());
		OutputDebugStringA("socket() error");
		return TRUE;
	}

	CreateSocketInformation(Listen);

	// Should not need FD_ACCEPT for UDP
	//if (WSAEventSelect(Listen, EventArray[EventTotal - 1], FD_ACCEPT | FD_CLOSE) == SOCKET_ERROR)
	if (WSAEventSelect(Listen, EventArray[EventTotal - 1], FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR)
	{
		printf("WSAEventSelect() failed with error %d\n", WSAGetLastError());
		OutputDebugStringA("WSAEventSelect() error");
		return TRUE;
	}

	InternetAddr.sin_family = AF_INET;
	InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	InternetAddr.sin_port = htons(PORT);

	OutputDebugStringA("Process1 ok");

	if (bind(Listen, (PSOCKADDR)&InternetAddr, sizeof(InternetAddr)) == SOCKET_ERROR)
	{
		printf("bind() failed with error %d\n", WSAGetLastError());
		OutputDebugStringA("bind() error");
		return TRUE;
	}

	// Should not need for UDP
	//if (listen(Listen, 5))
	//{
	//	printf("listen() failed with error %d\n", WSAGetLastError());
	//	OutputDebugStringA("listen() error");
	//	return TRUE;
	//}

	while (TRUE)
	{
		// Wait for one of the sockets to receive I/O notification and 
		if ((Event = WSAWaitForMultipleEvents(EventTotal, EventArray, FALSE,
			WSA_INFINITE, FALSE)) == WSA_WAIT_FAILED)
		{
			printf("WSAWaitForMultipleEvents failed with error %d\n", WSAGetLastError());
			OutputDebugStringA("WSAWaitForMultipleEvents error");
			return TRUE;
		}


		if (WSAEnumNetworkEvents(SocketArray[Event - WSA_WAIT_EVENT_0]->Socket, EventArray[Event -
			WSA_WAIT_EVENT_0], &NetworkEvents) == SOCKET_ERROR)
		{
			printf("WSAEnumNetworkEvents failed with error %d\n", WSAGetLastError());
			OutputDebugStringA("WSAEnum error");
			return TRUE;
		}


		// Should not need for UDP
		//if (NetworkEvents.lNetworkEvents & FD_ACCEPT)
		//{
		//	if (NetworkEvents.iErrorCode[FD_ACCEPT_BIT] != 0)
		//	{
		//		printf("FD_ACCEPT failed with error %d\n", NetworkEvents.iErrorCode[FD_ACCEPT_BIT]);
		//		OutputDebugStringA("FD_ACCEPT error");
		//		break;
		//	}

		//	if ((Accept = accept(SocketArray[Event - WSA_WAIT_EVENT_0]->Socket, NULL, NULL)) == INVALID_SOCKET)
		//	{
		//		printf("accept() failed with error %d\n", WSAGetLastError());
		//		OutputDebugStringA("accept() error");
		//		break;
		//	}

		//	if (EventTotal > WSA_MAXIMUM_WAIT_EVENTS)
		//	{
		//		printf("Too many connections - closing socket.\n");
		//		OutputDebugStringA("too many con error");
		//		closesocket(Accept);
		//		break;
		//	}

		//	CreateSocketInformation(Accept);

		//	if (WSAEventSelect(Accept, EventArray[EventTotal - 1], FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR)
		//	{
		//		printf("WSAEventSelect() failed with error %d\n", WSAGetLastError());
		//		OutputDebugStringA("WSAEventSelect2() error");
		//		return TRUE;
		//	}

		//	printf("Socket %d connected\n", Accept);
		//	OutputDebugStringA("Connected");
		//}


		// Try to read and write data to and from the data buffer if read and write events occur.

		if (NetworkEvents.lNetworkEvents & FD_READ ||
			NetworkEvents.lNetworkEvents & FD_WRITE)
		{
			if (NetworkEvents.lNetworkEvents & FD_READ &&
				NetworkEvents.iErrorCode[FD_READ_BIT] != 0)
			{
				printf("FD_READ failed with error %d\n", NetworkEvents.iErrorCode[FD_READ_BIT]);
				OutputDebugStringA("FD_READ error");
				break;
			}

			if (NetworkEvents.lNetworkEvents & FD_WRITE &&
				NetworkEvents.iErrorCode[FD_WRITE_BIT] != 0)
			{
				printf("FD_WRITE failed with error %d\n", NetworkEvents.iErrorCode[FD_WRITE_BIT]);
				OutputDebugStringA("FD_WRITE error");
				break;
			}

			LPSOCKET_INFORMATION SocketInfo = SocketArray[Event - WSA_WAIT_EVENT_0];

			// Read data only if the receive buffer is empty.

			if (SocketInfo->BytesRECV == 0)
			{
				SocketInfo->DataBuf.buf = SocketInfo->Buffer;
				SocketInfo->DataBuf.len = DATA_BUFSIZE;
				
				Flags = 0;
				if (WSARecvFrom(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes,
					&Flags, (SOCKADDR *)&sin, &sin_len, NULL, NULL) == SOCKET_ERROR)
				{
					if (WSAGetLastError() != WSAEWOULDBLOCK)
					{
						printf("WSARecv() failed with error %d\n", WSAGetLastError());
						OutputDebugStringA("WSARecv() error");
						FreeSocketInformation(Event - WSA_WAIT_EVENT_0);
						return TRUE;
					}
				}
				else
				{
					SocketInfo->BytesRECV = RecvBytes;
				}
			}

			// Write buffer data if it is available.
			if (SocketInfo->BytesRECV > SocketInfo->BytesSEND)
			{
				SocketInfo->DataBuf.buf = SocketInfo->Buffer + SocketInfo->BytesSEND;
				SocketInfo->DataBuf.len = SocketInfo->BytesRECV - SocketInfo->BytesSEND;

				fopen_s(&fp, filename, "a+");
				fputs(SocketInfo->DataBuf.buf, fp);
				fclose(fp);

				if (WSASendTo(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &SendBytes, 0,
					(SOCKADDR *)&sin, sin_len, NULL, NULL) == SOCKET_ERROR)
				{
					if (WSAGetLastError() != WSAEWOULDBLOCK)
					{
						printf("WSASend() failed with error %d\n", WSAGetLastError());
						OutputDebugStringA("WSASend() error");
						FreeSocketInformation(Event - WSA_WAIT_EVENT_0);
						return TRUE;
					}

					// A WSAEWOULDBLOCK error has occured. An FD_WRITE event will be posted
					// when more buffer space becomes available
				}
				else
				{
					SocketInfo->BytesSEND += SendBytes;

					if (SocketInfo->BytesSEND == SocketInfo->BytesRECV)
					{
						SocketInfo->BytesSEND = 0;
						SocketInfo->BytesRECV = 0;
					}
				}
			}
		}

		if (NetworkEvents.lNetworkEvents & FD_CLOSE)
		{
			if (NetworkEvents.iErrorCode[FD_CLOSE_BIT] != 0)
			{
				printf("FD_CLOSE failed with error %d\n", NetworkEvents.iErrorCode[FD_CLOSE_BIT]);
				OutputDebugStringA("FD_CLOSE error");
				break;
			}

			printf("Closing socket information %d\n", SocketArray[Event - WSA_WAIT_EVENT_0]->Socket);
			OutputDebugStringA("Closing sock");

			FreeSocketInformation(Event - WSA_WAIT_EVENT_0);
		}
	}
}

DWORD WINAPI runTCPServer(LPVOID tTcpParams) 
{
	SOCKET Listen;
	SOCKET Accept;
	SOCKADDR_IN InternetAddr;
	DWORD Event;
	WSANETWORKEVENTS NetworkEvents;
	WSADATA wsaData;
	DWORD Ret;
	DWORD Flags;
	DWORD RecvBytes;
	DWORD SendBytes;
	int recvd_packet = 0;
	int processed_packet = 0;

	// FILE WRITE
	FILE *fp;
	char filename[] = "test.txt";
	//

	svparams *svp = (svparams *)tTcpParams;
	char *packet_buf = (char *)malloc(svp->packet_size);
	memset(packet_buf, 0, sizeof(packet_buf));
	printf("%d, %d", svp->exp_packet_num, svp->packet_size);

	if ((Ret = WSAStartup(0x0202, &wsaData)) != 0)
	{
		printf("WSAStartup() failed with error %d\n", Ret);
		return TRUE;
	}

	if ((Listen = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("socket() failed with error %d\n", WSAGetLastError());
		OutputDebugStringA("socket() error");
		return TRUE;
	}

	CreateSocketInformation(Listen);

	if (WSAEventSelect(Listen, EventArray[EventTotal - 1], FD_ACCEPT | FD_CLOSE) == SOCKET_ERROR)
	{
		printf("WSAEventSelect() failed with error %d\n", WSAGetLastError());
		OutputDebugStringA("WSAEventSelect() error");
		return TRUE;
	}

	InternetAddr.sin_family = AF_INET;
	InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	InternetAddr.sin_port = htons(PORT);

	OutputDebugStringA("Process1 ok");

	if (bind(Listen, (PSOCKADDR)&InternetAddr, sizeof(InternetAddr)) == SOCKET_ERROR)
	{
		printf("bind() failed with error %d\n", WSAGetLastError());
		OutputDebugStringA("bind() error");
		return TRUE;
	}

	if (listen(Listen, 5))
	{
		printf("listen() failed with error %d\n", WSAGetLastError());
		OutputDebugStringA("listen() error");
		return TRUE;
	}


	while (TRUE)
	{
		// Wait for one of the sockets to receive I/O notification and 

		//if ((Event = WSAWaitForMultipleEvents(EventTotal, EventArray, FALSE,
			//WSA_INFINITE, FALSE)) == WSA_WAIT_FAILED)
		if ((Event = WSAWaitForMultipleEvents(EventTotal, EventArray, FALSE,
			10000, FALSE)) == WSA_WAIT_FAILED)
		{
			printf("WSAWaitForMultipleEvents failed with error %d\n", WSAGetLastError());
			OutputDebugStringA("WSAWaitForMultipleEvents error");
			return TRUE;
		}
		if (Event == WSA_WAIT_TIMEOUT) {
			closesocket(Accept);
			WSACleanup();
			ExitThread(0);
		}

		if (WSAEnumNetworkEvents(SocketArray[Event - WSA_WAIT_EVENT_0]->Socket, EventArray[Event -
			WSA_WAIT_EVENT_0], &NetworkEvents) == SOCKET_ERROR)
		{
			printf("WSAEnumNetworkEvents failed with error %d\n", WSAGetLastError());
			OutputDebugStringA("WSAEnum error");
			return TRUE;
		}


		if (NetworkEvents.lNetworkEvents & FD_ACCEPT)
		{
			if (NetworkEvents.iErrorCode[FD_ACCEPT_BIT] != 0)
			{
				printf("FD_ACCEPT failed with error %d\n", NetworkEvents.iErrorCode[FD_ACCEPT_BIT]);
				OutputDebugStringA("FD_ACCEPT error");
				break;
			}

			if ((Accept = accept(SocketArray[Event - WSA_WAIT_EVENT_0]->Socket, NULL, NULL)) == INVALID_SOCKET)
			{
				printf("accept() failed with error %d\n", WSAGetLastError());
				OutputDebugStringA("accept() error");
				break;
			}

			if (EventTotal > WSA_MAXIMUM_WAIT_EVENTS)
			{
				printf("Too many connections - closing socket.\n");
				OutputDebugStringA("too many con error");
				closesocket(Accept);
				break;
			}

			CreateSocketInformation(Accept);

			if (WSAEventSelect(Accept, EventArray[EventTotal - 1], FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR)
			{
				printf("WSAEventSelect() failed with error %d\n", WSAGetLastError());
				OutputDebugStringA("WSAEventSelect2() error");
				return TRUE;
			}

			printf("Socket %d connected\n", Accept);
			OutputDebugStringA("Connected");
		}


		// Try to read and write data to and from the data buffer if read and write events occur.

		if (NetworkEvents.lNetworkEvents & FD_READ ||
			NetworkEvents.lNetworkEvents & FD_WRITE)
		{
			if (NetworkEvents.lNetworkEvents & FD_READ &&
				NetworkEvents.iErrorCode[FD_READ_BIT] != 0)
			{
				printf("FD_READ failed with error %d\n", NetworkEvents.iErrorCode[FD_READ_BIT]);
				OutputDebugStringA("FD_READ error");
				break;
			}

			if (NetworkEvents.lNetworkEvents & FD_WRITE &&
				NetworkEvents.iErrorCode[FD_WRITE_BIT] != 0)
			{
				printf("FD_WRITE failed with error %d\n", NetworkEvents.iErrorCode[FD_WRITE_BIT]);
				OutputDebugStringA("FD_WRITE error");
				break;
			}

			LPSOCKET_INFORMATION SocketInfo = SocketArray[Event - WSA_WAIT_EVENT_0];

			// Read data only if the receive buffer is empty.
			int tout_counter = 0;
			char estr[256];
			int wrote_index = 0;
			int remaining = svp->packet_size;

			if (SocketInfo->BytesRECV == 0)
			{
				SocketInfo->DataBuf.buf = SocketInfo->Buffer;
				//SocketInfo->DataBuf.len = DATA_BUFSIZE;
				SocketInfo->DataBuf.len = svp->packet_size;
				
				Flags = 0;
				while (remaining != 0 && tout_counter <= 5)
				{

					// Here overriding databuf without writing to file when get partial packets and go to 2nd loop
					if (WSARecv(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes,
						&Flags, NULL, NULL) == SOCKET_ERROR)
					{
						if (WSAGetLastError() != WSAEWOULDBLOCK)
						{
							printf("WSARecv() failed with error %d\n", WSAGetLastError());
							sprintf_s(estr, "WSARecv() failed with error %d\n", WSAGetLastError());
							OutputDebugStringA("WSARecv() error");
							OutputDebugString(estr);
							FreeSocketInformation(Event - WSA_WAIT_EVENT_0);
							return TRUE;
						}
					}
					else
					{
						if (RecvBytes == 0) {
							tout_counter++;
						}
						else {
							SocketInfo->BytesRECV = RecvBytes;
							remaining = remaining - SocketInfo->BytesRECV;
							SocketInfo->DataBuf.len = remaining;

							for (int i=0; i<SocketInfo->BytesRECV; i++)
							{
								packet_buf[i+wrote_index] = SocketInfo->DataBuf.buf[i];
							}
							wrote_index += SocketInfo->BytesRECV;

							tout_counter = 0;
						}
					}
				}
				recvd_packet++;
			}

			// Write buffer data if it is available.
			//if (SocketInfo->BytesRECV > SocketInfo->BytesSEND)
			if (recvd_packet > processed_packet)
			{
				//SocketInfo->DataBuf.buf = SocketInfo->Buffer + SocketInfo->BytesSEND;
				//SocketInfo->DataBuf.len = SocketInfo->BytesRECV - SocketInfo->BytesSEND;
				SocketInfo->DataBuf.buf = SocketInfo->Buffer;
				remaining = svp->packet_size;

				//fputs(SocketInfo->DataBuf.buf, fp);

				fopen_s(&fp, filename, "a+");
				fputs(packet_buf, fp);
				fclose(fp);
				memset(packet_buf, 0, sizeof(packet_buf));
				wrote_index = 0;

				//if (WSASend(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &SendBytes, 0,
				//	NULL, NULL) == SOCKET_ERROR)
				//{
				//	if (WSAGetLastError() != WSAEWOULDBLOCK)
				//	{
				//		printf("WSASend() failed with error %d\n", WSAGetLastError());
				//		OutputDebugStringA("WSASend() error");
				//		FreeSocketInformation(Event - WSA_WAIT_EVENT_0);
				//		return TRUE;
				//	}

				//	// A WSAEWOULDBLOCK error has occured. An FD_WRITE event will be posted
				//	// when more buffer space becomes available
				//}
				//else
				//{
					//SocketInfo->BytesSEND += SendBytes;

					//if (SocketInfo->BytesSEND == SocketInfo->BytesRECV)
					//{
						//SocketInfo->BytesSEND = 0;
						SocketInfo->BytesRECV = 0;
					//}
				//}
				processed_packet++;
			}
		}

		if (NetworkEvents.lNetworkEvents & FD_CLOSE)
		{
			if (NetworkEvents.iErrorCode[FD_CLOSE_BIT] != 0)
			{
				printf("FD_CLOSE failed with error %d\n", NetworkEvents.iErrorCode[FD_CLOSE_BIT]);
				OutputDebugStringA("FD_CLOSE error");
				break;
			}

			printf("Closing socket information %d\n", SocketArray[Event - WSA_WAIT_EVENT_0]->Socket);
			OutputDebugStringA("Closing sock");

			FreeSocketInformation(Event - WSA_WAIT_EVENT_0);
		}
	}
}


BOOL CALLBACK handleServerDialog(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	const int INPUT_BUF_SIZE = 256;
	char hname_buf[INPUT_BUF_SIZE];
	HANDLE hTcpRunner;
	HANDLE hUdpRunner;
	DWORD dwTcpThreadID;
	DWORD dwUdpThreadID;

	WORD wVersionRequested = MAKEWORD(2, 2);
	WSADATA wsaData;

	int packet_size;
	int exp_packet_num;
	const int BUF_SIZE = 256;
	char DEF_HOST[] = "localhost";
	const int DEF_PORT = 5150;
	const int DEF_PSIZE = 5;
	const int DEF_REPEAT_NUM = 2;
	char port_buf[INPUT_BUF_SIZE];
	char psize_buf[INPUT_BUF_SIZE];
	char times_buf[INPUT_BUF_SIZE];
	HANDLE async_handle;
	svparams *svp = (svparams*)malloc(sizeof(svp));

	WSAStartup(wVersionRequested, &wsaData);

	if (GetDlgItemText(hwndDlg, IDC_EDIT6_3, psize_buf, INPUT_BUF_SIZE) == 0) 
	{
		packet_size = DEF_PSIZE;
	}
	else {
		packet_size = atoi(psize_buf);
	}
	if (GetDlgItemText(hwndDlg, IDC_EDIT6_4, times_buf, INPUT_BUF_SIZE) == 0) 
	{
		exp_packet_num = DEF_REPEAT_NUM;
	}
	else {
		exp_packet_num = atoi(times_buf);
	}

	svp->packet_size = packet_size;
	svp->exp_packet_num = exp_packet_num;

	switch (message)
	{
		case WM_INITDIALOG:
			return TRUE;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_BUTTON6_1:
					if (IsDlgButtonChecked(hwndDlg, IDC_RADIO6_1))
					{
						OutputDebugStringA("yep");
						hTcpRunner = CreateThread(NULL, 0, runTCPServer, svp, 0, &dwTcpThreadID);
						return TRUE;
					}
					else 
					{
						//UDP
						OutputDebugStringA("not yep");
						hUdpRunner = CreateThread(NULL, 0, runUDPServer, svp, 0, &dwUdpThreadID);
						return TRUE;
					}
				case IDCANCEL:
					EndDialog(hwndDlg, (INT_PTR)0);
					return TRUE;
			}
	}
	return FALSE;
}



BOOL CALLBACK handleClientDialog(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	const int INPUT_BUF_SIZE = 256;
	const int BUF_SIZE = 256;
	char DEF_HOST[] = "localhost";
	const int DEF_PORT = 5150;
	const int DEF_PSIZE = 5;
	const int DEF_REPEAT_NUM = 2;
	char hname_buf[INPUT_BUF_SIZE];
	char port_buf[INPUT_BUF_SIZE];
	char psize_buf[INPUT_BUF_SIZE];
	char times_buf[INPUT_BUF_SIZE];
	HANDLE async_handle;

	// NEW
	char *host;
	int port;
	int packet_size;
	int repeat_num;
	SOCKET sd;
	struct sockaddr_in server;
	struct hostent	*hp;
	char *rbuf;
	char *sbuf;
	char *bp;
	char **pptr;
	int n;
	int ns;
	int bytes_to_read;
	int i;
	int j;
	char *filename;
	FILE *fp;
	char f_read_buf[1024];
	// NEW

	// Client only
	struct	sockaddr_in client;
	int client_len;
	int server_len;
	int MAXLEN = 65535;
	SYSTEMTIME stStartTime;
	SYSTEMTIME stEndTime;


	// Client only


	WORD wVersionRequested = MAKEWORD(2, 2);
	WSADATA wsaData;

	WSAStartup(wVersionRequested, &wsaData);

	switch (message)
	{
		case WM_INITDIALOG:
			return TRUE;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_BUTTON5_1:
					// Forget about this for a while
					filename = openSelectFileDialog();
					//OutputDebugString(filename);
					filename = replace_char(filename, '\\', '/');
					
					OutputDebugString(filename);
					fopen_s(&fp, filename, "r");
					if (fp == NULL)
					{
						OutputDebugStringA("Failed to read file.");
					}
					while (fgets(f_read_buf, 1024, fp) != NULL)
						OutputDebugString(f_read_buf);
					fclose(fp);

					break;
				case IDC_BUTTON5_2:
					if (GetDlgItemText(hwndDlg, IDC_EDIT5_1, hname_buf, INPUT_BUF_SIZE) == 0) 
					{
						host = DEF_HOST;
					}
					else 
					{
						host = hname_buf;
					}
					if (GetDlgItemText(hwndDlg, IDC_EDIT5_2, port_buf, INPUT_BUF_SIZE) == 0)
					{
						port = DEF_PORT;
					}
					else {
						port = atoi(port_buf);
					}
					if (GetDlgItemText(hwndDlg, IDC_EDIT5_3, psize_buf, INPUT_BUF_SIZE) == 0) 
					{
						packet_size = DEF_PSIZE;
					}
					else {
						packet_size = atoi(psize_buf);
					}
					if (GetDlgItemText(hwndDlg, IDC_EDIT5_4, times_buf, INPUT_BUF_SIZE) == 0) 
					{
						repeat_num = DEF_REPEAT_NUM;
					}
					else {
						repeat_num = atoi(times_buf);
					}
					OutputDebugString(hname_buf);
					OutputDebugString(port_buf);
					OutputDebugString(psize_buf);
					OutputDebugString(times_buf);
					
					sbuf = (char*)malloc(packet_size);
					rbuf = (char*)malloc(packet_size);

					
					if (IsDlgButtonChecked(hwndDlg, IDC_RADIO5_1)) {
						// TCP code here
						// Create the socket
						if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
						{
							perror("Cannot create socket");
							exit(1);
						}
						// Initialize and set up the address structure
						memset((char *)&server, 0, sizeof(struct sockaddr_in));
						server.sin_family = AF_INET;
						server.sin_port = htons(port);
						if ((hp = gethostbyname(host)) == NULL)
						{
							fprintf(stderr, "Unknown server address\n");
							exit(1);
						}

						// Copy the server address
						memcpy((char *)&server.sin_addr, hp->h_addr, hp->h_length);

						// Connecting to the server
						if (connect(sd, (struct sockaddr *)&server, sizeof(server)) == -1)
						{
							fprintf(stderr, "Can't connect to server\n");
							perror("connect");
							OutputDebugStringA("muripo");
							exit(1);
						}
						//printf("Connected:    Server Name: %s\n", hp->h_name);
						OutputDebugStringA("Connected");
						pptr = hp->h_addr_list;
						//printf("\t\tIP Address: %s\n", inet_ntoa(server.sin_addr));
						//printf("Transmiting:\n");
						memset((char *)sbuf, 0, sizeof(sbuf));
						//gets_s(sbuf); // get user's text
						// data	is a, b, c, ..., z, a, b,...
						//for (i = 0; i < BUF_SIZE-1; i++)
						for (i = 0; i < packet_size-1; i++)
						{
							j = (i < 26) ? i : i % 26;
							sbuf[i] = 'a' + j;
						}
						sbuf[i] = '\0';
						OutputDebugString(sbuf);
						OutputDebugStringA("otawa1\n");

						for (i = 0; i < repeat_num; i++) {
							// Transmit data through the socket
							ns = send(sd, sbuf, packet_size, 0);
							//printf("Receive:\n");
						}

						bp = rbuf;
						bytes_to_read = packet_size;

						// client makes repeated calls to recv until no more data is expected to arrive.
						while (bytes_to_read != 0 && (n = recv(sd, bp, bytes_to_read, 0)) < packet_size)
						{
							bp += n;
							bytes_to_read -= n;
							OutputDebugStringA("reding..");
							if (n == 0) {
								OutputDebugStringA("otawa!");
								break;
							}
						}
						//printf("%s\n", rbuf);
						OutputDebugString(rbuf);
						closesocket(sd);
						OutputDebugStringA("yep");
					}
					else {
						// UDP code here
							// Create a datagram socket
						if ((sd = socket(PF_INET, SOCK_DGRAM, 0)) == -1)
						{
							perror("Can't create a socket\n");
							exit(1);
						}

						// Store server's information
						memset((char *)&server, 0, sizeof(server));
						server.sin_family = AF_INET;
						server.sin_port = htons(port);

						if ((hp = gethostbyname(host)) == NULL)
						{
							fprintf(stderr, "Can't get server's IP address\n");
							exit(1);
						}
						//strcpy((char *)&server.sin_addr, hp->h_addr);
						memcpy((char *)&server.sin_addr, hp->h_addr, hp->h_length);

						// Bind local address to the socket
						memset((char *)&client, 0, sizeof(client));
						client.sin_family = AF_INET;
						client.sin_port = htons(0);  // bind to any available port
						client.sin_addr.s_addr = htonl(INADDR_ANY);

						if (bind(sd, (struct sockaddr *)&client, sizeof(client)) == -1)
						{
							perror("Can't bind name to socket");
							exit(1);
						}
						// Find out what port was assigned and print it
						client_len = sizeof(client);
						if (getsockname(sd, (struct sockaddr *)&client, &client_len) < 0)
						{
							perror("getsockname: \n");
							exit(1);
						}
						printf("Port aasigned is %d\n", ntohs(client.sin_port));

						if (packet_size > MAXLEN)
						{
							fprintf(stderr, "Data is too big\n");
							exit(1);
						}

						// data	is a, b, c, ..., z, a, b,...
						for (i = 0; i < packet_size; i++)
						{
							j = (i < 26) ? i : i % 26;
							sbuf[i] = 'a' + j;
						}

						// Get the start time
						GetSystemTime(&stStartTime);

						// transmit data
						server_len = sizeof(server);
						if (sendto(sd, sbuf, packet_size, 0, (struct sockaddr *)&server, server_len) == -1)
						{
							perror("sendto failure");
							exit(1);
						}

						// receive data
						if (recvfrom(sd, rbuf, MAXLEN, 0, (struct sockaddr *)&server, &server_len) < 0)
						{
							perror(" recvfrom error");
							exit(1);
						}

						//Get the end time and calculate the delay measure
						GetSystemTime(&stEndTime);
//						printf("Round-trip delay = %ld ms.\n", delay(stStartTime, stEndTime));

						if (strncmp(sbuf, rbuf, packet_size) != 0)
							printf("Data is corrupted\n");

						closesocket(sd);
						OutputDebugStringA("not yep");
					}

//					GetDlgItemText(hwndDlg, IDC_EDIT5_3, psize_buf, INPUT_BUF_SIZE);
//					GetDlgItemText(hwndDlg, IDC_EDIT5_4, times_buf, INPUT_BUF_SIZE);
//					OutputDebugString(psize_buf);
//					OutputDebugString(times_buf);

					//async_handle = WSAAsncGetHostByName(hwnd, WM_GET_IP_BY_NAME_DONE, hname_buf, result_buf, MAXGETHOSTSTRUCT);
					//if (async_handle == 0)
					//{
					//	MessageBox(hwnd, "Error in get host by name", "Error", MB_OK);
					//}
					//else 
					//{
					//	EndDialog(hwndDlg, (INT_PTR)1);
					//}
					return TRUE;
				case IDCANCEL:
					EndDialog(hwndDlg, (INT_PTR)0);
					return TRUE;
			}
	}
	return FALSE;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: WinMain
--
-- DATE: January 21, 2019
--
-- REVISIONS:
--
-- DESIGNER: Keishi Asai
--
-- PROGRAMMER: Keishi Asai
--
-- INTERFACE: int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance,
--							LPSTR lspszCmdParam, int nCmdShow)
--							HINSTANCE hInst: A handle to the current instance of the application
--							HINSTANCE hprevInstance: A handle to the previous instance of the application
--							LPSTR lspszCmdParam: The command line for the application, excluding the program name.
--							int nCmdShow: Controls how the window is to be shown
-- RETURNS: int
--
-- NOTES:
-- Generates the application window and configure it.
----------------------------------------------------------------------------------------------------------------------*/
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance, LPSTR lspszCmdParam, int nCmdShow)
{
	MSG Msg;

	Wcl.cbSize = sizeof(WNDCLASSEX);
	Wcl.style = CS_HREDRAW | CS_VREDRAW;
	Wcl.hIcon = LoadIcon(NULL, IDI_APPLICATION); // large icon 
	Wcl.hIconSm = NULL; // use small version of large icon
	Wcl.hCursor = LoadCursor(NULL, IDC_ARROW);  // cursor style
	Wcl.lpfnWndProc = WndProc;
	Wcl.hInstance = hInst;
	Wcl.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); //white background
	Wcl.lpszClassName = Name;
	Wcl.lpszMenuName = "MYMENU"; // The menu Class
	Wcl.cbClsExtra = 0;      // no extra memory needed
	Wcl.cbWndExtra = 0;

	if (!RegisterClassEx(&Wcl))
		return 0;

	hwnd = CreateWindow(Name, Name, WS_OVERLAPPEDWINDOW, 10, 10,
		600, 400, NULL, NULL, hInst, NULL);

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	while (GetMessage(&Msg, NULL, 0, 0))
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	return Msg.wParam;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: WinProc
--
-- DATE: January 21, 2019
--
-- REVISIONS:
--
-- DESIGNER: Keishi Asai
--
-- PROGRAMMER: Keishi Asai
--
-- INTERFACE: LRESULT CALLBACK WndProc(HWND hwnd, UINT Message,
--										WPARAM wParam, LPARAM lParam)
--							HWND hwnd: A handle to a window.
--							UINT Message: The message processed by WndProc
--							WPARAM wParam: Additional message information
--							LPARAM lParam: Additional message information
-- RETURNS: LRESULT
--
-- NOTES:
-- Window event procedure. Process the messages dispatched from WinMain and Async functions
----------------------------------------------------------------------------------------------------------------------*/
LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT paintstruct;
	static unsigned k = 0;
	hostent* h_ip_query_result;
	servent* p_sv_query_result;
	char** p;
	char port_output[OUTPUT_BUF_SIZE];
	char service_output[OUTPUT_BUF_SIZE];
	char port_number[OUTPUT_BUF_SIZE];

	switch (Message)
	{
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case DE_CLIENT:
					DialogBox(Wcl.hInstance, MAKEINTRESOURCE(IDD_FORMVIEW5), hwnd, handleClientDialog);
					break;
				case DE_SERVER:
					DialogBox(Wcl.hInstance, MAKEINTRESOURCE(IDD_FORMVIEW6), hwnd, handleServerDialog);
					break;
				case IDM_HELP:
					MessageBox(hwnd, helpMessage, helpCaption, MB_OK);
					break;
				case IDM_EXIT:
					PostMessage(
						(HWND)hwnd,
						WM_DESTROY,
						NULL,
						NULL
					);
				break;
			}
			break;
		case WM_GET_IP_BY_NAME_DONE:
			if (WSAGETASYNCERROR(lParam) != 0) {
				MessageBox(hwnd, "Failed to resolve the hostname", NULL, MB_ICONWARNING);
			}
			else 
			{
				hdc = GetDC(hwnd);
				h_ip_query_result = (hostent*)result_buf;

				// Display all results received 
				for (p = h_ip_query_result->h_addr_list; *p != 0; p++)
				{
					struct in_addr in;
					memcpy(&in.s_addr, *p, sizeof(in.s_addr));

					TextOut(hdc, 0, 20 * k, h_ip_query_result->h_name, strlen(h_ip_query_result->h_name));
					k++; 
					TextOut(hdc, 0, 20 * k, inet_ntoa(in), strlen(inet_ntoa(in)));
					k++; 
				}
			}
			WSACleanup();
			break;
		case WM_GET_HOST_BY_IP_DONE:
			if (WSAGETASYNCERROR(lParam) != 0) {
				MessageBox(hwnd, "Failed to resolve the IP address", NULL, MB_ICONWARNING);
			}
			else
			{
				hdc = GetDC(hwnd);
				h_ip_query_result = (hostent*)result_buf;

				// Display all results received 
				for (p = h_ip_query_result->h_addr_list; *p != 0; p++)
				{
					struct in_addr in;

					memcpy(&in.s_addr, *p, sizeof(in.s_addr));
					TextOut(hdc, 0, 20 * k, inet_ntoa(in), strlen(inet_ntoa(in)));
					k++;
					TextOut(hdc, 0, 20 * k, h_ip_query_result->h_name, strlen(h_ip_query_result->h_name));
					k++;
				}
			}
			WSACleanup();
			break;
		case WM_GET_SERVICE_BY_PORT:
			if (WSAGETASYNCERROR(lParam) != 0) {
				MessageBox(hwnd, "Failed to resolve the port number", NULL, MB_ICONWARNING);
			}
			else
			{
				hdc = GetDC(hwnd);
				p_sv_query_result = (servent*)result_buf;

				// Display the information received
				sprintf_s(port_output, OUTPUT_BUF_SIZE, "Port Number: %hu, Protocol: %s", ntohs(p_sv_query_result->s_port), p_sv_query_result->s_proto);
				TextOut(hdc, 0, 20 * k, port_output, strlen(port_output));
				k++;
				TextOut(hdc, 0, 20 * k, p_sv_query_result->s_name, strlen(p_sv_query_result->s_name));
				k++;
				OutputDebugString(((servent*)result_buf)->s_name);
			}
			WSACleanup();
			break;
		case WM_GET_PORT_BY_SERVICE:
			if (WSAGETASYNCERROR(lParam) != 0) {
				MessageBox(hwnd, "Failed to resolve the service name", NULL, MB_ICONWARNING);
			}
			else
			{
				hdc = GetDC(hwnd);
				p_sv_query_result = (servent*)result_buf;

				// Display the information received
				sprintf_s(service_output, OUTPUT_BUF_SIZE, "Service Name: %s, Protocol: %s", p_sv_query_result->s_name, p_sv_query_result->s_proto);
				TextOut(hdc, 0, 20 * k, service_output, strlen(service_output));
				k++;
				sprintf_s(port_number, OUTPUT_BUF_SIZE, "%hu", ntohs(p_sv_query_result->s_port));
				TextOut(hdc, 0, 20 * k, port_number, strlen(port_number));
				k++;
			}
			WSACleanup();
			break;
		case WM_PAINT:		// Process a repaint message
			hdc = BeginPaint(hwnd, &paintstruct); // Acquire DC
			EndPaint(hwnd, &paintstruct); // Release DC
			break;
		case WM_DESTROY:	// Terminate program
			WSACleanup();
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	return 0;
}
