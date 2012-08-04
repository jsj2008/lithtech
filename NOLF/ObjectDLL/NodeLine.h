// ----------------------------------------------------------------------- //
//
// MODULE  : NodeLine.h
//
// PURPOSE : Lines used to debug node system
//
// CREATED : 2/11/99
//
// ----------------------------------------------------------------------- //

#ifndef __NODELINE_H__
#define __NODLINE_H__

#include "ltengineobjects.h"
#include "ClientServerShared.h"

class NodeLine : public BaseClass
{
	public : // Public methods

		// Ctors/dtors/etc

		NodeLine() : BaseClass (OT_NORMAL) {}

		// Simple accessors

        void    Setup(const LTVector& vSource, const LTVector& vDestination);

	protected : // Protected methods (only accessed by engine)

        uint32  EngineMessageFn (uint32 messageID, void *pData, LTFLOAT lData);
};

#endif