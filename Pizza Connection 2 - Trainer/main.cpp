#pragma comment(lib, "windowscodecs.lib")

#define _WIN32_WINNT _WIN32_WINNT_WIN7

#include "resource.h"
#include <windows.h>
#include <wincodec.h>
#include <tlhelp32.h>
#include <vector>

using namespace std;

enum CheatType { Cash, Points, Time };

struct Cheat
{
	BYTE VirtualKeyCode;
	BOOL IsKeyPressed;
	CheatType Type;
	DWORD Offset;
	const vector<DWORD> Pointers;
};

struct ClockStates
{
	const vector<BYTE> Normal;
	const vector<BYTE> Halt;
};

vector<Cheat> Cheats = {
	{ VK_F9, FALSE, Cash, 0x0017C670, vector<DWORD> { 0x200, 0x108 } },
	{ VK_F10, FALSE, Points, 0x0017C670, vector<DWORD> { 0x224, 0x3C } },
	{ VK_F11, FALSE, Time, 0x000DD3DD }
};

ClockStates ClockStatesShellcode = {
	vector<BYTE> { 0x41, 0x8B, 0xC1, 0x89, 0x4E },
	vector<BYTE> { 0xE9, 0x93, 0x00, 0x00, 0x00 }
};

BOOL IsRunningWithAdministrativePrivileges();
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void CreateRegion(HWND hwnd);
void PaintRegion();
HBITMAP LoadImageFromResource(LPCTSTR lpName, LPCTSTR lpType);
IStream* CreateStreamOnResource(LPCTSTR lpName, LPCTSTR lpType);
HBITMAP CreateHBITMAP(IWICBitmapSource* ipBitmap);
IWICBitmapSource* LoadBitmapFromStream(IStream* ipImageStream);
void DistortReality(CheatType& CheatType, DWORD& Offset, const vector<DWORD>& Pointers);
DWORD GetProcessIDByName();
DWORD GetBaseAddress(DWORD PID);
DWORD DereferencePointer(HANDLE hProc, DWORD BaseAddress, const vector<DWORD>& Offsets);

HWND hwnd = NULL;
HDC hdcScreen = NULL, hdcMemory = NULL;
BITMAP bmInfo = { 0 };
HRGN hRgn = NULL;
BOOL isDragging = FALSE;
POINT CursorPosition = { 0, 0 };
const WCHAR* BirnaryName = L"fastfood2.exe";

// Pulled from the abyss, grown in the hellish cauldron
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// TH32CS_SNAPMODULE require administrative privileges
	if (!IsRunningWithAdministrativePrivileges())
	{
		MessageBox(NULL, L"Trainer require administrative privileges!", L"Error", MB_ICONERROR);

		return 1;
	}

	WNDCLASSEX wc = { 0 };
	const LPCWSTR ClassName = L"Trainer";
	const LPCWSTR WindowName = L"Pizza Connection 2 - Trainer";

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = ClassName;
	wc.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));

	if (!RegisterClassEx(&wc))
	{
		// MessageBox(NULL, L"Window class registration failed!", L"Error", MB_ICONERROR);

		return 1;
	}

	hwnd = CreateWindowEx(0, ClassName, WindowName, WS_OVERLAPPED, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

	if (!hwnd)
	{
		// MessageBox(NULL, L"Window creation failed!", L"Error", MB_ICONERROR);

		return 1;
	}

	HWND Dialoghwnd = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DIALOG), hwnd, (DLGPROC)DialogProc);

	if (!Dialoghwnd)
	{
		// MessageBox(NULL, L"Dialog creation failed!", L"Error", MB_ICONERROR);

		return 1;
	}

	SetWindowText(Dialoghwnd, WindowName);

	MSG msg = { 0 };

	while (GetMessage(&msg, NULL, 0, 0))
	{
		DispatchMessage(&msg);
	}

	return 0;
}

BOOL IsRunningWithAdministrativePrivileges()
{
	HANDLE hToken = NULL;
	BOOL isAdministrator = FALSE;

	if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
	{
		TOKEN_ELEVATION Elevation;
		DWORD cbSize = sizeof(TOKEN_ELEVATION);

		if (GetTokenInformation(hToken, TokenElevation, &Elevation, sizeof(Elevation), &cbSize))
		{
			isAdministrator = Elevation.TokenIsElevated;
		}
	}

	CloseHandle(hToken);

	return isAdministrator;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_DESTROY:
		{
			PostQuitMessage(0);

			return TRUE;
		}
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// May the devil take care of us
LRESULT CALLBACK DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			// I'm not surprised that the gates of hell are open
			SetTimer(hwnd, 0x29A, 0b1010, NULL);
			CreateRegion(hwnd);

			return TRUE;
		}

		case WM_TIMER:
		{
			if (wParam == 0x29A)
			{
				for (auto& Cheat : Cheats)
				{
					if (GetAsyncKeyState(Cheat.VirtualKeyCode) != 0)
					{
						Cheat.IsKeyPressed = TRUE;
					}
					else if (Cheat.IsKeyPressed)
					{
						Cheat.IsKeyPressed = FALSE;
						DistortReality(Cheat.Type, Cheat.Offset, Cheat.Pointers);
					}
				}
			}

			return TRUE;
		}

		case WM_PAINT:
		{
			PaintRegion();
			ValidateRect(hwnd, FALSE);

			return TRUE;
		}

		case WM_MOVE:
		{
			PaintRegion();

			return TRUE;
		}

		case WM_LBUTTONDOWN:
		{
			isDragging = TRUE;

			return TRUE;
		}

		case WM_MOUSEMOVE:
		{
			if (isDragging)
			{
				isDragging = FALSE;
				GetCursorPos(&CursorPosition);
				PostMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(CursorPosition.x, CursorPosition.y));
			}

			return TRUE;
		}

		case WM_CLOSE:
		{
			KillTimer(hwnd, 0x29A);
			DeleteObject(hRgn);
			DeleteDC(hdcMemory);
			DeleteDC(hdcScreen);			
			EndDialog(hwnd, 0);
			SendMessage(::hwnd, WM_DESTROY, NULL, NULL);

			return TRUE;
		}
	}

	return FALSE;
}

void CreateRegion(HWND hwnd)
{
	hdcScreen = GetDC(hwnd);
	hdcMemory = CreateCompatibleDC(NULL);
	HANDLE hBitmap = LoadImageFromResource(MAKEINTRESOURCE(IDB_BACKGROUND), L"PNG");

	if (!hBitmap)
	{
		return;
	}

	GetObject(hBitmap, sizeof(bmInfo), &bmInfo);
	HGDIOBJ hGdiObj = SelectObject(hdcMemory, hBitmap);

	if (!hGdiObj)
	{
		return;
	}

	hRgn = CreateRectRgn(0, 0, 0, 0);
	COLORREF crTransparent = RGB(0, 255, 0);

	for (long Y = 0; Y < bmInfo.bmHeight; ++Y)
	{
		for (long X = 0; X < bmInfo.bmHeight; ++X)
		{
			while (X < bmInfo.bmWidth && GetPixel(hdcMemory, X, Y) == crTransparent)
			{
				++X;
			}

			long LeftX = X;

			while (X < bmInfo.bmWidth && GetPixel(hdcMemory, X, Y) != crTransparent)
			{
				++X;
			}

			HRGN hRgnTemp = CreateRectRgn(LeftX, Y, X, Y + 1);

			if (CombineRgn(hRgn, hRgn, hRgnTemp, RGN_OR) == ERROR)
			{
				return;
			}

			DeleteObject(hRgnTemp);
		}
	}

	if (!SetWindowRgn(hwnd, hRgn, TRUE))
	{
		return;
	}

	if (!SetWindowPos(hwnd, HWND_NOTOPMOST, (GetSystemMetrics(SM_CXSCREEN)) / 2 - (bmInfo.bmWidth / 2), (GetSystemMetrics(SM_CYSCREEN)) / 2 - (bmInfo.bmHeight / 2), bmInfo.bmWidth, bmInfo.bmHeight, NULL))
	{
		return;
	}

	PaintRegion();
	DeleteObject(hBitmap);
}

void PaintRegion()
{
	BitBlt(hdcScreen, 0, 0, bmInfo.bmWidth, bmInfo.bmHeight, hdcMemory, 0, 0, SRCCOPY);
}

HBITMAP LoadImageFromResource(LPCTSTR lpName, LPCTSTR lpType)
{
	HBITMAP hBitmap = NULL;
	IStream* ipImageStream = CreateStreamOnResource(lpName, lpType);

	if (ipImageStream != NULL)
	{
		IWICBitmapSource* ipBitmap = LoadBitmapFromStream(ipImageStream);

		if (ipBitmap != NULL)
		{
			hBitmap = CreateHBITMAP(ipBitmap);
			ipBitmap -> Release();
		}
		
		ipImageStream -> Release();
	}

	return hBitmap;
}

IStream* CreateStreamOnResource(LPCTSTR lpName, LPCTSTR lpType)
{
	IStream* ipStream = NULL;
	HRSRC hResource = FindResource(NULL, lpName, lpType);

	if (hResource != NULL)
	{
		DWORD dwResourceSize = SizeofResource(NULL, hResource);
		HGLOBAL hImage = LoadResource(NULL, hResource);

		if (hImage != NULL)
		{
			LPVOID lpSourceResourceData = LockResource(hImage);

			if (lpSourceResourceData != NULL)
			{
				HGLOBAL hResourceData = GlobalAlloc(GMEM_MOVEABLE, dwResourceSize);

				if (hResourceData != NULL)
				{
					LPVOID lpResourceData = GlobalLock(hResourceData);

					if (lpResourceData != NULL)
					{
						CopyMemory(lpResourceData, lpSourceResourceData, dwResourceSize);
						GlobalUnlock(hResourceData);
						CreateStreamOnHGlobal(hResourceData, TRUE, &ipStream);
					}
					else
					{
						GlobalFree(hResourceData);
					}
				}
			}
		}
	}

	return ipStream;
}

IWICBitmapSource* LoadBitmapFromStream(IStream* ipImageStream)
{
	CoInitialize(NULL);
	IWICImagingFactory* pFactory = nullptr;
	IWICBitmapSource* ipBitmap = NULL;
	IWICBitmapDecoder* ipDecoder = NULL;

	if (SUCCEEDED(CoCreateInstance(CLSID_WICPngDecoder, NULL, CLSCTX_INPROC_SERVER, __uuidof(ipDecoder), reinterpret_cast<void**>(&ipDecoder))))
	{
		if (SUCCEEDED(ipDecoder -> Initialize(ipImageStream, WICDecodeMetadataCacheOnLoad)))
		{
			UINT uFrameCount = 0;

			if (SUCCEEDED(ipDecoder -> GetFrameCount(&uFrameCount)) || uFrameCount != 1)
			{
				IWICBitmapFrameDecode* ipFrame = NULL;

				if (SUCCEEDED(ipDecoder -> GetFrame(0, &ipFrame)))
				{
					WICConvertBitmapSource(GUID_WICPixelFormat32bppPBGRA, ipFrame, &ipBitmap);
					ipFrame -> Release();
				}

				ipDecoder -> Release();
			}
		}
	}

	return ipBitmap;
}

HBITMAP CreateHBITMAP(IWICBitmapSource* ipBitmap)
{
	HBITMAP hBitmap = NULL;
	UINT uWidth = 0, uHeight = 0;

	if (SUCCEEDED(ipBitmap -> GetSize(&uWidth, &uHeight)) && uWidth > 0 && uHeight > 0)
	{
		BITMAPINFO bmInfo;
		bmInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmInfo.bmiHeader.biWidth = uWidth;
		bmInfo.bmiHeader.biHeight = -(LONG)uHeight;
		bmInfo.bmiHeader.biPlanes = 1;
		bmInfo.bmiHeader.biBitCount = 32;
		bmInfo.bmiHeader.biCompression = BI_RGB;

		void* pImageBits = nullptr;
		HDC hdcScreen = GetDC(NULL);
		hBitmap = CreateDIBSection(hdcScreen, &bmInfo, DIB_RGB_COLORS, &pImageBits, NULL, 0);
		ReleaseDC(NULL, hdcScreen);

		if (hBitmap != NULL)
		{
			const UINT cbStride = uWidth * 4;
			const UINT cbImage = cbStride * uHeight;

			if (FAILED(ipBitmap -> CopyPixels(NULL, cbStride, cbImage, static_cast<BYTE*>(pImageBits))))
			{
				DeleteObject(hBitmap);
				hBitmap = NULL;
			}
		}
	}

	return hBitmap;
}

void DistortReality(CheatType& CheatType, DWORD& Offset, const vector<DWORD>& Pointers)
{
	DWORD PID = GetProcessIDByName();

	if (PID > 0)
	{
		uintptr_t BaseAddress = GetBaseAddress(PID);

		if (BaseAddress != 0)
		{
			HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);

			if (hProcess != NULL)
			{
				switch (CheatType)
				{
					case Cash:
					{
						// Pizza2.exe aka fastfood2.exe + 0017C670 + 200 + 108
						uintptr_t CashAddress = DereferencePointer(hProcess, BaseAddress + Offset, Pointers);

						if (CashAddress != 0)
						{
							double Cash = 0;

							if (ReadProcessMemory(hProcess, (LPCVOID*)CashAddress, &Cash, sizeof(Cash), nullptr))
							{
								Cash += 10000.0;

								if (!WriteProcessMemory(hProcess, (LPCVOID*)CashAddress, &Cash, sizeof(Cash), nullptr))
								{
									MessageBox(NULL, L"Could not write memory!", L"Error", MB_ICONERROR | MB_OK);
								}
							}
							else
							{
								MessageBox(NULL, L"Could not read memory!", L"Error", MB_ICONERROR | MB_OK);
							}
						}
						else
						{
							MessageBox(NULL, L"Could not dereference address!", L"Error", MB_ICONERROR | MB_OK);
						}

						break;
					}

					case Points:
					{
						// Pizza2.exe aka fastfood2.exe + 0017C670 + 224 + 3C
						uintptr_t PointsAddress = DereferencePointer(hProcess, BaseAddress + Offset, Pointers);

						if (PointsAddress != 0)
						{
							// I'm not sure if 6 instead of 4 doesn't mess up the game math, anyway I haven't seen bonus values ​​above 4
							// The sequence of points is as follows: first the base points (we get these before game starts), then the extra points (we get these during the game)
							// The order of points is as follows: Buyer, Gangsters, Specialist, Politicians, Seller, Guard
							const vector<DWORD> Points = { 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4 };
							// or alternatively, we do this
							// vector<DWORD> Points;
							// Points.insert(Points.end(), 6, 3);
							// Points.insert(Points.end(), 6, 4);

							if (!WriteProcessMemory(hProcess, (LPCVOID*)PointsAddress, Points.data(), sizeof(DWORD) * Points.size(), nullptr))
							{
								MessageBox(NULL, L"Could not write memory!", L"Error", MB_ICONERROR | MB_OK);
							}
						}
						else
						{
							MessageBox(NULL, L"Could not dereference address!", L"Error", MB_ICONERROR | MB_OK);
						}

						break;
					}

					case Time:
					{						
						// This is place where the magic happens, i.e. we change "INC ECX" instruction from 004DD3DD to "JMP (DWORD) 00000098" which means we teleport to 004DD475 address and bypass clock bumping
						// Pizza2.exe aka fastfood2.exe + 004DD3DD
						uintptr_t InstructionAddress = BaseAddress + Offset;
						vector<BYTE> Instruction(5, 0);

						if (ReadProcessMemory(hProcess, (LPCVOID*)InstructionAddress, Instruction.data(), Instruction.size(), nullptr))
						{
							// ༼ つ ｡◕‿‿◕｡ ༽つ Cute solution aka minimal portion of shellcode required
							const vector<BYTE> Payload = (Instruction == ClockStatesShellcode.Normal ? ClockStatesShellcode.Halt : ClockStatesShellcode.Normal);

							if (!WriteProcessMemory(hProcess, (LPCVOID*)InstructionAddress, Payload.data(), sizeof(BYTE) * Payload.size(), nullptr))
							{
								MessageBox(NULL, L"Could not write memory!", L"Error", MB_ICONERROR | MB_OK);
							}
						}
						else
						{
							MessageBox(NULL, L"Could not read memory!", L"Error", MB_ICONERROR | MB_OK);
						}

						break;
					}
				}
			}
			else
			{
				MessageBox(NULL, L"Could not open target process!", L"Error", MB_ICONERROR | MB_OK);
			}

			CloseHandle(hProcess);
		}
		else
		{
			MessageBox(NULL, L"Could not get base address!", L"Error", MB_ICONERROR | MB_OK);
		}
	}
	else
	{
		MessageBox(NULL, L"Game process not found!", L"Error", MB_ICONERROR | MB_OK);
	}
}

DWORD GetProcessIDByName()
{
	DWORD PID = 0;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hSnapshot != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 pe;
		pe.dwSize = sizeof(pe);

		if (Process32First(hSnapshot, &pe))
		{
			do
			{
				if (wcscmp(pe.szExeFile, BirnaryName) == 0)
				{
					PID = pe.th32ProcessID;

					break;
				}
			}
			while (Process32Next(hSnapshot, &pe));
		}
	}

	CloseHandle(hSnapshot);

	return PID;
}

DWORD GetBaseAddress(DWORD PID)
{
	DWORD BaseAddress = 0;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, PID);

	if (hSnapshot != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 me;
		me.dwSize = sizeof(MODULEENTRY32);

		if (Module32First(hSnapshot, &me))
		{
			do
			{
				if (wcscmp(BirnaryName, me.szModule) == 0)
				{
					BaseAddress = reinterpret_cast<DWORD>(me.modBaseAddr);
				}

			} 
			while (Module32Next(hSnapshot, &me));
		}
	}

	CloseHandle(hSnapshot);

	return BaseAddress;
}

// May be also called FindDMAAddress()
DWORD DereferencePointer(HANDLE hProc, DWORD BaseAddress, const vector<DWORD>& Offsets)
{
	for (const auto& Offset : Offsets)
	{
		if (!ReadProcessMemory(hProc, (LPCVOID*)BaseAddress, &BaseAddress, sizeof(BaseAddress), nullptr))
		{
			return 0;
		}

		BaseAddress += Offset;
	}

	return BaseAddress;
}
