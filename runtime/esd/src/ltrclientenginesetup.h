/****************************************************************************
;
;	MODULE:		LTClientEngineSetup (.H)
;
;	PURPOSE:	Implement Real support in LithTech engine
;
;	HISTORY:	12-8-2000 [mds] File created.
;
;	NOTICE:		Copyright (c) 2000 Lithtech, Inc.
;
***************************************************************************/

#ifdef LITHTECH_ESD
#ifndef LTClientEngineSetup_H
#define LTClientEngineSetup_H

#define LITHTECH_ESD_INC 1
#include "lith.h"
#include "bdefs.h"
#undef LITHTECH_ESD_INC
#include "iltesd.h"
#include "pnwintyp.h"
#include "pncom.h"
#include "rmapckts.h"
#include "rmacomm.h"
#include "rmamon.h"
#include "rmafiles.h"
#include "rmaengin.h"
#include "rmacore.h"
#include "rmaclsnk.h"
#include "rmaerror.h"
#include "rmaauth.h"
#include "rmaausvc.h"
#include "rmawin.h"

//-----------------------------------------------------------------------------
// Main class for CLTClientEngineSetup
//-----------------------------------------------------------------------------
class CLTClientEngineSetup : public IRMAClientEngineSetup
{
public:
	// Default constructor
	CLTClientEngineSetup();

	// Default destructor (calls Term if it has not been called)
	~CLTClientEngineSetup();

	// Initialize core
	LTRESULT	Init();

	// Terminate core
	LTRESULT	Term();

    STDMETHOD(QueryInterface)	(THIS_
				REFIID riid,
				void** ppvObj);

    STDMETHOD_(ULONG32,AddRef)	(THIS);

    STDMETHOD_(ULONG32,Release)	(THIS);

    /*
     * IRMAClientEngineSetup methods
     */

    /************************************************************************
     *	Method:
     *	    IRMAClientEngineSetup::Setup
     *	Purpose:
     *      Top level clients use this interface to over-ride certain basic 
     *	    interfaces implemented by the core. Current over-ridable 
     *	    interfaces are: IRMAPreferences, IRMAHyperNavigate
     */
    STDMETHOD(Setup)		(THIS_
				IUnknown* pContext);

protected:
	LTBOOL		m_bInitialized;
	int32		m_RefCount;
};

#endif // LTClientEngineSetup_H
#endif // LITHTECH_ESD