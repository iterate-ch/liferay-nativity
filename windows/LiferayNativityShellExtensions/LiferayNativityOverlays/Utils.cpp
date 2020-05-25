#include "stdafx.h"
#include "Utils.h"

extern std::wstring logFilePath;

void Utils::Log(PCWSTR log)
{
	HANDLE file = CreateFile(logFilePath.c_str(), FILE_APPEND_DATA, FILE_SHARE_WRITE, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	DWORD dwBytesWritten = 0;

	SYSTEMTIME time;
	GetSystemTime(&time);
	int sizeRequired = GetTimeFormatEx(LOCALE_NAME_USER_DEFAULT, 0, &time, nullptr, nullptr, 0);
	std::vector<wchar_t> timePart(sizeRequired);
	GetTimeFormatEx(LOCALE_NAME_USER_DEFAULT, 0, &time, nullptr, timePart.data(), sizeRequired);
	std::wstring message;
	message.append(timePart.data());
	message.append(L" | ");
	message.append(log);
	message.append(L"\n");
	
	log = message.c_str();
	DWORD dwBytesToWrite = wcslen(log) * sizeof(wchar_t);
	while (dwBytesToWrite > 0) {
		WriteFile(file, log + dwBytesWritten, dwBytesToWrite, &dwBytesWritten, NULL);
		dwBytesToWrite -= dwBytesWritten;
	}

	CloseHandle(file);
}
