#ifndef __AI_NODETRACKERCONTEXT_H__
#define __AI_NODETRACKERCONTEXT_H__

#include "ServerNodeTrackerContext.h"

// Forward declarations.

class CAI;


//
// CAINodeTrackerContext:
// Manages the current node tracking settings per model instance
// on the server.
//

class CAINodeTrackerContext : public CServerNodeTrackerContext
{
	typedef CServerNodeTrackerContext super;

public:
	// Creation/destruction.

			 CAINodeTrackerContext();

	// Save / Load

	void			Save( ILTMessage_Write *pMsg );
    void			Load( ILTMessage_Read *pMsg );

	// Server update.

	void			UpdateNodeTrackers( CAI* pAI );

protected:

	CNodeTracker::ETargetType	m_eTrackerTarget;
	LTObjRef					m_hTarget;
	HMODELNODE					m_hTargetNode;
	LTVector					m_vTarget;
};


#endif

