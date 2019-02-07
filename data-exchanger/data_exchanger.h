#define IDM_HELP		200
#define IDM_EXIT		201
#define HTOIP			202
#define IPTOH			203
#define SVTOP			204
#define PTOSV			205

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK htoip_convert(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK iptoh_convert(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK ptosv_convert(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK svtop_convert(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);

