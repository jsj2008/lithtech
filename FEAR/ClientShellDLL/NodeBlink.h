// ----------------------------------------------------------------------- //
//
// MODULE  : NodeBlink.cpp
//
// PURPOSE : Blink Node Controller definition
//
// CREATED : 1/30/04
//
// (c) 1998-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __NODEBLINK_H__
#define __NODEBLINK_H__

#include <vector>
#include "LTObjRef.h"

struct BlinkNodeContext
{
	BlinkNodeContext() :
		m_hNode(INVALID_MODEL_NODE),
		m_flPercent(0.f)
	{
		m_rFinal.Identity();
	}

	// Node this controller is associated with.  Used to remove a node 
	// control function.
	HMODELNODE			m_hNode;
	
	// Final rotation this node interpolates towards.  Built from a data
	// supplied axis-angle pair.
	LTRotation			m_rFinal;

	// The current percent [0-1] to be applied.  Pre interpolation.
	float				m_flPercent;
};

class BlinkController
{
	typedef std::vector<BlinkNodeContext*> BlinkNodeContextList;

public:
	BlinkController();
	~BlinkController();

	void	Init(HOBJECT hObj, ModelsDB::HBLINKNODEGROUP hBlinkGroup);
	void	UpdateNodeBlink();

private:
	void	UnbindNodes();

	static void BlinkControllerCB(const NodeControlData& NodeData, void* pUserData);

	StopWatchTimer m_NextBlinkTimer;
	StopWatchTimer m_EndBlinkTimer;
	bool		m_bInBlink;

	float		m_fMinTimeBetweenBlinks;
	float		m_fMaxTimeBetweenBlinks;
	float		m_flBlinkDuration;
	LTObjRef	m_hObject;

	BlinkNodeContextList m_BlinkNodeContextList;
};

#endif // __NODEBLINK_H__