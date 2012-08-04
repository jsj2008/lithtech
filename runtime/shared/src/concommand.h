
// The concommand module implements the console functionality
// (getting/setting variables, calling commands, parsing console strings, etc..)

#ifndef __CONCOMMAND_H__
#define __CONCOMMAND_H__


class HHashElement;
class HHashTable;


// Command flags.
#define CMD_USERCOMMAND			(1<<0)	// User command (not an engine command).

// Flags to cc_HandleCommand2.
#define CC_NOVARS		(1<<0)	// Don't set variables.
#define CC_NOCOMMANDS	(1<<1)	// Don't run commands.

#define VARFLAG_SAVE	(1<<0)	// Save in cc_SaveConfigFile.


#define VARBUF_LEN		16


// Helpers to define engine variable tables.
#define EV_FLOAT(name, addr) { name, addr, NULL, NULL, NULL }
#define EV_LONG(name, addr) { name, NULL, (int32*)(addr), NULL, NULL }
#define EV_CVAR(name, addr) { name, NULL, NULL, addr, NULL }
#define EV_STRING(name, addr) { name, NULL, NULL, NULL, addr }



typedef void (*LTCommandFn)(int argc, char *argv[]);
typedef void (*LTSaveFn)(FILE *fp);


// This is a general variable that can be added/changed thru the console.
struct LTCommandVar
{
	char			m_Buffer[VARBUF_LEN]; // This is used instead of allocating if possible.
	char			*pVarName;
	char			*pStringVal;
	float			floatVal;
	HHashElement    *hElement;
	uint32			m_VarFlags; // Combination of VARFLAGS_ stuff.
};


// Engine vars are like command vars except that you can specify the
// address of a global variable for quick & easy access to its value.
struct LTEngineVar
{
	const char	*pVarName;
	float	*pValueAddressFloat;
	int32	*pValueAddressLong;
	LTCommandVar **pCommandVarAddress;
	char	**pValueAddressString;
};


// Command structure.. this is ONLY used in cc_InitState to automatically register a list
// of commands.
struct LTCommandStruct
{
	const char		*pCmdName;
	LTCommandFn		fn;
	uint32			flags;
};

// These are used for 'extra' commands registered with cc_AddExtraCommand.
struct LTExtraCommandStruct
{
	const char		*pCmdName;
	LTCommandFn		fn;
	LTLink			link;
	uint32			flags; // Use these however you want.
};


// A console state is a global state for all your variables and functions.
struct ConsoleState
{
// Initialize all these on entry.

	// Save functions, called when a config file is saved.
	LTSaveFn		*m_SaveFns;
	int				m_nSaveFns;

	// All the engine vars.
	LTEngineVar		*m_pEngineVars;
	int				m_nEngineVars;

	// Commands.  cc_InitState just calls cc_AddCommand for you with these.
	LTCommandStruct	*m_pCommandStructs;
	int				m_nCommandStructs;

	// 'Extra' commands.
	LTLink			m_ExtraCommands;

	// Function to print into the console.
	void			(*ConsolePrint)(const char *pMsg, ...);
	
	// Function to allocate/free memory.
	void*			(*Alloc)(uint32	 size);
	void			(*Free)(void *ptr);

	// Called when a variable is added.
	void			(*NewVar)(ConsoleState *pState, LTCommandVar *pVar);

	// Called when a variable is changed.
	void			(*VarChange)(ConsoleState *pState, LTCommandVar *pVar);

// These are managed by concommand.
	HHashTable      *m_VarHash;
	HHashTable      *m_StringHash;

};


// Initialize and shut down a console state.
// Set the variables in the console state, then call cc_InitState.
// cc_TermState clears ALL data in the ConsoleState.
void cc_InitState(ConsoleState *pState);
void cc_TermState(ConsoleState *pState);

// Add and remove extra commands.
LTExtraCommandStruct* cc_AddCommand(ConsoleState *pState, 
	const char *pCmdName, LTCommandFn fn, uint32 flags);

void cc_RemoveCommand(ConsoleState *pState, LTExtraCommandStruct *pCommand);
LTExtraCommandStruct* cc_FindCommand(ConsoleState *pState, const char *pName);


// Call this to handle a console command.
void cc_HandleCommand(ConsoleState *pState, const char *pCommand);

// Handle a command, but pass in flags.
void cc_HandleCommand2(ConsoleState *pState, const char *pCommand, uint32 flags);

// Same as 2, but ORs varFlags with all variable's flags.  varFlags is
// a combination of VARFLAG_ defines above.
void cc_HandleCommand3(ConsoleState *pState, const char *pCommand, uint32 flags, uint32 varFlags);

										 
// Call this to set a console variable
void cc_SetConsoleVariable(ConsoleState *pState, const char *pName, const char *pValue );
										 
// Find and add console variables.
LTCommandVar* cc_FindConsoleVar(ConsoleState *pState, const char *pName);

// Opens and runs each line of the given file like it was entered into the console.
bool cc_RunConfigFile(ConsoleState *pState, const char *pFilename, uint32 flag, uint32 varFlags);

// Saves out a config file with the state of things.
bool cc_SaveConfigFile(ConsoleState *pState, const char *pFilename);

// Saves out a config file with the specified variables
bool cc_SaveConfigFileFields(ConsoleState *pState, const char *pFilename, uint32 nNumValues, const char** pValues);

// Print out a variable description in the console.
void cc_PrintVarDescription(ConsoleState *pState, LTCommandVar *pVar);


#endif  // __CONCOMMAND_H__



