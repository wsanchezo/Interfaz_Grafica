#pragma once
#include <Windows.h>
class _DEBUGGER
{

	//********************************** VARIAS 
	static const int COMODIN_LENGHT = 1024;
	//comodin reservado para this->Debug()
	wchar_t comodin[COMODIN_LENGHT];
	//******************************  OUTPUT   *****************************************************
	enum class OUTPUT_t { DEBUG, LISTBOX, LISTBOX_ONLY };
	OUTPUT_t OUTPUT = OUTPUT_t::DEBUG;
	HWND ListBox_forOutput = NULL;
public:wchar_t* Debug(const wchar_t* message) {
	int pos;
	switch (OUTPUT)
	{
	case OUTPUT_t::LISTBOX_ONLY:
		pos = (int)SendMessage(ListBox_forOutput, LB_ADDSTRING, 0,
			(LPARAM)message);
		break;
	case OUTPUT_t::LISTBOX:
		pos = (int)SendMessage(ListBox_forOutput, LB_ADDSTRING, 0,
			(LPARAM)message);
		lstrcpyW(comodin, message);
		lstrcatW(comodin, L"\n");
		OutputDebugString(comodin);
		break;
	case OUTPUT_t::DEBUG:
	default:
		lstrcpyW(comodin, message);
		lstrcatW(comodin, L"\n");
		OutputDebugString(comodin);
		break;
	}
	return comodin;
}
public:wchar_t* Debug(const wchar_t* message, wchar_t* message2) {


	lstrcpyW(comodin, message);
	lstrcatW(comodin, message2);
	Debug(comodin);
	return comodin;
}
public:wchar_t* Debug(wchar_t* message, char* message2) {
	wchar_t  comodin2[1024];
	size_t length = 0;
	int size = strlen(message2);
	mbstowcs_s(&length, comodin2, message2, size + 1);

	lstrcpyW(comodin, message);
	lstrcatW(comodin, comodin2);
	Debug(comodin);
	return comodin;
}
public:wchar_t* Debug(const wchar_t* message, const char* message2) {
	wchar_t  comodin2[1024];
	size_t length = 0;
	int size = strlen(message2);
	mbstowcs_s(&length, comodin2, message2, size + 1);

	lstrcpyW(comodin, message);
	lstrcatW(comodin, comodin2);
	Debug(comodin);
	return comodin;
}
public:wchar_t* Debug(const wchar_t* message, int number) {
	wchar_t  comodin2[1024];
	_itow_s(number, comodin2, 10);
	lstrcpyW(comodin, message);
	lstrcatW(comodin, comodin2);
	Debug(comodin);
	return comodin;
}
public:wchar_t* Debug(const wchar_t* message, int number, const wchar_t* message2, int number2) {
	wchar_t  comodin2[1024];
	_itow_s(number, comodin2, 10);
	lstrcpyW(comodin, message);
	lstrcatW(comodin, comodin2);
	lstrcatW(comodin, message2);
	_itow_s(number2, comodin2, 10);
	lstrcatW(comodin, comodin2);
	Debug(comodin);
	return comodin;
}
public:wchar_t* Debug(const wchar_t* message, int number, const wchar_t* message2, int number2, const wchar_t* message3) {
	wchar_t  comodin2[1024];
	_itow_s(number, comodin2, 10);
	lstrcpyW(comodin, message);
	lstrcatW(comodin, comodin2);
	lstrcatW(comodin, message2);
	_itow_s(number2, comodin2, 10);
	lstrcatW(comodin, comodin2);
	lstrcatW(comodin, message3);
	Debug(comodin);
	return comodin;
}
public:wchar_t* Debug(const wchar_t* message, int number, const wchar_t* message2, int number2, const wchar_t* message3, int number3) {
	wchar_t  comodin2[1024];
	_itow_s(number, comodin2, 10);
	lstrcpyW(comodin, message);
	lstrcatW(comodin, comodin2);
	lstrcatW(comodin, message2);
	_itow_s(number2, comodin2, 10);
	lstrcatW(comodin, comodin2);
	lstrcatW(comodin, message3);
	_itow_s(number3, comodin2, 10);
	lstrcatW(comodin, comodin2);
	Debug(comodin);
	return comodin;
}
public:wchar_t* Debug(const wchar_t* message, int number, const wchar_t* message2, int number2, const wchar_t* message3, int number3, const wchar_t* message4) {
	wchar_t  comodin2[1024];
	_itow_s(number, comodin2, 10);
	lstrcpyW(comodin, message);
	lstrcatW(comodin, comodin2);
	lstrcatW(comodin, message2);
	_itow_s(number2, comodin2, 10);
	lstrcatW(comodin, comodin2);
	lstrcatW(comodin, message3);
	_itow_s(number3, comodin2, 10);
	lstrcatW(comodin, comodin2);
	lstrcatW(comodin, message4);
	Debug(comodin);
	return comodin;
}
public:wchar_t* Debug(const wchar_t* message, int number, const wchar_t* message2) {
	wchar_t  comodin2[1024];
	_itow_s(number, comodin2, 10);
	lstrcpyW(comodin, message);
	lstrcatW(comodin, comodin2);
	lstrcatW(comodin, message2);
	Debug(comodin);
	return comodin;
}
public:wchar_t* Debug(const wchar_t* message, int number, const wchar_t* message2, const wchar_t* message3) {
	wchar_t  comodin2[1024];
	_itow_s(number, comodin2, 10);
	lstrcpyW(comodin, message);
	lstrcatW(comodin, comodin2);
	lstrcatW(comodin, message2);
	lstrcatW(comodin, message3);
	Debug(comodin);
	return comodin;
}
public:wchar_t* Debug(const wchar_t* message, int number, const wchar_t* message2, const char* message3) {
	wchar_t  comodin2[1024];
	_itow_s(number, comodin2, 10);
	lstrcpyW(comodin, message);
	lstrcatW(comodin, comodin2);
	lstrcatW(comodin, message2);

	size_t length = 0;
	int size = strlen(message3);
	mbstowcs_s(&length, comodin2, message3, size + 1);
	lstrcatW(comodin, comodin2);
	Debug(comodin);
	return comodin;
}
public:wchar_t* Debug(const wchar_t* message, int number, int number2) {
	wchar_t  comodin2[1024];
	_itow_s(number, comodin2, 10);
	lstrcpyW(comodin, message);
	lstrcatW(comodin, comodin2);
	_itow_s(number2, comodin2, 10);
	lstrcatW(comodin, comodin2);
	Debug(comodin);
	return comodin;
}
};
