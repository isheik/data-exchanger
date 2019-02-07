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
#define STRICT

#pragma comment(lib, "Ws2_32.lib")
#include <windows.h>
#include <stdio.h>
#include "resource.h"
#include "resource1.h"
#include "resource2.h"
#include "resource3.h"
#include "resource4.h"
#include "custom_message.h"
#include "data_exchanger.h"

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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: htoip_convert
--
-- DATE: January 21, 2019
--
-- REVISIONS:
--
-- DESIGNER: Keishi Asai
--
-- PROGRAMMER: Keishi Asai
--
-- INTERFACE: BOOL CALLBACK htoip_convert(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
--							HWND hwndDlg: A handle to the dialog box
--							UINT message: The message processed by this dialog box procedure
--							WPARAM wParam: Additional message-specific information
--							LPARAM lParam: Additional message-specific information
-- RETURNS: BOOL Return TRUE if this dialog box procedure processed the message. If not, return FALSE.
--
-- NOTES:
-- This function is a dialog box procedure for resolving a host name into IP address.
-- It takes user's input and issue an asynchronous query.
----------------------------------------------------------------------------------------------------------------------*/
BOOL CALLBACK htoip_convert(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	const int INPUT_BUF_SIZE = 256;
	char hname_buf[INPUT_BUF_SIZE];
	HANDLE async_handle;

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
				case IDC_BUTTON1:
					GetDlgItemText(hwndDlg, IDC_EDIT1, hname_buf, INPUT_BUF_SIZE);
					async_handle = WSAAsyncGetHostByName(hwnd, WM_GET_IP_BY_NAME_DONE, hname_buf, result_buf, MAXGETHOSTSTRUCT);
					if (async_handle == 0)
					{
						MessageBox(hwnd, "Error in get host by name", "Error", MB_OK);
					}
					else 
					{
						EndDialog(hwndDlg, (INT_PTR)1);
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
-- FUNCTION: iptoh_convert
--
-- DATE: January 21, 2019
--
-- REVISIONS:
--
-- DESIGNER: Keishi Asai
--
-- PROGRAMMER: Keishi Asai
--
-- INTERFACE: BOOL CALLBACK htoip_convert(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
--							HWND hwndDlg: A handle to the dialog box
--							UINT message: The message processed by this dialog box procedure
--							WPARAM wParam: Additional message-specific information
--							LPARAM lParam: Additional message-specific information
-- RETURNS: BOOL Return TRUE if this dialog box procedure processed the message. If not, return FALSE.
--
-- NOTES:
-- This function is a dialog box procedure for resolving IP address into a host name.
-- It takes user's input and issue an asynchronous query.
----------------------------------------------------------------------------------------------------------------------*/
BOOL CALLBACK iptoh_convert(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	const int INPUT_BUF_SIZE = 256;

	struct in_addr my_addr, *addr_p;
	char ipaddr_buf[INPUT_BUF_SIZE];
	HANDLE async_handle;

	WORD wVersionRequested = MAKEWORD(2, 2);
	WSADATA wsaData;

	WSAStartup(wVersionRequested, &wsaData);

	addr_p = &my_addr;
	
	switch (message)
	{
		case WM_INITDIALOG:
			return TRUE;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_BUTTON2_1:
					GetDlgItemText(hwndDlg, IDC_EDIT2_2, ipaddr_buf, INPUT_BUF_SIZE);
					addr_p->s_addr = inet_addr(ipaddr_buf);

					async_handle = WSAAsyncGetHostByAddr(hwnd, WM_GET_HOST_BY_IP_DONE, (char*)addr_p, 4, PF_INET, result_buf, MAXGETHOSTSTRUCT);
					if (async_handle == 0)
					{
						MessageBox(hwnd, "Error in get host by addr", "Error", MB_OK);
					}
					else {
						EndDialog(hwndDlg, (INT_PTR)1);
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
-- FUNCTION: ptosv_convert
--
-- DATE: January 21, 2019
--
-- REVISIONS:
--
-- DESIGNER: Keishi Asai
--
-- PROGRAMMER: Keishi Asai
--
-- INTERFACE: BOOL CALLBACK htoip_convert(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
--							HWND hwndDlg: A handle to the dialog box
--							UINT message: The message processed by this dialog box procedure
--							WPARAM wParam: Additional message-specific information
--							LPARAM lParam: Additional message-specific information
-- RETURNS: BOOL Return TRUE if this dialog box procedure processed the message. If not, return FALSE.
--
-- NOTES:
-- This function is a dialog box procedure for resolving a port number/protocol into a service name.
-- It takes user's input and issue an asynchronous query.
----------------------------------------------------------------------------------------------------------------------*/
BOOL CALLBACK ptosv_convert(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	const int INPUT_BUF_SIZE = 128;

	int s_port;
	char port_number[INPUT_BUF_SIZE];
	char protocol_name[INPUT_BUF_SIZE];
	HANDLE async_handle;

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
				case IDC_BUTTON3_1:
					GetDlgItemText(hwndDlg, IDC_EDIT3_1, port_number, INPUT_BUF_SIZE);
					GetDlgItemText(hwndDlg, IDC_EDIT3_2, protocol_name, INPUT_BUF_SIZE);
					s_port = atoi(port_number);

					async_handle = WSAAsyncGetServByPort(hwnd, WM_GET_SERVICE_BY_PORT, htons(s_port), protocol_name, result_buf, MAXGETHOSTSTRUCT);
					if (async_handle == 0)
					{
						MessageBox(hwnd, "Error in get serv by port", "Error", MB_OK);
					}
					else
					{
						EndDialog(hwndDlg, (INT_PTR)1);
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
-- FUNCTION: svtop_convert
--
-- DATE: January 21, 2019
--
-- REVISIONS:
--
-- DESIGNER: Keishi Asai
--
-- PROGRAMMER: Keishi Asai
--
-- INTERFACE: BOOL CALLBACK htoip_convert(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
--							HWND hwndDlg: A handle to the dialog box
--							UINT message: The message processed by this dialog box procedure
--							WPARAM wParam: Additional message-specific information
--							LPARAM lParam: Additional message-specific information
-- RETURNS: BOOL Return TRUE if this dialog box procedure processed the message. If not, return FALSE.
--
-- NOTES:
-- This function is a dialog box procedure for resolving a service name/protocol into a port number.
-- It takes user's input and issue an asynchronous query.
----------------------------------------------------------------------------------------------------------------------*/
BOOL CALLBACK svtop_convert(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	const int INPUT_BUF_SIZE = 128;

	char service_name[INPUT_BUF_SIZE];
	char protocol_name[INPUT_BUF_SIZE];
	HANDLE async_handle;

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
				case IDC_BUTTON4_1:
					GetDlgItemText(hwndDlg, IDC_EDIT4_1, service_name, INPUT_BUF_SIZE);
					GetDlgItemText(hwndDlg, IDC_EDIT4_2, protocol_name, INPUT_BUF_SIZE);

					async_handle = WSAAsyncGetServByName(hwnd, WM_GET_PORT_BY_SERVICE, service_name, protocol_name, result_buf, MAXGETHOSTSTRUCT);
					if (async_handle == 0)
					{
						MessageBox(hwnd, "Error in get serv by name", "Error", MB_OK);
					}
					else
					{
						EndDialog(hwndDlg, (INT_PTR)1);
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
				case HTOIP:
					DialogBox(Wcl.hInstance, MAKEINTRESOURCE(IDD_FORMVIEW), hwnd, htoip_convert);
					break;
				case IPTOH:
					DialogBox(Wcl.hInstance, MAKEINTRESOURCE(IDD_FORMVIEW2), hwnd, iptoh_convert);
					break;
				case SVTOP:
					DialogBox(Wcl.hInstance, MAKEINTRESOURCE(IDD_FORMVIEW4), hwnd, svtop_convert);
					break;
				case PTOSV:
					DialogBox(Wcl.hInstance, MAKEINTRESOURCE(IDD_FORMVIEW3), hwnd, ptosv_convert);
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
