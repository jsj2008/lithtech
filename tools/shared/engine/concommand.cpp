
#include "bdefs.h"
#include <math.h>
#include <ctype.h>
#include <string.h>
#include "concommand.h"
#include "conparse.h"
#include "console.h"

#ifdef __PS2
#include <sifdev.h>
#endif

// ------------------------------------------------------------------ //
// Helpers..
// ------------------------------------------------------------------ //

static inline char cc_Toupper(char theChar)
{
	if(theChar >= 'a' && theChar <= 'z')
		return 'A' + (theChar - 'a');
	else
		return theChar;
}

static inline int cc_UpperStrcmp(char *pStr1, char *pStr2)
{
	for(;;)
	{
		if(cc_Toupper(*pStr1) != cc_Toupper(*pStr2))
			return 0;

		if(*pStr1 == 0)
			return 1;

		++pStr1;
		++pStr2;
	}
}

static char *cc_AddString(ConsoleState *pState, char *pName)
{
	HHASHELEMENT hElement;
	char *pString;
	int nStrLen;

	nStrLen = strlen( pName );

	// See if it's in the hash table.
	hElement = hs_FindElement( pState->m_StringHash, ( void * )pName, nStrLen + 1 );
	if( hElement )
	{
		pString = ( char * )hs_GetElementKey( hElement, LTNULL );
		return pString;
	}

	// Make a new one...
	hElement = hs_AddElement( pState->m_StringHash, ( void * )pName, nStrLen + 1 );
	if( !hElement )
		return LTNULL;

	pString = ( char * )hs_GetElementKey( hElement, LTNULL );

	return pString;
}


static LTCommandVar* cc_AddConsoleVar(ConsoleState *pState, char *pName)
{
	LTCommandVar *pVar;
	HHASHELEMENT hElement;
	int nStrLen;

	nStrLen = strlen( pName );

	// See if it's in the hash table.
	hElement = hs_FindElement( pState->m_VarHash, ( void * )pName, nStrLen + 1 );
	if( hElement )
	{
		return ( LTCommandVar * )hs_GetElementUserData( hElement );
	}

	// Make a new one...
	pVar = (LTCommandVar*)pState->Alloc(sizeof(LTCommandVar));
	if( !pVar )
		return LTNULL;
	memset(pVar, 0, sizeof(LTCommandVar));

	// Add to the var hash table...
	hElement = hs_AddElement( pState->m_VarHash, ( void * )pName, nStrLen + 1 );
	if( !hElement )
	{
		pState->Free( pVar );
		return LTNULL;
	}

	// Setup member variables...
	pVar->pVarName = ( char * )hs_GetElementKey( hElement, LTNULL );
	pVar->hElement = hElement;
	hs_SetElementUserData( hElement, ( void * )pVar );

	return pVar;
}

static LTEngineVar* cc_FindEngineVar(ConsoleState *pState, char *pName)
{
	int i;

	for(i=0; i < pState->m_nEngineVars; i++)
	{
		if(cc_UpperStrcmp(pState->m_pEngineVars[i].pVarName, pName))
			return &pState->m_pEngineVars[i];
	}

	return LTNULL;
}

static void cc_SetEngineVar(ConsoleState *pState, LTCommandVar *pVar)
{
	LTEngineVar	*pEngineVar = cc_FindEngineVar(pState, pVar->pVarName);

	if(pEngineVar)
	{
		if(pEngineVar->pCommandVarAddress)
			*pEngineVar->pCommandVarAddress = pVar;

		if(pEngineVar->pValueAddressFloat)
			*pEngineVar->pValueAddressFloat = pVar->floatVal;

		if(pEngineVar->pValueAddressLong)
			*pEngineVar->pValueAddressLong = (int32)pVar->floatVal;

		if(pEngineVar->pValueAddressString)
			*pEngineVar->pValueAddressString = pVar->pStringVal;
	}
}



// ------------------------------------------------------------------ //
// Main functions.
// ------------------------------------------------------------------ //

void cc_PrintStringColor(uint8 r, uint8 g, uint8 b, char *pMsg, ...)
{
	va_list marker;
	char msg[256];

	va_start(marker, pMsg);
	#ifdef __PS2
	vsprintf(msg, pMsg, marker);
	#else
	_vsnprintf(msg, 255, pMsg, marker);
	#endif
	va_end(marker);

	#ifdef DE_CLIENT_COMPILE
		con_PrintString(CONRGB(r,g,b), 0, msg);
	#endif
}

void cc_HandleCommand(ConsoleState *pState, char *pCommand)
{
	cc_HandleCommand2(pState, pCommand, 0);
}


void cc_HandleCommand2(ConsoleState *pState, char *pCommand, uint32 flags)
{
	cc_HandleCommand3(pState, pCommand, flags, 0);
}


void cc_HandleCommand3(ConsoleState *pState, char *pCommand, uint32 flags, uint32 varFlags)
{
	int bNew, bChanged, bFound;
	LTCommandVar *pVar;
	LTLink *pCur;
	LTExtraCommandStruct *pExtraCommand;
	char tempNumString[50];
	int tempNum;
	char *pVarName, *pString;
	LTBOOL bForceNoSave;
	uint32 newFlags;
	ConParse parse;


	parse.Init(pCommand);
	while(parse.Parse())
	{
		if(parse.m_nArgs > 0)
		{
			// Forward the command to command structs.
			bFound = 0;

			if(!(flags & CC_NOCOMMANDS))
			{
				if(!bFound)
				{
					pCur = pState->m_ExtraCommands.m_pNext;
					while(pCur != &pState->m_ExtraCommands)
					{
						pExtraCommand = (LTExtraCommandStruct*)pCur->m_pData;

						if(cc_UpperStrcmp(pExtraCommand->pCmdName, parse.m_Args[0]))
						{
							pExtraCommand->fn(parse.m_nArgs-1, &parse.m_Args[1]);
							bFound = 1;
							break;
						}

						pCur = pCur->m_pNext;
					}
				}
			}

			// Check if it was a command...
			if( bFound )
				continue;

			// Treat it like a variable.
			if(parse.m_nArgs == 2 && !(flags & CC_NOVARS))
			{
				bChanged = 0;
				bNew = 0;

				newFlags = varFlags;
				pVarName = parse.m_Args[0];
				bForceNoSave = LTFALSE;
				if(pVarName[0] == '+')
				{
					newFlags |= VARFLAG_SAVE;
					++pVarName;
				}
				else if(pVarName[0] == '-')
				{
					bForceNoSave = LTTRUE;
					++pVarName;
				}


				// Add the variable if not already there.
				pVar = cc_FindConsoleVar(pState, pVarName);
				if(!pVar)
				{
					bNew = bChanged = 1;
					pVar = cc_AddConsoleVar(pState, pVarName);
				}

				pVar->m_VarFlags |= newFlags;
				if(bForceNoSave)
					pVar->m_VarFlags &= ~VARFLAG_SAVE;

				// Check the negation syntax.
				if(parse.m_Args[1][0] == '!' && parse.m_Args[1][1] == 0)
				{
					if(pVar->pStringVal)
					{
						if(isdigit(pVar->pStringVal[0]) || pVar->pStringVal[0] == '-')
						{
							tempNum = !((int)pVar->floatVal);
							sprintf(tempNumString, "%d", tempNum);
							parse.m_Args[1] = tempNumString;
						}
					}
					else
					{
						parse.m_Args[1] = "1";
					}
				}

				// Check if it changed..
				if(pVar->pStringVal && strcmp(pVar->pStringVal, parse.m_Args[1])==0)
				{
				}
				else
				{
					if(strlen(parse.m_Args[1]) < VARBUF_LEN)
					{
						// Put it in its own buffer so we don't have to allocate or search.
						strcpy(pVar->m_Buffer, parse.m_Args[1]);
						pString = pVar->m_Buffer;
					}
					else
					{
						pString = cc_AddString(pState, parse.m_Args[1]);
					}

					pVar->pStringVal = pString;
					pVar->floatVal = (float)atof(parse.m_Args[1]);
					bChanged = 1;
				}

				cc_SetEngineVar(pState, pVar);

				// Call the appropriate callback.
				if(bChanged)
				{
					if(bNew)
					{
						if(pState->NewVar)
							pState->NewVar(pState, pVar);
					}
					else
					{
						if(pState->VarChange)
							pState->VarChange(pState, pVar);
					}
				}
			}
			else if(parse.m_nArgs == 1)
			{
				pVar = cc_FindConsoleVar(pState, parse.m_Args[0]);
				if(pVar)
					cc_PrintVarDescription(pState, pVar);
			}
		}
	}
}


// Sets a specific variable without having to parse it...
void cc_SetConsoleVariable(ConsoleState *pState, char *pName, char *pValue )
{
	int bNew, bChanged;
	LTCommandVar *pVar;
	char *pString;

	bChanged = 0;
	bNew = 0;

	// Add the variable if not already there.
	pVar = cc_FindConsoleVar(pState, pName);
	if(!pVar)
	{
		bNew = bChanged = 1;
		pVar = cc_AddConsoleVar(pState, pName);
	}

	// Try to add the string.  If it returns the same pointer var
	// already has, then the string didn't change.
	pString = cc_AddString(pState, pValue);
	if( pString != pVar->pStringVal )
	{
		pVar->pStringVal = pString;
		pVar->floatVal = (float)atof(pValue);
		bChanged = 1;
	}

	cc_SetEngineVar(pState, pVar);

	// Call the appropriate callback.
	if(bChanged)
	{
		if(bNew)
		{
			if(pState->NewVar)
				pState->NewVar(pState, pVar);
		}
		else
		{
			if(pState->VarChange)
				pState->VarChange(pState, pVar);
		}
	}
}

void cc_InitState(ConsoleState *pState)
{
	int i;

	pState->m_StringHash = hs_CreateHashTable(500, HASH_STRING_NOCASE);
	pState->m_VarHash = hs_CreateHashTable(500, HASH_STRING_NOCASE);
	dl_TieOff(&pState->m_ExtraCommands);

	// Register the commands.
	for(i=0; i < pState->m_nCommandStructs; i++)
	{
		cc_AddCommand(pState, pState->m_pCommandStructs[i].pCmdName, pState->m_pCommandStructs[i].fn,
			pState->m_pCommandStructs[i].flags);
	}

	// Create the variables.
}

void cc_TermState(ConsoleState *pState)
{
	LTCommandVar *pVar;
	char *pString;
	HHASHITERATOR hIterator;
	HHASHELEMENT hElement;
	LTLink *pCur, *pNext;
	int i;


	// Clear the console variable strings.
	for(i=0; i < pState->m_nEngineVars; i++)
	{
		if(pState->m_pEngineVars[i].pValueAddressString)
			*pState->m_pEngineVars[i].pValueAddressString = LTNULL;
	}

	// Remove the extra commands.
	pCur = pState->m_ExtraCommands.m_pNext;
	while(pCur && pCur != &pState->m_ExtraCommands)
	{
		pNext = pCur->m_pNext;
		cc_RemoveCommand(pState, (LTExtraCommandStruct*)pCur->m_pData);
		pCur = pNext;
	}

	// Delete the strings...
	hIterator = hs_GetFirstElement(pState->m_StringHash);
	while(hIterator)
	{
		hElement = hs_GetNextElement(&hIterator);
		if( !hElement )
			continue;

		pString = ( char * )hs_GetElementUserData( hElement );
		pState->Free( pString );
	}
	hs_DestroyHashTable(pState->m_StringHash);

	// Delete the vars...
	hIterator = hs_GetFirstElement(pState->m_VarHash);
	while(hIterator)
	{
		hElement = hs_GetNextElement(&hIterator);
		if( !hElement )
			continue;

		pVar = ( LTCommandVar * )hs_GetElementUserData( hElement );
		pState->Free( pVar );
	}
	hs_DestroyHashTable(pState->m_VarHash);

	memset(pState, 0, sizeof(ConsoleState));
}


LTExtraCommandStruct* cc_AddCommand(ConsoleState *pState,
	char *pCmdName, LTCommandFn fn, uint32 flags)
{
	LTExtraCommandStruct *pCommand;

	pCommand = (LTExtraCommandStruct*)pState->Alloc(sizeof(LTExtraCommandStruct));
	if(!pCommand)
		return 0;

	pCommand->link.m_pData = pCommand;
	pCommand->fn = fn;
	pCommand->pCmdName = pCmdName;
	pCommand->flags = flags;
	dl_Insert(&pState->m_ExtraCommands, &pCommand->link);

	return pCommand;
}


void cc_RemoveCommand(ConsoleState *pState, LTExtraCommandStruct *pCommand)
{
	dl_Remove(&pCommand->link);
	pState->Free(pCommand);
}


LTExtraCommandStruct* cc_FindCommand(ConsoleState *pState, char *pName)
{
	LTLink *pCur;
	LTExtraCommandStruct *pCommand;

	pCur = pState->m_ExtraCommands.m_pNext;
	while(pCur != &pState->m_ExtraCommands)
	{
		pCommand = (LTExtraCommandStruct*)pCur->m_pData;
		if(cc_UpperStrcmp(pCommand->pCmdName, pName))
			return pCommand;

		pCur = pCur->m_pNext;
	}

	return 0;
}


LTCommandVar* cc_FindConsoleVar(ConsoleState *pState, char *pName)
{
	HHASHELEMENT hElement;

	// See if it's in the hash table.
	hElement = hs_FindElement( pState->m_VarHash, ( void * )pName, strlen( pName ) + 1 );
	if( hElement )
	{
		return ( LTCommandVar * )hs_GetElementUserData( hElement );
	}

	return LTNULL;
}


int cc_RunConfigFile(ConsoleState *pState, const char *pFilename, uint32 flags, uint32 varFlags)
{
#ifdef __PS2
    char fullName[64];
    char *fileBuf;
    int fp;

    sprintf(fullName, "host0:%s", pFilename);
    fp = sceOpen(fullName, SCE_RDONLY);
    if(fp < 0) {
        printf("Couldn't open config file %s\n", pFilename);
        return 0;
	}

    int fileLen = sceLseek(fp, 0, SCE_SEEK_END);
	sceLseek(fp, 0, SCE_SEEK_SET);
    fileBuf = (char *)calloc(1, fileLen + 4);
    if(!fileBuf) {
        sceClose(fp);
        return(0);
    }
    int nRead = sceRead(fp, (void *)fileBuf, fileLen);
    sceClose(fp);

    char *end  = &fileBuf[fileLen];
    char *line = fileBuf;
    char *cur  = line;
    while(cur < end) {
        while(*cur != 0x0A && cur < end) {
            cur++;
        }
        *cur = 0;
        cc_HandleCommand3(pState, line, flags, varFlags);
        line = cur + 1;
    }
    return(1);
#else
	FILE *fp;
	char line[500];
	
	fp = fopen(pFilename, "rt");
	if(!fp)
		return 0;

	while(fgets(line, 500, fp))
	{
		cc_HandleCommand3(pState, line, flags, varFlags);
	}

	fclose(fp);
	return 1;
#endif
}


int cc_SaveConfigFile(ConsoleState *pState, const char *pFilename)
{
	FILE *fp;
	int i;
	LTCommandVar *pVar;
	HHASHITERATOR hIterator;
	HHASHELEMENT hElement;

	fp = fopen(pFilename, "wt");
	if(!fp)
		return 0;

	// Save the vars.
	hIterator = hs_GetFirstElement( pState->m_VarHash );
	while(hIterator)
	{
		hElement = hs_GetNextElement(&hIterator);
		if( !hElement )
			continue;
		
		pVar = ( LTCommandVar * )hs_GetElementUserData( hElement );
		if(pVar->m_VarFlags & VARFLAG_SAVE)
		{
			fprintf(fp, "\"%s\" \"%s\"\n", pVar->pVarName, pVar->pStringVal);
		}
	}
	
	// Call the save functions.
	for(i=0; i < pState->m_nSaveFns; i++)
	{
		pState->m_SaveFns[i](fp);
	}
	
	fclose(fp);
	return 1;
}

void cc_PrintVarDescription(ConsoleState *pState, LTCommandVar *pVar)
{
	if(pVar->pStringVal)
	{
		pState->ConsolePrint("%s = %s", pVar->pVarName, pVar->pStringVal);
	}
}