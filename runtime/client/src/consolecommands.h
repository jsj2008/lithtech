#ifndef __CONSOLECOMMANDS_H__
#define __CONSOLECOMMANDS_H__

#ifndef __CONCOMMAND_H__
#include "concommand.h"
#endif

// The main client console state, to be used with the concommand functions.
extern ConsoleState g_ClientConsoleState;


// Initializes all the base DirectEngine console variables.
void			c_InitConsoleCommands();
void			c_TermConsoleCommands();

// Call this to run a console command.
void			c_CommandHandler(const char *pCommand);


#endif  // __CONSOLECOMMANDS_H__

