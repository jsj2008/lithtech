//------------------------------------------------------------------
//	FILE	  : Console.h
//	PURPOSE	  : Defines the console.
//	CREATED	  : September 6 1997
// ------------------------------------------------------------------ //

#ifndef __CONSOLE_H__
#define __CONSOLE_H__


	#ifndef __PS2
        #ifndef __RENDERSTRUCT_H__
//	    #include "renderstruct.h"
        #endif
	#endif

    #ifndef __PIXELFORMAT_H__
	#include "pixelformat.h"
    #endif


	typedef void (*CommandHandler)(char *pCommand);
	typedef void (*ErrorLogFn)(char *pMsg);

	// 00 bb gg rr
	typedef PValue CONCOLOR;
	#define CONRGB(r,g,b) PValue_Set(0, r ,g, b)



	// ------------------------------------------------------------------ //
	// The console interface.
	// ------------------------------------------------------------------ //

	// Initialize/term the console.
	LTBOOL con_InitBare();
	#ifdef __PS2
	LTBOOL con_Init(LTRect *pRect, CommandHandler handler);
	#else
	LTBOOL con_Init(LTRect *pRect, CommandHandler handler/*, RenderStruct *pStruct*/);
	#endif
	void con_Term(LTBOOL bDeleteTextLines);

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
	void con_PrintString(CONCOLOR theColor, int filterLevel, char *pMsg);
	void con_Printf(CONCOLOR theColor, int filterLevel, char *pMsg, ...);
	void con_WhitePrintf(char *pMsg, ...);

#endif // __CONSOLE_H__

