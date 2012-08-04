// ----------------------------------------------------------------------- //
//
// MODULE  : HUDEvidence.h
//
// PURPOSE : HUDItem to display evidence info
//
// CREATED : 03/29/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUDEVIDENCE_H__
#define __HUDEVIDENCE_H__

#include "HUDDialogue.h"

class CHUDEvidence : public CHUDDialogue
{
protected:
	virtual HRECORD	GetLayout();
};

#endif //__HUDEVIDENCE_H__

