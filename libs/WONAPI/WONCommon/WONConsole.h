#ifndef __WON_CONSOLE_H__
#define __WON_CONSOLE_H__
#include "WONShared.h"

#if defined(WIN32)
#include <conio.h>
#elif defined(_LINUX)
#include <termios.h> 
#include <unistd.h>
#include <sys/poll.h>
#elif defined(macintosh)
#include <console.h>
#endif

#include <stdio.h>
#include <string>

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class Console
{
private:
#ifdef _LINUX
	termios mOldTerm;
#endif

public:
	Console();
	~Console();

	int getch();
	int kbhit();
	std::string ReadLine(int theEchoChar = -1);
	std::string ReadPassword() { return ReadLine('*'); }
};

};	// namespace WONAPI

#endif
