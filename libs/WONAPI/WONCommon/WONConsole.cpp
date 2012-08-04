#include "WONConsole.h"

#include <stdarg.h>

using namespace std;
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
Console::Console()
{
#ifdef _LINUX
	tcgetattr(0, &mOldTerm);
	termios aTerm = mOldTerm;
	aTerm.c_lflag &= ~(ICANON | ECHO);
	aTerm.c_cc[VMIN] = 1;
	aTerm.c_cc[VTIME] = 0;
	tcsetattr(0, TCSANOW, &aTerm);
#endif // _LINUX
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
Console::~Console()
{
#ifdef _LINUX
	tcsetattr(0, TCSANOW, &mOldTerm);
#endif
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef _LINUX
static void SetCanonical(bool on)
{
	termios term;
	tcgetattr(0, &term);

	if(on)
		term.c_lflag |= (ECHO | ICANON);
	else
		term.c_lflag &= ~(ECHO | ICANON);

	tcsetattr(0, TCSANOW, &term);	
}
#else
static void SetCanonical(bool)
{
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int Console::getch()
{
#if defined(_LINUX) || defined(_XBOX)
	return ::getchar();
#else
	return ::getch();
#endif
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int Console::kbhit()
{
#ifdef _LINUX
	pollfd aPollStruct;
	aPollStruct.fd = 0; // stdin
	aPollStruct.events = POLLIN;
	aPollStruct.revents = 0;
	return poll(&aPollStruct, 1, 0)>0;
#elif defined(_MAC) || defined(WIN32_NOT_XBOX)
	return ::kbhit();
#endif

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
string Console::ReadLine(int theEchoChar)
{
	string aStr;

	if(theEchoChar==-1)
	{
		SetCanonical(true);
		char aBuf[256];
		while(fgets(aBuf,256,stdin)!=NULL)
		{
			char *aPtr = strchr(aBuf,'\n');
			if(aPtr!=NULL)
			{
				*aPtr = '\0';
				aStr += aBuf;
				break;
			}
			else
				aStr += aBuf;
		}	
		SetCanonical(false);
		return aStr;
	}

	while(true)
	{
		int aChar = getch();
		if(aChar==EOF || aChar=='\r' || aChar=='\n')
		{
			printf("\n");
			break;
		}
		else if(aChar=='\b')
		{
			if(aStr.length()>0)
			{
				aStr = aStr.substr(0,aStr.length()-1);
				printf("\b \b");
			}
		}
		else
		{
			aStr+=aChar;
			if(theEchoChar==-1)
				printf("%c",aChar);
			else if(theEchoChar>0)
				printf("%c",theEchoChar);
		}
	}

	return aStr;
}
