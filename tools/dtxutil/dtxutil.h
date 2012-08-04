//------------------------------------------------------------------
//
//  FILE      : dtxutil.h
//
//  PURPOSE   :	Conversion functions for dtxutil tool
//
//  COPYRIGHT : (c) 2003 Touchdown Entertainment, Inc. All rights reserved.
//
//------------------------------------------------------------------

#ifndef __DTXUTIL_H__
#define __DTXUTIL_H__

#include "bdefs.h"
#include "dtxmgr.h"
#include "load_pcx.h"
#include "pixelformat.h"
#include "streamsim.h"
#include "load_pcx.h"
#include "LTTexture.h"




enum
{
	TGA2DTX,
	TGA2BPP_32P,
	DTX2TGA,
	DTX2BPP_32P,
	QUANTIZE,
	QUANTIZE_ALL,
	R_QUANTIZE,
	R_QUANTIZE_ALL,
	R_DTX2TGA,
	R_FILLDTX,
	DTX_PIXELS,
	DTX_DIR_REPORT,
	DTX_DIR_COMMADELIM,
	NUM_ARGS
};


extern FormatMgr	g_FormatMgr;
extern LAlloc		g_DefAlloc;
extern int			g_LithExceptionType;
extern char *  		g_ReturnErrString;
extern int32		g_DebugLevel;
extern char *		g_Version;



class DtxUtil
{
public:

	static void 		TGA2DTXhandler(const char* inputfile, const char* outputfile);
	static BOOL 		DTX2TGAhandler(const char* inputfile, const char* outputfile);
	static void 		DTX2BPP_32Phandler(const char* inputfile, const char* outputfile);
	static void 		Quantizehandler(const char* inputfile, const char* outputfile, char* command);
	static void 		QuantizeAllhandler(const char* inputfile, const char* outputfile, char* command);
	static void 		FillDTXHandler(const char* input, const char* output, char* command);
	static bool			OutputDTXPixels(const char* inputfile, const char* outputfile);
	static bool			DTXDirReport(const char* inputdir, const char* outputfile, bool CommaDelim);
	static void 		RecursiveHandlerWrapper(const char* startdir, const char* outputdir, char* command, int option);
};


#ifdef _DEBUG
#	define DEBUG_MODE
#endif
#ifdef DEBUG_MODE
#	define DEBUG_EXEC(arg) (arg)
#else
#	define DEBUG_EXEC(arg)
#endif



#define DELETE_FILE(arg) if( _unlink((arg)) )  DEBUG_EXEC(printf("\nerror deleting %s", (arg) ));



#endif // __DTXUTIL_H__
