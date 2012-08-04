
// Defines the server-side console command stuff..
#ifndef __S_CONCOMMAND_H__
#define __S_CONCOMMAND_H__


class CServerMgr;
struct ConsoleState;


void* sc_Alloc(unsigned long size);
void sc_Free(void *ptr);

void sm_InitConsoleCommands(ConsoleState *pState);
void sm_TermConsoleCommands(ConsoleState *pState);

LTRESULT sm_HandleCommand(ConsoleState *pState, char *pCommand);


#endif // __S_CONCOMMAND_H__


