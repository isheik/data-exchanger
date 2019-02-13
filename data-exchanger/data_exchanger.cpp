/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: data_exchanger.cpp - A data exchange application
--
-- PROGRAM: data-exchanger.exe
--
-- FUNCTIONS:
-- int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance, LPSTR lspszCmdParam, int nCmdShow)
-- LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
-- void window_printline(char* str, int str_size)
-- long delay(SYSTEMTIME t1, SYSTEMTIME t2)
-- char* replace_char(char* str, char find, char replace)
-- char* openSelectFileDialog()
-- BOOL CreateSocketInformation(SOCKET s)
-- void FreeSocketInformation(DWORD Event)
-- DWORD WINAPI runUDPServer(LPVOID tUdpParams)
-- DWORD WINAPI runTCPServer(LPVOID tTcpParams)
-- INT_PTR CALLBACK handleServerDialog(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
-- INT_PTR CALLBACK handleClientDialog(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
--
-- DATE: February 12, 2019
--
-- REVISIONS:
--
-- DESIGNER: Keishi Asai
--
-- PROGRAMMER: Keishi Asai
--
-- NOTES:
-- This program is a simple data send/receive client-server program can perform the follwoing tasks.
-- 1. Send packets by TCP
-- 2. Send a file by TCP
-- 3. Send packets by UDP
-- 4. Send packets by UDP
----------------------------------------------------------------------------------------------------------------------*/
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define STRICT
#define PORT 5150
#define DATA_BUFSIZE 61440
#define DISP_LINE_BUFSIZE 512

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
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

typedef struct _SOCKET_INFORMATION {
	CHAR Buffer[DATA_BUFSIZE];
	WSABUF DataBuf;
	SOCKET Socket;
	DWORD BytesSEND;
	DWORD BytesRECV;
} SOCKET_INFORMATION, *LPSOCKET_INFORMATION;

typedef struct svparams {
	int port;
	int packet_size;
	int exp_packet_num;
} svparams;

const char Name[] = "Data Exchanger";
const LPCTSTR helpMessage =
	"Data Exchanger is a client-server program to send/receive packets or files.\n"
	"From the Start menu, the role of the program can be chosen from Client or Server.\n"
	"Run the program as server on a host, and connect to it from another host using this program as client.\n";
const LPCTSTR helpCaption = "Help";

WNDCLASSEX Wcl;
HWND hwnd;

char result_buf[MAXGETHOSTSTRUCT];
char disp_line_buf[DISP_LINE_BUFSIZE];
char quit_msg[DISP_LINE_BUFSIZE];
char stat_line1[DISP_LINE_BUFSIZE];
char stat_line2[DISP_LINE_BUFSIZE];

BOOL CreateSocketInformation(SOCKET s);
void FreeSocketInformation(DWORD Event);

DWORD EventTotal = 0;
WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
LPSOCKET_INFORMATION SocketArray[WSA_MAXIMUM_WAIT_EVENTS];

char *selected_filename = NULL;
int selected_filesize;
FILE *selected_fp = NULL;
int num_packets;

static unsigned k = 0;

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: window_printline
--
-- DATE: February 12, 2019
--
-- REVISIONS:
--
-- DESIGNER: Keishi Asai
--
-- PROGRAMMER: Keishi Asai
--
-- INTERFACE: void window_printline(char* str, int str_size)
--							char* str : a string to display on the window
--							int str_size: the size of the string
-- RETURNS: void
--
-- NOTES:
-- A wrapper function to display text on the window.
----------------------------------------------------------------------------------------------------------------------*/
void window_printline(char* str, int str_size) 
{
	HDC hdc;
	char clearText[] = "                                                                                                                                                                                                          ";
	int max_row = 16;

	hdc = GetDC(hwnd);

	if (k >= max_row) {
		for (int i = 0; i < max_row; i++) {
			TextOut(hdc, 0, 20 * i, clearText, strlen(clearText));
		}
		
		k = 0;
	}
	// Display the information received
	TextOut(hdc, 0, 20 * k, str, str_size);
	k++;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: delay
--
-- DATE: February 12, 2019
--
-- REVISIONS:
--
-- DESIGNER: Keishi Asai
--
-- PROGRAMMER: Keishi Asai
--
-- INTERFACE: long delay(SYSTEMTIME t1, SYSTEMTIME t2)
--							SYSTEMTIME t1 : the start time to measure elapsed time
--							SYSTEMTIME t2 : the end time to measure elapsed time
-- RETURNS: long
--
-- NOTES:
-- This function compute the delay between t1 and t2 in milliseconds
----------------------------------------------------------------------------------------------------------------------*/
long delay(SYSTEMTIME t1, SYSTEMTIME t2)
{
	long d;

	d = (t2.wSecond - t1.wSecond) * 1000;
	d += (t2.wMilliseconds - t1.wMilliseconds);
	return(d);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: replace_char
--
-- DATE: February 12, 2019
--
-- REVISIONS:
--
-- DESIGNER: Keishi Asai
--
-- PROGRAMMER: Keishi Asai
--
-- INTERFACE: char* replace_char(char* str, char find, char replace)
--							char* str : the string contains a certain character to replace
--							char find : the character to be replaced
--							char replace : the character to be used insted of the found character
-- RETURNS: char*
--
-- NOTES:
-- Replace a certain character occurances in a string
----------------------------------------------------------------------------------------------------------------------*/
char* replace_char(char* str, char find, char replace) 
{
	char *current_pos = strchr(str, find);
	for (char* p = current_pos; (current_pos = strchr(str, find)) != NULL; *current_pos = replace);
	return str;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: openSelectFileDialog
--
-- DATE: February 12, 2019
--
-- REVISIONS:
--
-- DESIGNER: Keishi Asai
--
-- PROGRAMMER: Keishi Asai
--
-- INTERFACE: char* openSelectFileDialog()
--
-- RETURNS: char*
--
-- NOTES:
-- Get a file path to send
----------------------------------------------------------------------------------------------------------------------*/
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
		return _strdup(filename);
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: CreateSocketInformation
--
-- DATE: February 12, 2019
--
-- REVISIONS:
--
-- DESIGNER: Keishi Asai
--
-- PROGRAMMER: Keishi Asai
--
-- INTERFACE: BOOL CreateSocketInformation(SOCKET s)
--							SOCKET s : socket to create information
--
-- RETURNS: BOOL
--
-- NOTES:
-- Create information associated with sockets
----------------------------------------------------------------------------------------------------------------------*/
BOOL CreateSocketInformation(SOCKET s)
{
	LPSOCKET_INFORMATION SI;

	if ((EventArray[EventTotal] = WSACreateEvent()) == WSA_INVALID_EVENT)
	{
		return FALSE;
	}

	if ((SI = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR,
		sizeof(SOCKET_INFORMATION))) == NULL)
	{
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: FreeSocketInformation
--
-- DATE: February 12, 2019
--
-- REVISIONS:
--
-- DESIGNER: Keishi Asai
--
-- PROGRAMMER: Keishi Asai
--
-- INTERFACE: void FreeSocketInformation(DWORD Event)
--							DWORD Event : Index to free socket information
--
-- RETURNS: BOOL
--
-- NOTES:
-- Close socket and release the associated information
----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: runUDPServer
--
-- DATE: February 12, 2019
--
-- REVISIONS:
--
-- DESIGNER: Keishi Asai
--
-- PROGRAMMER: Keishi Asai
--
-- INTERFACE: DWORD WINAPI runUDPServer(LPVOID tUdpParams)
--							LPVOID tUdpParams : Parameters for transfer configuration
--
-- RETURNS: DWORD WINAPI
--
-- NOTES:
-- A thread fuction to process UDP server tasks asynchronously
----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI runUDPServer(LPVOID tUdpParams) 
{
	SOCKET Listen;
	SOCKADDR_IN InternetAddr;
	DWORD Event;
	WSANETWORKEVENTS NetworkEvents;
	WSADATA wsaData;
	DWORD Ret;
	DWORD Flags;
	DWORD RecvBytes;

	FILE *fp;
	char filename[] = "udptest.txt";

	SOCKADDR_IN sin;
	int sin_len;
	sin_len = sizeof(sin);

	svparams *svp = (svparams *)tUdpParams;

	int recvd_packet = 0;
	int processed_packet = 0;
	int lost_packet = 0;

	SYSTEMTIME tr_start_time;
	SYSTEMTIME tr_end_time;
	int first_read_flag = 1;

	int total_bytes_recvd = 0;
	int packet_bytes_recvd = 0;

	if ((Ret = WSAStartup(0x0202, &wsaData)) != 0)
	{
		return TRUE;
	}

	// Prepare for listening port
	if ((Listen = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		return TRUE;
	}

	CreateSocketInformation(Listen);

	if (WSAEventSelect(Listen, EventArray[EventTotal - 1], FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR)
	{
		return TRUE;
	}

	InternetAddr.sin_family = AF_INET;
	InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	InternetAddr.sin_port = htons(svp->port);

	if (bind(Listen, (PSOCKADDR)&InternetAddr, sizeof(InternetAddr)) == SOCKET_ERROR)
	{
		return TRUE;
	}

	sprintf_s(disp_line_buf, DISP_LINE_BUFSIZE, "Start listening...");
	PostMessage(
		(HWND)hwnd,
		WM_DISPLAY_TEXT,
		(WPARAM)disp_line_buf,
		NULL
	);

	// Event controls
	while (TRUE)
	{
		// Wait for one of the sockets to receive I/O notification and 
		if ((Event = WSAWaitForMultipleEvents(EventTotal, EventArray, FALSE,
			10000, FALSE)) == WSA_WAIT_FAILED)
		{
			return TRUE;
		}
		if (Event == WSA_WAIT_TIMEOUT) {
			// Consider timeout as the end of data transfer
			// Display statistics on the window
			sprintf_s(quit_msg, DISP_LINE_BUFSIZE, "**** Finished the data transfer ****");
			PostMessage(
				(HWND)hwnd,
				WM_DISPLAY_TEXT,
				(WPARAM)quit_msg,
				NULL
			);
			sprintf_s(stat_line1, DISP_LINE_BUFSIZE, "Total data transferred: %d Bytes  Total Packet lost: %d", total_bytes_recvd, svp->exp_packet_num - recvd_packet);
			PostMessage(
				(HWND)hwnd,
				WM_DISPLAY_TEXT,
				(WPARAM)stat_line1,
				NULL
			);
			sprintf_s(stat_line2, DISP_LINE_BUFSIZE, "Transfer time: %ld ms", delay(tr_start_time, tr_end_time));
			PostMessage(
				(HWND)hwnd,
				WM_DISPLAY_TEXT,
				(WPARAM)stat_line2,
				NULL
			);

			WSACleanup();
			ExitThread(0);
		}

		if (WSAEnumNetworkEvents(SocketArray[Event - WSA_WAIT_EVENT_0]->Socket, EventArray[Event -
			WSA_WAIT_EVENT_0], &NetworkEvents) == SOCKET_ERROR)
		{
			return TRUE;
		}

		if (NetworkEvents.lNetworkEvents & FD_READ ||
			NetworkEvents.lNetworkEvents & FD_WRITE)
		{
			if (NetworkEvents.lNetworkEvents & FD_READ &&
				NetworkEvents.iErrorCode[FD_READ_BIT] != 0)
			{
				break;
			}

			if (NetworkEvents.lNetworkEvents & FD_WRITE &&
				NetworkEvents.iErrorCode[FD_WRITE_BIT] != 0)
			{
				break;
			}

			LPSOCKET_INFORMATION SocketInfo = SocketArray[Event - WSA_WAIT_EVENT_0];

			// Record start time for statistics
			if (first_read_flag) {
				GetSystemTime(&tr_start_time);
				first_read_flag = 0;
			}

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
						FreeSocketInformation(Event - WSA_WAIT_EVENT_0);
						return TRUE;
					}
				}
				else
				{
					SocketInfo->BytesRECV = RecvBytes;
				}
			}

			// Accept packets only if the received packet is the same size of expected packet size
			// if not, consider the packet was lost
			if (SocketInfo->BytesRECV == svp->packet_size)
			{
				GetSystemTime(&tr_end_time);
				packet_bytes_recvd += SocketInfo->BytesRECV;
				total_bytes_recvd += SocketInfo->BytesRECV;
				recvd_packet++;

				fopen_s(&fp, filename, "a+");
				fputs(SocketInfo->DataBuf.buf, fp);
				fclose(fp);

				sprintf_s(disp_line_buf, DISP_LINE_BUFSIZE, "Received: %d Bytes packet.  Total number of packets received: %d", packet_bytes_recvd, recvd_packet);
				PostMessage(
					(HWND)hwnd,
					WM_DISPLAY_TEXT,
					(WPARAM)disp_line_buf,
					NULL
				);

				packet_bytes_recvd = 0;
				SocketInfo->BytesRECV = 0;
			}
			else {
				lost_packet++;
				packet_bytes_recvd = 0;
				SocketInfo->BytesRECV = 0;
			}
		}

		if (NetworkEvents.lNetworkEvents & FD_CLOSE)
		{
			if (NetworkEvents.iErrorCode[FD_CLOSE_BIT] != 0)
			{
				break;
			}

			FreeSocketInformation(Event - WSA_WAIT_EVENT_0);
		}
	}
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: runTCPServer
--
-- DATE: February 12, 2019
--
-- REVISIONS:
--
-- DESIGNER: Keishi Asai
--
-- PROGRAMMER: Keishi Asai
--
-- INTERFACE: DWORD WINAPI runUDPServer(LPVOID tTcpParams)
--							LPVOID tTcpParams : Parameters for transfer configuration
--
-- RETURNS: DWORD WINAPI
--
-- NOTES:
-- A thread fuction to process TCP server tasks asynchronously
----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI runTCPServer(LPVOID tTcpParams) 
{
	SOCKET Listen;
	SOCKET Accept;
	SOCKADDR_IN InternetAddr;
	DWORD Event;
	WSANETWORKEVENTS NetworkEvents;
	WSADATA wsaData;
	WSABUF quit_buf;
	DWORD Ret;
	DWORD Flags;
	DWORD RecvBytes;
	DWORD SendBytes;
	int recvd_packet = 0;
	int processed_packet = 0;
	int lost_packet = 0;
	SYSTEMTIME tr_start_time;
	SYSTEMTIME tr_end_time;
	int total_bytes_recvd = 0;
	int packet_bytes_recvd = 0;

	FILE *fp;
	char filename[] = "tcptest.txt";

	svparams *svp = (svparams *)tTcpParams;
	char *packet_buf = (char *)malloc(svp->packet_size);
	memset(packet_buf, 0, sizeof(packet_buf));

	if ((Ret = WSAStartup(0x0202, &wsaData)) != 0)
	{
		return TRUE;
	}

	// Prepare for listening port
	if ((Listen = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		return TRUE;
	}

	CreateSocketInformation(Listen);

	if (WSAEventSelect(Listen, EventArray[EventTotal - 1], FD_ACCEPT | FD_CLOSE) == SOCKET_ERROR)
	{
		return TRUE;
	}

	InternetAddr.sin_family = AF_INET;
	InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	InternetAddr.sin_port = htons(svp->port);

	if (bind(Listen, (PSOCKADDR)&InternetAddr, sizeof(InternetAddr)) == SOCKET_ERROR)
	{
		return TRUE;
	}

	if (listen(Listen, 5))
	{
		return TRUE;
	}
	sprintf_s(disp_line_buf, DISP_LINE_BUFSIZE, "Start listening...");
	PostMessage(
		(HWND)hwnd,
		WM_DISPLAY_TEXT,
		(WPARAM)disp_line_buf,
		NULL
	);

	// Event controls
	while (TRUE)
	{
		// Wait for one of the sockets to receive I/O notification and 
		if ((Event = WSAWaitForMultipleEvents(EventTotal, EventArray, FALSE,
			10000, FALSE)) == WSA_WAIT_FAILED)
		{
			return TRUE;
		}
		if (Event == WSA_WAIT_TIMEOUT) {
			// Consider timeout as the end of the file transfer
			// Display statistics
			sprintf_s(quit_msg, DISP_LINE_BUFSIZE, "**** Finished the data transfer ****");
			PostMessage(
				(HWND)hwnd,
				WM_DISPLAY_TEXT,
				(WPARAM)quit_msg,
				NULL
			);
			sprintf_s(stat_line1, DISP_LINE_BUFSIZE, "Total data transferred: %d Bytes  Total Packet lost: %d", total_bytes_recvd, lost_packet);
			PostMessage(
				(HWND)hwnd,
				WM_DISPLAY_TEXT,
				(WPARAM)stat_line1,
				NULL
			);
			sprintf_s(stat_line2, DISP_LINE_BUFSIZE, "Transfer time: %ld ms", delay(tr_start_time, tr_end_time));
			PostMessage(
				(HWND)hwnd,
				WM_DISPLAY_TEXT,
				(WPARAM)stat_line2,
				NULL
			);
			sprintf_s(quit_msg, DISP_LINE_BUFSIZE, "Server: finish the connection");
			quit_buf.buf = quit_msg;
			quit_buf.len = strlen(quit_msg);

			if (WSASend(Accept, &(quit_buf), 1, &SendBytes, 0,
				NULL, NULL) == SOCKET_ERROR)
			{
				if (WSAGetLastError() != WSAEWOULDBLOCK)
				{
					FreeSocketInformation(Event - WSA_WAIT_EVENT_0);
					return TRUE;
				}
			}

			closesocket(Accept);
			free(svp);
			free(packet_buf);

			WSACleanup();
			ExitThread(0);
		}

		if (WSAEnumNetworkEvents(SocketArray[Event - WSA_WAIT_EVENT_0]->Socket, EventArray[Event -
			WSA_WAIT_EVENT_0], &NetworkEvents) == SOCKET_ERROR)
		{
			return TRUE;
		}

		if (NetworkEvents.lNetworkEvents & FD_ACCEPT)
		{
			if (NetworkEvents.iErrorCode[FD_ACCEPT_BIT] != 0)
			{
				break;
			}

			if ((Accept = accept(SocketArray[Event - WSA_WAIT_EVENT_0]->Socket, NULL, NULL)) == INVALID_SOCKET)
			{
				break;
			}

			if (EventTotal > WSA_MAXIMUM_WAIT_EVENTS)
			{
				closesocket(Accept);
				break;
			}

			CreateSocketInformation(Accept);

			if (WSAEventSelect(Accept, EventArray[EventTotal - 1], FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR)
			{
				return TRUE;
			}

			sprintf_s(disp_line_buf, DISP_LINE_BUFSIZE, "Connected.");
			PostMessage(
				(HWND)hwnd,
				WM_DISPLAY_TEXT,
				(WPARAM)disp_line_buf,
				NULL
			);

			GetSystemTime(&tr_start_time);
		}

		if (NetworkEvents.lNetworkEvents & FD_READ ||
			NetworkEvents.lNetworkEvents & FD_WRITE)
		{
			if (NetworkEvents.lNetworkEvents & FD_READ &&
				NetworkEvents.iErrorCode[FD_READ_BIT] != 0)
			{
				break;
			}

			if (NetworkEvents.lNetworkEvents & FD_WRITE &&
				NetworkEvents.iErrorCode[FD_WRITE_BIT] != 0)
			{
				break;
			}

			LPSOCKET_INFORMATION SocketInfo = SocketArray[Event - WSA_WAIT_EVENT_0];

			int tout_counter = 0;
			char estr[256];
			int wrote_index = 0;
			int remaining = svp->packet_size;

			// Read data only if the receive buffer is empty.
			if (SocketInfo->BytesRECV == 0)
			{
				SocketInfo->DataBuf.buf = SocketInfo->Buffer;
				SocketInfo->DataBuf.len = svp->packet_size;
				
				Flags = 0;
				while (remaining != 0 && tout_counter <= 5)
				{
					if (WSARecv(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes,
						&Flags, NULL, NULL) == SOCKET_ERROR)
					{
						if (WSAGetLastError() != WSAEWOULDBLOCK)
						{
							FreeSocketInformation(Event - WSA_WAIT_EVENT_0);
							return TRUE;
						}
					}
					else
					{
						// Keep tracking how many bytes are received
						// When received the same bytes as packet size,
						// Consider the chunk of bytes as one packet
						if (RecvBytes == 0) {
							tout_counter++;
						}
						else {
							SocketInfo->BytesRECV = RecvBytes;
							remaining = remaining - SocketInfo->BytesRECV;
							SocketInfo->DataBuf.len = remaining;

							// Calculations to determine next bytes to read to complete a packet
							for (unsigned i=0; i<SocketInfo->BytesRECV; i++)
							{
								packet_buf[i+wrote_index] = SocketInfo->DataBuf.buf[i];
							}
							wrote_index += SocketInfo->BytesRECV;
							packet_bytes_recvd += SocketInfo->BytesRECV;
							total_bytes_recvd += SocketInfo->BytesRECV;

							tout_counter = 0;
						}
					}
				}
				recvd_packet++;
			}

			// Write packet to file 
			if (recvd_packet > processed_packet)
			{
				GetSystemTime(&tr_end_time);
				SocketInfo->DataBuf.buf = SocketInfo->Buffer;
				remaining = svp->packet_size;

				fopen_s(&fp, filename, "a+");
				fputs(packet_buf, fp);
				fclose(fp);

				sprintf_s(disp_line_buf, DISP_LINE_BUFSIZE, "Received: %d Bytes packet.  Total number of packets received: %d", packet_bytes_recvd, recvd_packet);
				PostMessage(
					(HWND)hwnd,
					WM_DISPLAY_TEXT,
					(WPARAM)disp_line_buf,
					NULL
				);

				packet_bytes_recvd = 0;
				memset(packet_buf, 0, sizeof(packet_buf));
				wrote_index = 0;

				SocketInfo->BytesRECV = 0;
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: handleServerDialog
--
-- DATE: February 12, 2019
--
-- REVISIONS:
--
-- DESIGNER: Keishi Asai
--
-- PROGRAMMER: Keishi Asai
--
-- INTERFACE: INT_PTR CALLBACK handleServerDialog(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
--							HWND hwndDlg : Dialog handler
--							UINT message : Message sent to the dialog
--							WPARAM wParam : parameter sent with the message
--							LPARAM lParam : parameter sent with the message
--
-- RETURNS: INT_PTR 
--
-- NOTES:
-- Process Server configuration dialog window. Create server threads according to the given parameters.
----------------------------------------------------------------------------------------------------------------------*/
INT_PTR CALLBACK handleServerDialog(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	const int INPUT_BUF_SIZE=256;
	HANDLE hTcpRunner;
	HANDLE hUdpRunner;
	DWORD dwTcpThreadID;
	DWORD dwUdpThreadID;

	WORD wVersionRequested = MAKEWORD(2, 2);
	WSADATA wsaData;

	int packet_size;
	int exp_packet_num;
	int port;
	const int BUF_SIZE = 256;
	char DEF_HOST[] = "localhost";
	const int DEF_PORT = 5150;
	const int DEF_PSIZE = 1024;
	const int DEF_REPEAT_NUM = 1;
	char port_buf[INPUT_BUF_SIZE];
	char psize_buf[INPUT_BUF_SIZE];
	char times_buf[INPUT_BUF_SIZE];

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
	if (GetDlgItemText(hwndDlg, IDC_EDIT6_2, port_buf, INPUT_BUF_SIZE) == 0) 
	{
		port = DEF_PORT;
	}
	else {
		port = atoi(port_buf);
	}

	switch (message)
	{
		case WM_INITDIALOG:
			return TRUE;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_BUTTON6_1:
					if (IsDlgButtonChecked(hwndDlg, IDC_RADIO6_2))
					{
						// Run UDP Server
						svparams *svp = (svparams*)malloc(sizeof(svp));
						svp->port = port;
						svp->packet_size = packet_size;
						svp->exp_packet_num = exp_packet_num;

						hUdpRunner = CreateThread(NULL, 0, runUDPServer, svp, 0, &dwUdpThreadID);
						return TRUE;
					}
					else 
					{
						// Run TCP Server
						svparams *svp = (svparams*)malloc(sizeof(svp));
						svp->port = port;
						svp->packet_size = packet_size;
						svp->exp_packet_num = exp_packet_num;

						hTcpRunner = CreateThread(NULL, 0, runTCPServer, svp, 0, &dwTcpThreadID);
						return TRUE;
					}
				case IDCANCEL:
					EndDialog(hwndDlg, (INT_PTR)0);
					return TRUE;
			}
	}
	return FALSE;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: handleClientDialog
--
-- DATE: February 12, 2019
--
-- REVISIONS:
--
-- DESIGNER: Keishi Asai
--
-- PROGRAMMER: Keishi Asai
--
-- INTERFACE: INT_PTR CALLBACK handleClientDialog(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
--							HWND hwndDlg : Dialog handler
--							UINT message : Message sent to the dialog
--							WPARAM wParam : parameter sent with the message
--							LPARAM lParam : parameter sent with the message
--
-- RETURNS: INT_PTR 
--
-- NOTES:
-- Process Client configuration dialog window. Perform client tasks according to the given paramters.
----------------------------------------------------------------------------------------------------------------------*/
INT_PTR CALLBACK handleClientDialog(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	const int INPUT_BUF_SIZE=256;
	const int BUF_SIZE = 256;
	char DEF_HOST[] = "localhost";
	const int DEF_PORT = 5150;
	const int DEF_PSIZE = 1024;
	const int DEF_REPEAT_NUM = 1;
	char hname_buf[INPUT_BUF_SIZE];
	char port_buf[INPUT_BUF_SIZE];
	char psize_buf[INPUT_BUF_SIZE];
	char times_buf[INPUT_BUF_SIZE];

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
	int packet_sent = 0;

	struct	sockaddr_in client;
	int client_len;
	int server_len;
	int MAXLEN = 65535;
	SYSTEMTIME stStartTime;
	SYSTEMTIME stEndTime;

	int fread_bytes = 0;

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
					// Launch file selecetion dialog window and get a file path to send
					selected_filename = openSelectFileDialog();
					selected_filename = replace_char(selected_filename, '\\', '/');
					
					fopen_s(&selected_fp, selected_filename, "r");
					fseek(selected_fp, 0L, SEEK_END);
					selected_filesize = ftell(selected_fp);
					rewind(selected_fp);

					fclose(selected_fp);
					break;
				case IDC_BUTTON5_2:
					// Check configurations
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
					
					sbuf = (char*)malloc(packet_size);
					rbuf = (char*)malloc(packet_size);
					
					if (IsDlgButtonChecked(hwndDlg, IDC_RADIO5_2)) {
						// UDP client tasks
						if ((sd = socket(PF_INET, SOCK_DGRAM, 0)) == -1)
						{
							perror("Can't create a socket\n");
							exit(1);
						}

						sprintf_s(disp_line_buf, DISP_LINE_BUFSIZE, "Opened socket for UDP.");
						PostMessage(
							(HWND)hwnd,
							WM_DISPLAY_TEXT,
							(WPARAM)disp_line_buf,
							NULL
						);

						// Store server's information
						memset((char *)&server, 0, sizeof(server));
						server.sin_family = AF_INET;
						server.sin_port = htons(port);

						if ((hp = gethostbyname(host)) == NULL)
						{
							fprintf(stderr, "Can't get server's IP address\n");
							exit(1);
						}
						memcpy((char *)&server.sin_addr, hp->h_addr, hp->h_length);

						if (packet_size > MAXLEN)
						{
							fprintf(stderr, "Data is too big\n");
							exit(1);
						}

						// transmit data
						server_len = sizeof(server);

						if (selected_filename == NULL) {
							// If file to send is not selected, generate padding data

							// data	is a, b, c, ..., z, a, b,...
							for (i = 0; i < packet_size; i++)
							{
								j = (i < 26) ? i : i % 26;
								sbuf[i] = 'a' + j;
							}
							sprintf_s(disp_line_buf, DISP_LINE_BUFSIZE, "Start sending packets.");
							PostMessage(
								(HWND)hwnd,
								WM_DISPLAY_TEXT,
								(WPARAM)disp_line_buf,
								NULL
							);

							for (i = 0; i < repeat_num; i++) {
								// Transmit data through the socket
								if (sendto(sd, sbuf, packet_size, 0, (struct sockaddr *)&server, server_len) == -1)
								{
									perror("sendto failure");
									exit(1);
								}
								packet_sent++;
							}
						}
						else {
							// If file to send is chosen, send it

							// Calculate number of packets
							num_packets = selected_filesize / (packet_size-1);
							if (selected_filesize % packet_size-1 != 0) {
								num_packets++;
							}

							fopen_s(&selected_fp, selected_filename, "r");
							for (i = 0; i < num_packets; i++)
							{
								// Packetize file bytes
								fread_bytes = fread_s(sbuf, packet_size, 1, packet_size-1, selected_fp);
								if (fread_bytes < packet_size-1) {
									memset(sbuf + fread_bytes, 0, packet_size - fread_bytes);
								}
								sbuf[packet_size - 1] = '\0';

								sprintf_s(disp_line_buf, DISP_LINE_BUFSIZE, "Start sending packets.");
								PostMessage(
									(HWND)hwnd,
									WM_DISPLAY_TEXT,
									(WPARAM)disp_line_buf,
									NULL
								);

								// Send a packet
								if (sendto(sd, sbuf, packet_size, 0, (struct sockaddr *)&server, server_len) == -1)
								{
									perror("sendto failure");
									exit(1);
								}
								packet_sent++;
							}
							fclose(selected_fp);
						}
						sprintf_s(disp_line_buf, DISP_LINE_BUFSIZE, "Finished sending packets. Sent: %d packets", packet_sent);
						PostMessage(
							(HWND)hwnd,
							WM_DISPLAY_TEXT,
							(WPARAM)disp_line_buf,
							NULL
						);

						free(sbuf);
						free(rbuf);
						closesocket(sd);
					}
					else {
						// TCP client tasks
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
							exit(1);
						}
						sprintf_s(disp_line_buf, DISP_LINE_BUFSIZE, "Connected.");
						PostMessage(
							(HWND)hwnd,
							WM_DISPLAY_TEXT,
							(WPARAM)disp_line_buf,
							NULL
						);

						pptr = hp->h_addr_list;
						memset((char *)sbuf, 0, sizeof(sbuf));

						if (selected_filename == NULL) {
							// If file to send is not chosen, create padding data
							for (i = 0; i < packet_size-1; i++)
							{
								j = (i < 26) ? i : i % 26;
								sbuf[i] = 'a' + j;
							}
							sbuf[i] = '\0';

							sprintf_s(disp_line_buf, DISP_LINE_BUFSIZE, "Start sending packets.");
							PostMessage(
								(HWND)hwnd,
								WM_DISPLAY_TEXT,
								(WPARAM)disp_line_buf,
								NULL
							);

							for (i = 0; i < repeat_num; i++) 
							{
								// Transmit data through the socket
								ns = send(sd, sbuf, packet_size, 0);
							}
							packet_sent++;
						}
						else 
						{
							// If file to send is chosen, send it

							// Calculate number of packets
							num_packets = selected_filesize / (packet_size-1);
							if (selected_filesize % packet_size-1 != 0) {
								num_packets++;
							}

							sprintf_s(disp_line_buf, DISP_LINE_BUFSIZE, "Start sending packets.");
							PostMessage(
								(HWND)hwnd,
								WM_DISPLAY_TEXT,
								(WPARAM)disp_line_buf,
								NULL
							);

							fopen_s(&selected_fp, selected_filename, "r");
							for (i = 0; i < num_packets; i++)
							{
								// Packetize bytes
								fread_bytes = fread_s(sbuf, packet_size, 1, packet_size-1, selected_fp);
								if (fread_bytes < packet_size-1) {
									memset(sbuf + fread_bytes, 0, packet_size - fread_bytes);
								}
								sbuf[packet_size - 1] = '\0';

								ns = send(sd, sbuf, packet_size, 0);
								packet_sent++;
							}
							fclose(selected_fp);
						}
						sprintf_s(disp_line_buf, DISP_LINE_BUFSIZE, "Finished sending packets. Sent: %d packets", packet_sent);
						PostMessage(
							(HWND)hwnd,
							WM_DISPLAY_TEXT,
							(WPARAM)disp_line_buf,
							NULL
						);

						bp = rbuf;
						bytes_to_read = packet_size;

						free(sbuf);
						free(rbuf);

						// client makes repeated calls to recv until no more data is expected to arrive.
						while (bytes_to_read != 0 && (n = recv(sd, bp, bytes_to_read, 0)) < packet_size)
						{
							bp += n;
							bytes_to_read -= n;
							if (n == 0) {
								break;
							}
						}
						closesocket(sd);
						OutputDebugStringA("yep");
					}
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
-- DATE: February 12, 2019
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
-- DATE: February 12, 2019
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
	char line[1024];
	static unsigned k = 0;

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
				default:
					break;
			}
			break;
		case WM_DISPLAY_TEXT:
			sprintf_s(line, "%s", (char *)wParam);

			window_printline(line, strlen(line));
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
