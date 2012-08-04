//------------------------------------------------------------------
//	FILE	  : Console.h
//	PURPOSE	  : Defines the console.
//	CREATED	  : September 6 1997
// ------------------------------------------------------------------ //

#ifndef __CONSOLE_H__
#define __CONSOLE_H__


typedef void (*CommandHandler)(const char *pCommand);
typedef void (*ErrorLogFn)(const char *pMsg);

// 00 bb gg rr
typedef uint32 CONCOLOR;
#define CONRGB(r,g,b) PValue_Set(0xFF, r ,g, b)



// ------------------------------------------------------------------ //
// The console interface.
// ------------------------------------------------------------------ //

// Initialize/term the console.
bool con_InitBare();

struct RenderStruct;
bool con_Init(LTRect *pRect, CommandHandler handler, RenderStruct *pStruct);

void con_Term(bool bDeleteTextLines);

// Load the background bitmap..
LTRESULT con_LoadBackground();

// Set the error log...
void con_SetErrorLog(ErrorLogFn theFunction);

// Keypress handler.
void con_OnKeyPress(uint32 key);

// Cycle thru the commands they've typed in...
void con_CycleCommandsBack();
void con_CycleCommandsForward();


// Draw the console	
void con_Draw();
void con_DrawSmall(int nLines);

// Print strings to the console.
void con_PrintString(CONCOLOR theColor, int filterLevel, const char *pMsg);
void con_Printf(CONCOLOR theColor, int filterLevel, const char *pMsg, ...);
void con_WhitePrintf(const char *pMsg, ...);

#endif // __CONSOLE_H__

