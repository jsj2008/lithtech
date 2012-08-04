#ifndef __CLIENT_NODETRACKERCONTEXT_H__
#define __CLIENT_NODETRACKERCONTEXT_H__

#include "NodeTrackerContext.h"


//
// CClientNodeTrackerContext:
// Manages the current node tracking settings per model instance
// on the client.
//

class CClientNodeTrackerContext : public CNodeTrackerContext
{
	typedef CNodeTrackerContext super;

public:
	// Creation/destruction.

			 CClientNodeTrackerContext();
	virtual	~CClientNodeTrackerContext();

	// Message handling.

	void HandleServerMessage(ILTMessage_Read *pMsg);

	// Update.

	void UpdateNodeTrackers();

protected:

	void HandleToggleGroups(ILTMessage_Read *pMsg);
	void HandleSetTarget(ILTMessage_Read *pMsg);

};


#endif

