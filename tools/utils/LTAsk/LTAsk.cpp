// LTAsk - used for batch utilities.  Returns 1 if the user presses

#include "stdafx.h"
#include <stdio.h>
#include <conio.h>
#include <ctype.h>
#include "tdguard.h"


int main(int argc, char* argv[])
{
	if (!TdGuard::Aegis::GetSingleton().Init() ||
		!TdGuard::Aegis::GetSingleton().DoWork())
	{
        return 1;
	}

	if(argc < 2)
	{
		printf("LTAsk - for batch utilities.  Returns 1 if user presses Y and 0 otherwise.\n");
		printf("LTAsk <question string>\n\n");
		return 0;
	}

	printf(argv[1]);
	if(toupper(getche()) == 'Y')
	{
		printf("\n");
		return 1;
	}
	else
	{
		printf("\n");
		return 0;
	}
}
