#include "ExceptionUtil.h"
#include <iostream>
#include "Defines.h"

ExceptionUtil::ExceptionUtil()
{
}

ExceptionUtil::~ExceptionUtil()
{
}

void ExceptionUtil::ErrQuit(char * msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCSTR)msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

void ExceptionUtil::ErrDisplay(const char * msg, int err_no)
{
	WCHAR *lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("%s", msg);
	wprintf(L"¿¡·¯%s\n", lpMsgBuf);
	LocalFree(lpMsgBuf);
}

void ExceptionUtil::ErrDisplay(const char * msg)
{
	ErrDisplay(msg, WSAGetLastError());
}