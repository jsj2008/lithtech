// ----------------------------------------------------------------------- //
//
// MODULE  : ObjectiveSprite.h
//
// PURPOSE : ObjectiveSprite class - definition
//
// CREATED : 12/07/97
//
// ----------------------------------------------------------------------- //

#ifndef __OBJECTIVESPRITE_H__
#define __OBJECTIVESPRITE_H__

#include "GameBase.h"
#include "SFXFuncs.h"
#include "SharedFXStructs.h"

LINKTO_MODULE( ObjectiveSprite );

class ObjectiveSprite : public GameBase
{
	public :

		ObjectiveSprite();
		~ObjectiveSprite();

	protected :

        virtual uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
		virtual bool OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

	private :

        LTBOOL InitialUpdate();
        LTBOOL Update();
		void  ReadProp(ObjectCreateStruct *pStruct);
		void  PostPropRead(ObjectCreateStruct *pStruct);

        void    Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        void    Load(ILTMessage_Read *pMsg, uint32 dwSaveFlags);

		void	SetCompleted();
		void	Reset();
		void	CreateSFXMsg();

        LTVector	m_vScale;               // Size (relative) of sprite
		uint8		m_nObjectiveNum;
		LTBOOL		m_bStartOn;


		SFXCREATESTRUCT m_ObjSpriteStruct;  // Holds all special fx obj sprite info


};

#endif // __OBJECTIVESPRITE_H__