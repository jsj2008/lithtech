#ifndef __CLIENT_TRACKEDNODECONTEXT_H__
#define __CLIENT_TRACKEDNODECONTEXT_H__

#include "TrackedNodeContext.h"


//
// CClientTrackedNodeContext:
// Manages the current node tracking settings per model instance
// on the client.
//

class CClientTrackedNodeContext : public CTrackedNodeContext
{
	typedef CTrackedNodeContext super;

public:
	// Creation/destruction.

			 CClientTrackedNodeContext();
	virtual	~CClientTrackedNodeContext();

	// Message handling.

	void HandleServerMessage(ILTMessage_Read *pMsg);

protected:

	void HandleActivateGroup(ILTMessage_Read *pMsg);
	void HandleSetTarget(ILTMessage_Read *pMsg);
};


#endif

