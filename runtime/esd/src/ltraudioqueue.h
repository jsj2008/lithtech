/****************************************************************************
;
;	MODULE:		LTRAudioQueue (.H)
;
;	PURPOSE:	Support class for RealAudioMgr
;
;	HISTORY:	4-18-2000 [mds] File created.
;
;	NOTICE:		Copyright (c) 2000 Lithtech, Inc.
;
***************************************************************************/

#ifdef LITHTECH_ESD
#ifndef LTRAudioQueue_H
#define LTRAudioQueue_H

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
// Audio queue element class
//-----------------------------------------------------------------------------
class CLTAudioQueueElement
{
    friend class CLTAudioQueue;

public:
    CLTAudioQueueElement();
	~CLTAudioQueueElement();
	virtual LTRESULT Init(void* pPtr);
	virtual LTRESULT Term();
    
protected:
    void*					m_pPtr;
    CLTAudioQueueElement*	m_pNext;

};

//-----------------------------------------------------------------------------
// Audio queue class
//-----------------------------------------------------------------------------
class CLTAudioQueue
{
public:
	CLTAudioQueue();
	~CLTAudioQueue();
	virtual LTRESULT Init();
	virtual LTRESULT Term();

	void	Add(void* pElement);
	void	Remove(RMAAudioData** ppPtr);

protected:
	CLTAudioQueueElement*	m_pElementList;
};

#endif // LTRAudioQueue_H
#endif // LITHTECH_ESD