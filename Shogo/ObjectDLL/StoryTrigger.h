// ----------------------------------------------------------------------- //
//
// MODULE  : StoryTrigger.h
//
// PURPOSE : CStoryTrigger - Definition
//
// CREATED : 12/8/97
//
// ----------------------------------------------------------------------- //

#ifndef __STORY_TRIGGER_H__
#define __STORY_TRIGGER_H__

#include "cpp_engineobjects_de.h"

class CStoryTrigger : public BaseClass
{
	public :

		CStoryTrigger();
		~CStoryTrigger() {}

	protected :

		virtual DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
		virtual DDWORD ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

		virtual void  HandleTriggerMsg(HOBJECT hSender, HMESSAGEREAD hRead);

};

#endif // __STORY_TRIGGER_H__