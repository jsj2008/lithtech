/****************************************************************************
;
;	MODULE:		LTRealDef (.CPP)
;
;	PURPOSE:	Definitions for Real Network's SDK functions
;
;	HISTORY:	5-15-2000 [mds] File created.
;
;	NOTICE:		Copyright (c) 2000 Lithtech, Inc.
;
***************************************************************************/

#ifdef LITHTECH_ESD

#define INITGUID

#define LITHTECH_ESD_INC 1
#include "lith.h"
#undef LITHTECH_ESD_INC
#include "pncom.h"
#include "rmamon.h"
#include "rmaengin.h"
#include "rmacore.h"
#include "rmaclsnk.h"
#include "rmaerror.h"
#include "rmaausvc.h"
#include "rmawin.h"
#include "rmavsurf.h"
#include "rmacomm.h"
#include "rmaallow.h"
#include "pntypes.h"
#include "pnwintyp.h"
#include "rmapckts.h"
#include "rmasite2.h"
#include "rmafiles.h"
#include "rmaplugn.h"

IRMAAudioDevice;
IRMACallback;
IRMAClientAdviseSink;
IRMACommonClassFactory;
IRMAErrorMessages;
IRMAErrorSink;
IRMAPlayer;
IRMAPNRegistry;
IRMAScheduler;
IRMAValues;
IRMAVideoSurface;
IRMAFileSystemObject;
IRMAPlugin;
IRMAGetFileFromSamePool;
IRMAFileStat;
IRMAFileExists;
IRMARequestHandler;
IRMADirHandler;
IRMAFileObject;

#endif // LITHTECH_ESD