#pragma once

class ExceptionUtil
{
public:
	explicit ExceptionUtil();
	virtual ~ExceptionUtil();

public:
	static void ErrQuit(char * msg);
	static void ErrDisplay(const char * msg, int err_no);
	static void ErrDisplay(const char * msg);


};