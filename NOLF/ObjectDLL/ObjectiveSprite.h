// ----------------------------------------------------------------------- //
//
// MODULE  : ObjectiveSprite.h
//
// PURPOSE : ObjectiveSprite class - definition
//
// CREATED : 12/07/97
//
// ----------------------------------------------------------------------- //

#ifndef __SCALE_SPRITE_H__
#define __SCALE_SPRITE_H__

#include "GameBase.h"
#include "SFXFuncs.h"

class ObjectiveSprite : public GameBase
{
	public :

		ObjectiveSprite();
		~ObjectiveSprite();

	protected :

        virtual uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
        uint32 ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

	private :

        LTBOOL InitialUpdate();
        LTBOOL Update();
		void  ReadProp(ObjectCreateStruct *pStruct);
		void  PostPropRead(ObjectCreateStruct *pStruct);

        void    Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
        void    Load(HMESSAGEREAD hRead, uint32 dwSaveFlags);
		void	CacheFiles();

		void	SetCompleted();
		void	Reset();
		void	CreateSFXMsg();

        LTVector	m_vScale;               // Size (relative) of sprite
		uint8		m_nObjectiveNum;
		LTBOOL		m_bStartOn;


		OBJSPRITECREATESTRUCT m_ObjSpriteStruct;  // Holds all special fx obj sprite info


};

#endif // __SCALE_SPRITE_H__