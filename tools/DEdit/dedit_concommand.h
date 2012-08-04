
#ifndef __DEDIT_CONCOMMAND_H__
#define __DEDIT_CONCOMMAND_H__

	
	#include "concommand.h"


	extern ConsoleState g_DEditConsoleState;


	// Initializes all the base DirectEngine console variables.
	void			dedit_InitConsoleCommands();
	void			dedit_TermConsoleCommands();

	// Call this to run a console command.
	void			dedit_CommandHandler(char *pCommand);


#endif  // __DEDIT_CONCOMMAND_H__



