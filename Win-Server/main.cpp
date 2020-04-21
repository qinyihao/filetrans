#include <windows.h>
#include "main.h"
#include <io.h>
#include <bits/stdc++.h>
#include <direct.h>

using namespace std;

ofstream fout;
time_t timenow;
string choosefilestr = "NOT_CHOSEN";

bool checkMyselfExist() {//如果程序已经有一个在运行，则返回true
    HANDLE  hMutex = CreateMutex(NULL, FALSE, "DevState");
    if (hMutex && (GetLastError() == ERROR_ALREADY_EXISTS)) {
       	CloseHandle(hMutex);
       	hMutex = NULL;
       	return true;
    } else {
      	return false;
    }
}

BOOL DoFileOpen(HWND hwnd, char rt[]) {
	OPENFILENAME ofn;

	ZeroMemory(&ofn, sizeof(ofn));
	rt[0] = 0;
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = "All Files(*.*)\0*.*\0\0";
	ofn.lpstrFile = rt;
	ofn.nMaxFile = MAX_PATH*4;
	ofn.lpstrDefExt = "";	
	ofn.Flags = OFN_EXPLORER|OFN_FILEMUSTEXIST|OFN_HIDEREADONLY;
	if(GetOpenFileName(&ofn)) {
		;
	} else {
		return FALSE;
	}
	return TRUE;
}

/* This is where all the input to the window goes to */
LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	string dir = "C:\\.filetrans";
	string checkstr = "";
	switch(Message) {
		case WM_CREATE: {
			CreateWindow("STATIC", "", WS_CHILD|WS_VISIBLE|WS_BORDER, 5, 5, 405, 20,hwnd, (HMENU)IDC_PATH, NULL, NULL);
			CreateWindow("BUTTON", "选取文件", WS_CHILD|WS_VISIBLE,415, 5, 100, 20,hwnd, (HMENU)IDC_CHOOSEPATH, NULL, NULL);
			CreateWindow("STATIC", "    IP=", WS_CHILD|WS_VISIBLE, 5, 30, 60, 20,hwnd, (HMENU)IDC_STATIC1, NULL, NULL);
			CreateWindow("EDIT", "", WS_CHILD|WS_VISIBLE|WS_BORDER, 70, 30, 250, 20,hwnd, (HMENU)IDC_IP, NULL, NULL);
			CreateWindow("STATIC", "    Port=", WS_CHILD|WS_VISIBLE, 330, 30, 80, 20,hwnd, (HMENU)IDC_STATIC2, NULL, NULL);
			CreateWindow("EDIT", "", WS_CHILD|WS_VISIBLE|WS_BORDER, 415, 30, 100, 20,hwnd, (HMENU)IDC_PORT, NULL, NULL);
			CreateWindow("BUTTON", "一键传输文件", WS_CHILD|WS_VISIBLE,5, 55, 510, 20,hwnd, (HMENU)IDC_TRANS, NULL, NULL);
			return 0;
		}
		case WM_COMMAND: {
			switch (LOWORD(wParam)) {
				case IDC_CHOOSEPATH: {
					char filepath[MAX_PATH*10];
					DoFileOpen(hwnd, filepath);
					SetDlgItemText(hwnd, IDC_PATH, filepath);
					choosefilestr.clear();
					choosefilestr += filepath;
					break;
				}
				case IDC_TRANS: {
					if (MessageBox(hwnd, "Are you sure to transport the file you selected at once?", "Transport", MB_YESNO) != IDYES) {
						MessageBox(hwnd, "Transporting terminated.", "Transport", MB_OK | MB_ICONINFORMATION);
						break;
					}
					if (_access(dir.c_str(), F_OK) == -1) {
						int flag = _mkdir(dir.c_str());
						if (flag != 0) {
							MessageBox(hwnd, "Error: Fail to mkdir: C:\\.filetrans\nTransporting terminated.", "", MB_OK | MB_ICONHAND);
							break;
						}
					}
					if (_access(dir.c_str(), W_OK) == -1) {
						int flag = _mkdir(dir.c_str());
						if (flag != 0) {
							MessageBox(hwnd, "Error: No accessibility to folder: C:\\.filetrans\nTransporting terminated.", "", MB_OK | MB_ICONHAND);
							break;
						}
					}
					SetFileAttributes("C:\\.filetrans", FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
					fout.open("C:\\.filetrans\\win-trans.ini");
					char tmpinfo[100];
					fout << "; This is a temporary transporting information file generated by Filetrans." << endl;
					fout << "; This will be overwritted. DO NOT EDIT!!!" << endl;
					fout << endl;
					
					if (choosefilestr == "NOT_CHOSEN" || choosefilestr == "") {
						//GetDlgItemText(hwnd, IDC_PATH, tmpinfo, MAX_PATH*10);
						MessageBox(hwnd, "You haven't chosen the path.\nTrasporting terminated.", "Error", MB_ICONHAND);
						fout << "[error]" << endl;
						fout.close();
						break;
					} else {
						sprintf(tmpinfo, "%s", choosefilestr.c_str());
					}
					fout << "[File]" << endl;
					fout << "filepath=" << tmpinfo << endl;
					
					fout << "[IPConfig]" << endl;
					GetDlgItemText(hwnd, IDC_IP, tmpinfo, 18);
					fout << "ipaddress=" << tmpinfo << endl;
					GetDlgItemText(hwnd, IDC_PORT, tmpinfo, 10);
					fout << "port=" << tmpinfo << endl;
					
					timenow = time(0);
					fout << "[Time]" << endl;
					fout << "date=" << asctime(localtime(&timenow));
					fout << flush;
					
					fout.close();
					MessageBox(hwnd, "Transportation finished. Please wait for the respond.", "Success", MB_OK | MB_ICONINFORMATION);
					break; 
				}
			}
			break;
		}
		/* Upon destruction, tell the main thread to stop */
		case WM_DESTROY: {
			PostQuitMessage(0);
			break;
		}
		
		/* All other messages (a lot of them) are processed using default procedures */
		default:
			return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	return 0;
}

/* The 'main' function of Win32 GUI programs: this is where execution starts */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	if (checkMyselfExist()) {
		return 0;
	}
	
	WNDCLASSEX wc; /* A properties struct of our window */
	HWND hwnd; /* A 'HANDLE', hence the H, or a pointer to our window */
	MSG msg; /* A temporary location for all messages */

	/* zero out the struct and set the stuff we want to modify */
	memset(&wc,0,sizeof(wc));
	wc.cbSize		 = sizeof(WNDCLASSEX);
	wc.lpfnWndProc	 = WndProc; /* This is where we will send messages to */
	wc.hInstance	 = hInstance;
	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
	
	/* White, COLOR_WINDOW is just a #define for a system color, try Ctrl+Clicking it */
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszClassName = "WindowClass";
	wc.hIcon		 = LoadIcon(NULL, IDI_APPLICATION); /* Load a standard icon */
	wc.hIconSm		 = LoadIcon(NULL, IDI_APPLICATION); /* use the name "A" to use the project icon */

	if(!RegisterClassEx(&wc)) {
		MessageBox(NULL, "Window Registration Failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	hwnd = CreateWindowEx(WS_EX_CLIENTEDGE,"WindowClass","Filetrans Windows Server",WS_VISIBLE|WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, /* x */
		CW_USEDEFAULT, /* y */
		540, /* width */
		125, /* height */
		NULL,NULL,hInstance,NULL);

	if(hwnd == NULL) {
		MessageBox(NULL, "Window Creation Failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}
	
	
	
	/*
		This is the heart of our program where all input is processed and 
		sent to WndProc. Note that GetMessage blocks code flow until it receives something, so
		this loop will not produce unreasonably high CPU usage
	*/
	while(GetMessage(&msg, NULL, 0, 0) > 0) { /* If no error is received... */
		TranslateMessage(&msg); /* Translate key codes to chars if present */
		DispatchMessage(&msg); /* Send it to WndProc */
	}
	return msg.wParam;
}
